/*
 * carl9170 firmware - used by the ar9170 wireless device
 *
 * Clock, Timer & Timing
 *
 * Copyright (c) 2000-2005 ZyDAS Technology Corporation
 * Copyright (c) 2007-2009 Atheros Communications, Inc.
 * Copyright	2009	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2009-2011	Christian Lamparter <chunkeey@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __CARL9170FW_TIMER_H
#define __CARL9170FW_TIMER_H

#include "config.h"

enum cpu_clock_t {
	AHB_40MHZ_OSC   = 0,
	AHB_20_22MHZ    = 1,
	AHB_40_44MHZ    = 2,
	AHB_80_88MHZ    = 3
};

static inline __inline uint32_t get_clock_counter(void)
{
	return (get(AR9170_TIMER_REG_CLOCK_HIGH) << 16) | get(AR9170_TIMER_REG_CLOCK_LOW);
}

/*
 * works only up to 97 secs [44 MHz] or 107 secs for 40 MHz
 * Also, the delay wait will be affected by 2.4GHz<->5GHz
 * band changes.
 */
static inline __inline bool is_after_msecs(const uint32_t t0, const uint32_t msecs)
{
	return ((get_clock_counter() - t0) / 1000) > (msecs * fw.ticks_per_msec);
}

/*
 * Note: Be careful with [u]delay. They won't service the
 * hardware watchdog timer. It might trigger if you
 * wait long enough. Also they don't terminate if sec is
 * above 97 sec [44MHz] or more than 107 sec [40MHz].
 */
static inline __inline void delay(const uint32_t msec)
{
	uint32_t t1, t2, dt, wt;

	wt = msec * fw.ticks_per_msec;

	t1 = get_clock_counter();
	while (1) {
		t2 = get_clock_counter();
		dt = (t2 - t1) / 1000;
		if (dt >= wt)
			break;
	}
}

static inline __inline void udelay(const uint32_t usec)
{
	uint32_t t1, t2, dt;

	t1 = get_clock_counter();
	while (1) {
		t2 = get_clock_counter();
		dt = (t2 - t1);
		if (dt >= (usec * fw.ticks_per_msec))
			break;
	}
}

static inline void clock_set(enum cpu_clock_t _clock, bool on)
{
	/*
	 * Word of Warning!
	 * This setting does more than just mess with the CPU Clock.
	 * So watch out, if you need _stable_ timer interrupts.
	 */

	fw.ticks_per_msec = GET_VAL(AR9170_PWR_PLL_ADDAC_DIV, get(AR9170_PWR_REG_PLL_ADDAC)) >> 1;

	set(AR9170_PWR_REG_CLOCK_SEL, (uint32_t) ((on ? 0x70 : 0x600) | _clock));
}

#endif /* __CARL9170FW_TIMER_H */
