/*
 * config.h — Configuration Management
 *
 * Provides a single config_read()/config_save() pair (DRY — all config
 * writes go through config_save). Also includes date argument parsing
 * (supports absolute YYYY-MM-DD, relative +Nd/+Nw/+Nm, today, tomorrow)
 * and timezone-aware current date retrieval.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "context.h"

/* --- Config persistence ---------------------------------------------------- */
void config_read(AppContext *ctx);
void config_save(const AppContext *ctx);    /* single save point for all config */

/* --- Date parsing -----------------------------------------------------------
 * Supported formats:
 *   YYYY-MM-DD    absolute date
 *   +Nd           N days from today
 *   +Nw           N weeks from today
 *   +Nm           N months from today
 *   today         today's date
 *   tomorrow      tomorrow's date                                           */
int  config_parse_date_arg(const AppContext *ctx, const char *arg,
                           int *year, int *month, int *day);

/* --- Current date (respects timezone setting) ------------------------------- */
void config_get_current_date(const AppContext *ctx, int *y, int *m, int *d);

#endif
