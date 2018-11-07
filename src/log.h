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

#include <syslog.h>

#define pr_err(fmt, ...)    syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)   syslog(LOG_INFO, fmt, ##__VA_ARGS__)
