/*
 * calendar.c — Lunar Calendar Engine & Date Utilities
 *
 * Core date computation module. Contains the compressed lunar calendar
 * lookup table for 1900–2100 and all solar/lunar/Julian date conversions.
 */

#include <stdint.h>
#include "calendar.h"

/* === Lunar calendar data table (1900—2100, 201 entries) ==================
 * Encoding per entry (20-bit value):
 *   bits [0:3]   — leap month number (0 if none)
 *   bits [4:15]  — month 1..12 size (1=30d, 0=29d)
 *   bits [16]    — leap month size (1=30d, 0=29d)                        */
static const uint32_t lunar_info[] = {
    0x04bd8, 0x04ae0, 0x0a570, 0x054d5, 0x0d260, 0x0d950, 0x16554, 0x056a0, 0x09ad0, 0x055d2,
    0x04ae0, 0x0a5b6, 0x0a4d0, 0x0d250, 0x1d255, 0x0b540, 0x0d6a0, 0x0ada2, 0x095b0, 0x14977,
    0x04970, 0x0a4b0, 0x0b4b5, 0x06a50, 0x06d40, 0x1ab54, 0x02b60, 0x09570, 0x052f2, 0x04970,
    0x06566, 0x0d4a0, 0x0ea50, 0x06e95, 0x05ad0, 0x02b60, 0x186e3, 0x092e0, 0x1c8d7, 0x0c950,
    0x0d4a0, 0x1d8a6, 0x0b550, 0x056a0, 0x1a5b4, 0x025d0, 0x092d0, 0x0d2b2, 0x0a950, 0x0b557,
    0x06ca0, 0x0b550, 0x15355, 0x04da0, 0x0a5b0, 0x14573, 0x052b0, 0x0a9a8, 0x0e950, 0x06aa0,
    0x0aea6, 0x0ab50, 0x04b60, 0x0aae4, 0x0a570, 0x05260, 0x0f263, 0x0d950, 0x05b57, 0x056a0,
    0x096d0, 0x04dd5, 0x04ad0, 0x0a4d0, 0x0d4d4, 0x0d250, 0x0d558, 0x0b540, 0x0b6a0, 0x195a6,
    0x095b0, 0x049b0, 0x0a974, 0x0a4b0, 0x0b27a, 0x06a50, 0x06d40, 0x0af46, 0x0ab60, 0x09570,
    0x04af5, 0x04970, 0x064b0, 0x074a3, 0x0ea50, 0x06b58, 0x055c0, 0x0ab60, 0x096d5, 0x092e0,
    0x0c960, 0x0d954, 0x0d4a0, 0x0da50, 0x07552, 0x056a0, 0x0abb7, 0x025d0, 0x092d0, 0x0cab5,
    0x0a950, 0x0b4a0, 0x0baa4, 0x0ad50, 0x055d9, 0x04ba0, 0x0a5b0, 0x15176, 0x052b0, 0x0a930,
    0x07954, 0x06aa0, 0x0ad50, 0x05b52, 0x04b60, 0x0a6e6, 0x0a4e0, 0x0d260, 0x0ea65, 0x0d530,
    0x05aa0, 0x076a3, 0x096d0, 0x04afb, 0x04ad0, 0x0a4d0, 0x1d0b6, 0x0d250, 0x0d520, 0x0dd45,
    0x0b5a0, 0x056d0, 0x055b2, 0x049b0, 0x0a577, 0x0a4b0, 0x0aa50, 0x1b255, 0x06d20, 0x0ada0,
    0x14b63, 0x09370, 0x049f8, 0x04970, 0x064b0, 0x168a6, 0x0ea50, 0x06b20, 0x1a6c4, 0x0aae0,
    0x0a2e0, 0x0d2e3, 0x0c960, 0x0d557, 0x0d4a0, 0x0da50, 0x05d55, 0x056a0, 0x0a6d0, 0x055d4,
    0x052d0, 0x0a9b8, 0x0a950, 0x0b4a0, 0x0b6a6, 0x0ad50, 0x055a0, 0x0aba4, 0x0a5b0, 0x052b0,
    0x0b273, 0x06930, 0x07337, 0x06aa0, 0x0ad50, 0x14b55, 0x04b60, 0x0a570, 0x054e4, 0x0d160,
    0x0e968, 0x0d520, 0x0daa0, 0x16aa6, 0x056d0, 0x04ae0, 0x0a9d4, 0x0a4d0, 0x0d150, 0x0f252,
    0x0d520
};

int is_leap_year(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int get_days_in_month(int year, int month) {
    static int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) return 29;
    return d[month - 1];
}

int get_first_weekday(int year, int month) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = year;
    if (month < 3) y--;
    return (y + y/4 - y/100 + y/400 + t[month-1] + 1) % 7;
}

int date_to_jdn(int y, int m, int d) {
    int a = (14 - m) / 12;
    int yy = y + 4800 - a;
    int mm = m + 12 * a - 3;
    return d + (153 * mm + 2) / 5 + 365 * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
}

void jdn_to_date(int jdn, int *y, int *m, int *d) {
    int a = jdn + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - (146097 * b) / 4;
    int dd = (4 * c + 3) / 1461;
    int e = c - (1461 * dd) / 4;
    int mm = (5 * e + 2) / 153;
    *d = e - (153 * mm + 2) / 5 + 1;
    *m = mm + 3 - 12 * (mm / 10);
    *y = 100 * b + dd - 4800 + mm / 10;
}

void add_months(int *y, int *m, int *d, int n) {
    *m += n;
    while (*m > 12) { *m -= 12; (*y)++; }
    while (*m < 1)  { *m += 12; (*y)--; }
    int maxd = get_days_in_month(*y, *m);
    if (*d > maxd) *d = maxd;
}

int get_leap_month(int year) {
    if (year < 1900 || year > 2100) return 0;
    return lunar_info[year - 1900] & 0xf;
}

int get_lunar_month_days(int year, int month) {
    if (year < 1900 || year > 2100) return 29;
    return (lunar_info[year - 1900] & (1 << (16 - month))) ? 30 : 29;
}

int get_leap_month_days(int year) {
    if (year < 1900 || year > 2100) return 29;
    return (lunar_info[year - 1900] & 0x10000) ? 30 : 29;
}

int get_lunar_year_days(int year) {
    int sum = 0;
    for (int m = 1; m <= 12; m++)
        sum += get_lunar_month_days(year, m);
    if (get_leap_month(year))
        sum += get_leap_month_days(year);
    return sum;
}

static int days_from_1900_1_31(int year, int month, int day) {
    int total = 0;
    for (int y = 1900; y < year; y++)
        total += is_leap_year(y) ? 366 : 365;
    int mdays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap_year(year)) mdays[2] = 29;
    for (int m = 1; m < month; m++)
        total += mdays[m];
    total += day;
    total -= 31;
    return total;
}

void solar_to_lunar(int sy, int sm, int sd,
                    int *ly, int *lm, int *ld, int *is_leap) {
    *ly = *lm = *ld = *is_leap = 0;
    int offset = days_from_1900_1_31(sy, sm, sd);
    if (offset < 0) return;
    int year;
    for (year = 1900; year <= 2100; year++) {
        int yd = get_lunar_year_days(year);
        if (offset < yd) break;
        offset -= yd;
    }
    if (year > 2100) return;
    *ly = year;
    int lm_val = get_leap_month(year);
    for (int m = 1; m <= 12; m++) {
        int md = get_lunar_month_days(year, m);
        if (offset < md) {
            *lm = m; *ld = offset + 1; *is_leap = 0; return;
        }
        offset -= md;
        if (m == lm_val) {
            int lmd = get_leap_month_days(year);
            if (offset < lmd) {
                *lm = m; *ld = offset + 1; *is_leap = 1; return;
            }
            offset -= lmd;
        }
    }
    *lm = 12; *ld = offset + 1;
}

int get_zodiac_index(int month, int day) {
    static const int starts[] = {20, 19, 21, 20, 21, 22, 23, 23, 23, 24, 23, 22};
    if (day < starts[month - 1]) return (month + 8) % 12;
    return (month + 9) % 12;
}

int get_chinese_zodiac_index(int lunar_year) {
    return (lunar_year - 1900) % 12;
}
