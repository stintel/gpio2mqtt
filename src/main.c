/*
 * This file is part of gpio2mqttd.
 *
 * Copyright (C) 2018  Stijn Tintel <stijn@linux-ipv6.be>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "gpio.h"
#include "log.h"
#include "mosquitto.h"

static void print_usage(void)
{
    printf("Usage: -g GPIOchip -l GPIOline -h host [-p port] [-u username] [-P password] -t topic\n");
    printf(" -g: GPIO chip\n");
    printf(" -l: GPIO line\n");
    printf(" -h: MQTT host to connect to\n");
    printf(" -p: MQTT port to connect to (1883)\n");
    printf(" -u: MQTT username\n");
    printf(" -P: MQTT password\n");
    printf(" -t: MQTT topic\n");
    printf(" -V: print version number and exit\n");
#ifdef WITH_TLS
    printf("TLS options:\n");
    printf(" -c: path to CA file\n");
    printf(" -T: use TLS\n");
#endif
}

int main(int argc, char **argv)
{
#ifdef WITH_TLS
    bool tls = false;
    char *cafile = NULL;
#endif
    char *chip = NULL;
    char *host = NULL;
    char *username = NULL;
    char *password = NULL;
    char *topic = NULL;
    int opt = 0;
    int line = -1;
    unsigned int port = 1883;

    openlog("gpio2mqtt", LOG_PID, LOG_DAEMON);

    while ((opt = getopt(argc, argv, "c:g:h:l:p:P:t:Tu:V")) != -1) {
        switch (opt) {
            case 'g':
                chip = optarg;
                break;
            case 'l':
                line = atoi(optarg);
                break;
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'P':
                password = optarg;
                break;
            case 't':
                topic = optarg;
                break;
#ifdef WITH_TLS
            case 'c':
                cafile = optarg;
                break;
            case 'T':
                tls = true;
                break;
#endif
            case 'u':
                username = optarg;
                break;
            case 'V':
                fprintf(stdout, "gpio2mqttd-%s\n", VERSION);
                return 0;
            default:
                print_usage();
                return 1;
        }
    }

    if (chip == NULL || line < 0 || host == NULL || topic == NULL) {
        print_usage();
        return 1;
    }

#ifdef WITH_TLS
    if (tls) {
        if (cafile) {
            if (access(cafile, F_OK) == -1) {
                fprintf(stderr, "CA file %s does not exist.\n", cafile);
                return 1;
            }
        } else {
            fprintf(stderr, "TLS operation requires a CA file.\n");
            return 1;
        }
    }
#endif

    mqtt_host = host;
    mqtt_port = port;
    mqtt_username = username;
    mqtt_password = password;
    mqtt_topic = topic;
#ifdef WITH_TLS
    mqtt_cafile = cafile;
    mqtt_tls = tls;
#endif

    init_gpio(chip, line);

    return 0;
}

