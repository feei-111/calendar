/*
 * ui.h — Terminal Rendering & Display Engine
 *
 * Responsible for all terminal output: calendar views (month/year/week),
 * festival/holiday loading, text wrapping for narrow cells, and
 * general-purpose display helpers (width calculation, centered printing).
 * All functions receive an AppContext pointer — no hidden globals.
 */

#ifndef UI_H
#define UI_H

#include "context.h"

/* --- Low-level display utilities ------------------------------------------- */
int  ui_display_width(const char *s);       /* column count for UTF-8 string  */
void ui_spaces(int n);                      /* print n spaces                  */
void ui_centered(const char *text, int width, const char *color);
void ui_day_cell(int day, int has_todo, const char *color, int width);

/* --- Text wrapping structure ------------------------------------------------
 * Holds up to MAX_WRAP_LINES lines of a split festival/holiday name.
 * Used by ui_print_month when text exceeds the cell width.                    */
typedef struct {
    char lines[MAX_WRAP_LINES][128];
    int  count;
} wrap_t;
wrap_t ui_wrap_text(const char *text, int max_w);

/* --- Festival & holiday data loading --------------------------------------- */
void ui_load_festivals(AppContext *ctx);       /* reads lang/festival_*.txt   */
void ui_load_holidays(AppContext *ctx, int year, int month);

/* --- Day information lookup ------------------------------------------------
 * Fills buf with the display string for date (sy,sm,sd). Priority:
 * 1) holiday from holiday cache  2) solar festival  3) lunar festival
 * 4) lunar month/day name. Sets *out_is_festival when a festival is found.   */
void ui_get_day_info(const AppContext *ctx, int sy, int sm, int sd,
                     char *buf, size_t bufsz, int *out_is_festival);

/* --- Dynamic cell width -----------------------------------------------------
 * Scans all days in the month to determine the narrowest column width
 * that fits all text without overflow (capped at MAX_CELL_W).                 */
int  ui_calc_cell_width(const AppContext *ctx, int year, int month);

/* --- Calendar views -------------------------------------------------------- */
void ui_print_month(AppContext *ctx, int year, int month, int week_start);
void ui_print_year(AppContext *ctx, int year, int week_start);
void ui_print_week(AppContext *ctx, int year, int month, int day, int week_start);

/* --- Other output ---------------------------------------------------------- */
void ui_print_help(const AppContext *ctx);
void ui_print_zodiac(const AppContext *ctx, int year, int month, int day);

#endif
