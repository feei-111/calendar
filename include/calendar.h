/*
 * calendar.h — Lunar Calendar Engine & Date Utilities
 *
 * Contains the 1900–2100 lunar calendar lookup table (lunar_info[]),
 * solar-to-lunar conversion, Julian Day Number arithmetic, and
 * zodiac / Chinese zodiac calculations. All functions are pure and
 * stateless — no side effects, no globals.
 */

#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdint.h>

/* --- Solar calendar helpers ------------------------------------------------ */
int  is_leap_year(int y);
int  get_days_in_month(int year, int month);
int  get_first_weekday(int year, int month);  /* 0=Sun .. 6=Sat (Sakamoto)    */

/* --- Julian Day Number conversion (proleptic Gregorian) -------------------- */
int  date_to_jdn(int y, int m, int d);
void jdn_to_date(int jdn, int *y, int *m, int *d);
void add_months(int *y, int *m, int *d, int n);

/* --- Lunar calendar engine ------------------------------------------------- */
int  get_leap_month(int year);              /* returns 0 if no leap month      */
int  get_lunar_month_days(int year, int month);   /* 29 or 30                  */
int  get_leap_month_days(int year);
int  get_lunar_year_days(int year);

/* --- Core conversion -------------------------------------------------------
 * Converts a Gregorian date (sy,sm,sd) to a Chinese lunar date (ly,lm,ld).
 * is_leap is set to 1 if the lunar month is a leap month.                    */
void solar_to_lunar(int sy, int sm, int sd,
                    int *ly, int *lm, int *ld, int *is_leap);

/* --- Astrology helpers ----------------------------------------------------- */
int  get_zodiac_index(int month, int day);         /* 0=Aries .. 11=Pisces     */
int  get_chinese_zodiac_index(int lunar_year);     /* 0=Rat .. 11=Pig         */

#endif
