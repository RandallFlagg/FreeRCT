/* $Id$ */

/*
 * This file is part of FreeRCT.
 * FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file dates.cpp Days and years in the program. */

#include "stdafx.h"
#include "dates.h"
#include "language.h"
#include "finances.h"
#include "sprite_store.h"
#include "ride_type.h"
#include "person.h"
#include "people.h"
#include "window.h"

assert_compile(TICK_COUNT_PER_DAY < (1 << CDB_FRAC_LENGTH)); ///< Day length should stay within the fraction limit.

static const int _days_per_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; ///< Numbers of days in each month (in a non-leap year).

Date _date; ///< %Date in the program.

/**
 * Constructor for a specific date.
 * @param pday Day of the month (1-based).
 * @param pmonth Month (1-based).
 * @param pyear Year (1-based).
 * @param pfrac Day fraction (0-based).
 */
Date::Date(int pday, int pmonth, int pyear, int pfrac) : day(pday), month(pmonth), year(pyear), frac(pfrac)
{
	assert(pyear > 0 && pyear < (1 << CDB_YEAR_LENGTH));
	assert(pmonth > 0 && pmonth < 13);
	assert(pday > 0 && pday <= _days_per_month[pmonth]);
	assert(pfrac >= 0 && pfrac < TICK_COUNT_PER_DAY);
}

/** Default constructor. */
Date::Date() : day(1), month(1), year(1), frac(0)
{
}

/**
 * Copy constructor.
 * @param d Existing date.
 */
Date::Date(const Date &d) : day(d.day), month(d.month), year(d.year), frac(d.frac)
{
}

/**
 * Constructor od a date from a compressed date.
 * @param cd Compressed date source.
 */
Date::Date(CompressedDate cd)
{
	int pyear  = (cd >> CDB_YEAR_START)  & ((1 << CDB_YEAR_LENGTH)  - 1);
	int pmonth = (cd >> CDB_MONTH_START) & ((1 << CDB_MONTH_LENGTH) - 1);
	int pday   = (cd >> CDB_DAY_START)   & ((1 << CDB_DAY_LENGTH)   - 1);
	int pfrac  = (cd >> CDB_FRAC_START)  & ((1 << CDB_FRAC_LENGTH)  - 1);

	assert(pyear > 0 && pyear < (1 << CDB_YEAR_LENGTH));
	assert(pmonth > 0 && pmonth < 13);
	assert(pday > 0 && pday <= _days_per_month[pmonth]);
	assert(pfrac >= 0 && pfrac < TICK_COUNT_PER_DAY);

	this->year  = pyear;
	this->month = pmonth;
	this->day   = pday;
	this->frac  = pfrac;
}

/**
 * Assignment operator.
 * @param d Existing date.
 * @return Assigned value.
 */
Date &Date::operator=(const Date &d)
{
	if (this != &d) {
		this->day = d.day;
		this->month = d.month;
		this->year = d.year;
		this->frac = d.frac;
	}
	return *this;
}

/**
 * Compress the date to an integer number.
 * @return The compressed date.
 */
CompressedDate Date::Compress() const
{
	return (this->year << CDB_YEAR_START) | (this->month << CDB_MONTH_START) | (this->day << CDB_DAY_START) | (this->frac << CDB_FRAC_START);
}

/** Runs various procedures that have to be done yearly. */
static void OnNewYear()
{
	// Nothing (yet) needed.
}

/** Runs various procedures that have to be done monthly. */
static void OnNewMonth()
{
	_finances_manager.AdvanceMonth();
	_rides_manager.OnNewMonth();
}

/** Runs various procedures that have to be done daily. */
static void OnNewDay()
{
	_guests.OnNewDay();
	NotifyChange(WC_BOTTOM_TOOLBAR, ALL_WINDOWS_OF_TYPE, CHG_DISPLAY_OLD, 0);
}

/**
 * Update the day.
 * @todo Care about leap years.
 */
void DateOnTick()
{
	bool newmonth = false;
	bool newyear  = false;

	/* New tick. */
	_date.frac++;
	if (_date.frac < TICK_COUNT_PER_DAY) return;

	/* New day. */
	_date.frac = 0;
	_date.day++;

	/* New month. */
	if (_date.day > _days_per_month[_date.month]) {
		_date.day = 1;
		_date.month++;
		newmonth = true;

		/* New year. */
		if (_date.month > 12) {
			_date.month = 1;
			_date.year++;
			newyear = true;
		}
	}

	OnNewDay();
	if (newmonth) OnNewMonth();
	if (newyear)  OnNewYear();
}

/**
 * Load the current date from the save game.
 * @param ldr Input stream to load from.
 */
void LoadDate(Loader &ldr)
{
	uint32 version = ldr.OpenBlock("DATE");
	if (version == 1) {
		_date = Date(ldr.GetLong());
	} else {
		_date = Date();
		if (version != 0) ldr.SetFail("Unknown date block number");
	}
	ldr.CloseBlock();
}

/**
 * Save the current date to the save game.
 * @param svr Output stream to save to.
 */
void SaveDate(Saver &svr)
{
	svr.StartBlock("DATE", 1);
	svr.PutLong(_date.Compress());
	svr.EndBlock();
}
