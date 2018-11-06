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

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "mosquitto.h"

#ifdef WITH_TLS
bool mqtt_tls;
char *mqtt_cafile;
#endif
char *mqtt_host;
char *mqtt_username;
char *mqtt_password;
char *mqtt_topic;
int mqtt_port;

static void gen_cid(char *mqtt_cid)
{
    char hostname[16];

    gethostname(hostname, 15);
    snprintf(mqtt_cid, MOSQ_MQTT_ID_MAX_LENGTH, "%s-%d", hostname, getpid());
}

static void cb_publish(struct mosquitto *m, void *data, int mid)
{
    (void) data;
    (void) mid;

    mosquitto_disconnect(m);
}

int publish(char *msg)
{
    bool clean_session = true;
    char mqtt_cid[MOSQ_MQTT_ID_MAX_LENGTH];
    struct mosquitto *m = NULL;
    int keepalive = 60;

    gen_cid(&mqtt_cid[0]);

    mosquitto_lib_init();

    m = mosquitto_new(mqtt_cid, clean_session, NULL);

    mosquitto_publish_callback_set(m, cb_publish);

    mosquitto_username_pw_set(m, mqtt_username, mqtt_password);

#ifdef WITH_TLS
    if (mqtt_tls) {
        mosquitto_tls_set(m, mqtt_cafile, NULL, NULL, NULL, NULL);
    }
#endif

    mosquitto_connect(m, mqtt_host, mqtt_port, keepalive);

    mosquitto_publish(m, 0, mqtt_topic, strlen(msg), msg, 2, false);
    mosquitto_loop_forever(m, -1, 1);

    mosquitto_destroy(m);
    mosquitto_lib_cleanup();
    return 0;
}
