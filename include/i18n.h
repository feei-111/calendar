/*
 * i18n.h — Internationalisation Module
 *
 * Defines the lang_t structure that holds every localisable string used by
 * the program and the static langs[] table containing all 6 languages.
 * All strings are immutable string literals — no heap allocations.
 */

#ifndef I18N_H
#define I18N_H

typedef struct {
    const char *code;              /* ISO 639-1 language code                  */
    const char *name;              /* human-readable name                      */

    /* --- UI labels --------------------------------------------------------- */
    const char *week_start_label;
    const char *weekdays[7];       /* full weekday names                       */
    const char *weekdays_short[7]; /* single/double-char for year view         */
    const char *month_names[12];   /* full month names for year view           */
    const char *lunar_months[12];  /* traditional lunar month names            */
    const char *lunar_days[30];    /* lunar day-of-month names                 */
    const char *leap_prefix;       /* prefix for leap-month name               */
    const char *todo_header;       /* "[This Month's Todos]" section title     */
    const char *none_str;          /* "None" or equivalent                     */
    const char *help_hint;         /* footer hint to show help                 */

    const char *help_text;         /* full help output (multi-line string)     */

    /* --- Operation feedback strings ---------------------------------------- */
    const char *added_str, *deleted_str, *edited_str;
    const char *not_found_str, *no_todo_str;
    const char *err_open_str, *err_write_str;
    const char *set_week_start_fmt;  /* printf format with one %s             */
    const char *set_lang_fmt;        /* printf format with one %s             */
    const char *invalid_month, *invalid_arg, *specify_month, *unknown_subcmd;

    /* --- Zodiac & lunar labels --------------------------------------------- */
    const char *zodiac_signs[12];     /* Western zodiac                        */
    const char *zodiac_animals[12];   /* Chinese zodiac (Rat, Ox, …)          */
    const char *priority_names[4];    /* none, high, med, low                  */
    const char *days_left_fmt;        /* printf format with one %d            */
    const char *today_str;            /* "TODAY" or equivalent                 */
    const char *overdue_str;          /* "OVERDUE" or equivalent               */

    /* --- Undo / redo ------------------------------------------------------- */
    const char *undo_done_str, *redo_done_str;
    const char *nothing_undo_str, *nothing_redo_str;

    /* --- Date & view labels ------------------------------------------------ */
    const char *invalid_date_str;      /* printf format with one %s            */
    const char *week_label;
    const char *zodiac_label, *chinese_zodiac_label;

    /* --- Config status prefixes --------------------------------------------- */
    const char *lunar_prefix, *compact_prefix, *tz_prefix;
    const char *tz_local_str, *calendar_prefix;
    const char *days_unit;             /* "days", "天", "日", ...            */

    /* --- One-line usage strings (printed when subcommands miss arguments) --- */
    const char *config_usage;
    const char *todo_usage;
    const char *datecalc_usage_str;
    const char *fest_missing_str;      /* printf format with one %s            */
} lang_t;

extern const lang_t langs[];   /* defined in i18n.c — all 6 languages          */
extern const int   NUM_LANGS;  /* number of entries in langs[]                 */

int lang_index_by_code(const char *code);
int detect_system_lang(void);   /* returns index into langs[] or 1 (English)   */

#endif
