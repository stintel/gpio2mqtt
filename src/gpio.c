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

#include <errno.h>
#include <gpiod.h>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <string.h>
#include <time.h>

#include "gpio.h"
#include "log.h"
#include "mosquitto.h"

struct mon_ctx {
        bool watch_rising;
        bool watch_falling;

        unsigned int offset;
        unsigned int events_wanted;
        unsigned int events_done;

        bool silent;
        char *fmt;

        int sigfd;
};

static int cb_event(int event_t, unsigned int line,
                    const struct timespec *timestamp, void *data)
{
    (void) data;
    (void) line;
    (void) timestamp;

    switch (event_t) {
        case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
            publish("{\"command\": \"switchlight\", \"idx\": 56, \"switchcmd\": \"On\" }");
            break;
        case GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE:
            publish("{\"command\": \"switchlight\", \"idx\": 56, \"switchcmd\": \"Off\" }");
            break;
    }

    return GPIOD_CTXLESS_EVENT_CB_RET_OK;
}

int cb_poll(unsigned int num_lines,
            struct gpiod_ctxless_event_poll_fd *fds,
            const struct timespec *timeout, void *data)
{
    struct pollfd pfds[GPIOD_LINE_BULK_MAX_LINES + 1];
    struct mon_ctx *ctx = data;
    int cnt, ts, ret;
    unsigned int i;

    for (i = 0; i < num_lines; i++) {
        pfds[i].fd = fds[i].fd;
        pfds[i].events = POLLIN | POLLPRI;
    }

    pfds[i].fd = ctx->sigfd;
    pfds[i].events = POLLIN | POLLPRI;

    ts = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;

    cnt = poll(pfds, num_lines + 1, ts);
    if (cnt < 0)
        return GPIOD_CTXLESS_EVENT_POLL_RET_ERR;
    else if (cnt == 0)
        return GPIOD_CTXLESS_EVENT_POLL_RET_TIMEOUT;

    ret = cnt;
    for (i = 0; i < num_lines; i++) {
        if (pfds[i].revents) {
            fds[i].event = true;
            if (!--cnt)
                return ret;
        }
    }

    return GPIOD_CTXLESS_EVENT_POLL_RET_STOP;
}

static int make_signalfd(void)
{
    sigset_t sigmask;
    int sigfd, rv;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);

    rv = sigprocmask(SIG_BLOCK, &sigmask, NULL);
    if (rv < 0) {
        pr_err("error masking signals: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sigfd = signalfd(-1, &sigmask, 0);
    if (sigfd < 0) {
        pr_err("error creating signalfd: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return sigfd;
}

int init_gpio()
{
    struct mon_ctx ctx;
    struct timespec timeout = { 1, 0 };

    ctx.sigfd = make_signalfd();
    ctx.watch_falling = false;

    gpiod_ctxless_event_loop(gpio_chip, gpio_line, false, "gpio2mqttd", &timeout, cb_poll, cb_event, &ctx);

    return 0;
}
