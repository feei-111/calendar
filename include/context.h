/*
 * context.h — Application Context & Platform Abstraction
 *
 * Central module that owns the global application state (AppContext struct)
 * and provides cross-platform abstractions (console setup, UTF-8 argv,
 * data directory paths). All mutable state is encapsulated here and
 * passed by pointer throughout the program — no global variables exist.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdio.h>
#include <stdint.h>

/* --- ANSI escape codes for terminal colors --------------------------------- */
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_DIM     "\033[2m"
#define COLOR_RESET   "\033[0m"

/* --- Layout & capacity constants ------------------------------------------- */
#define MIN_CELL_W     8       /* minimum calendar cell width (display cols)    */
#define MAX_CELL_W     12      /* maximum cell width before text wraps          */
#define MAX_WRAP_LINES 4       /* max lines for wrapped festival text           */
#define MAX_LINE       2048    /* max bytes for a file line buffer              */
#define MAX_TODOS      4096    /* max number of todo items                      */
#define MAX_FESTIVALS  32      /* max solar/lunar festival entries per lang     */

/* --- Todo item structure ---------------------------------------------------- */
typedef struct {
    char date[16];             /* YYYY-MM-DD                                    */
    int  num;                  /* sequence number per date (1-based)            */
    int  priority;             /* 0=none 1=high 2=medium 3=low                  */
    char tags[128];            /* comma-separated tags                          */
    char calendar[32];         /* calendar name (default: "default")            */
    char recur[16];            /* none | weekly | monthly | yearly              */
    char deadline[16];         /* YYYY-MM-DD or empty                           */
    char content[512];         /* todo description                              */
} todo_t;

/* --- Festival entry (loaded from lang/ directory at startup) ----------------- */
typedef struct {
    char name[64];
    int  month;
    int  day;
} festival_t;

/* --- Configuration snapshot (read from .cal_config) ------------------------- */
typedef struct {
    int week_start;            /* 0=Sunday  1=Monday                            */
    int lang_idx;              /* index into langs[] array                      */
    int lunar_mode;            /* 0=auto  1=on  2=off                          */
    int compact_mode;          /* compact display toggle                        */
    int tz_offset;             /* UTC offset in hours (e.g. +8)                 */
    int tz_is_set;             /* 1 if user configured timezone                 */
    char calendar[32];         /* default calendar name                         */
} config_t;

/* include i18n definitions needed by AppContext below */
#include "i18n.h"

/* --- Application context (the single source of truth) -------------------------
 * All mutable program state lives here. No global variables anywhere.
 * Allocated on the heap in main() due to large todo array (~3 MB).           */
typedef struct {
    const lang_t *lang;        /* pointer into immutable langs[] table         */

    config_t      config;      /* configuration snapshot                       */

    todo_t        todos[MAX_TODOS];
    int           todo_count;

    festival_t    solar_fest[MAX_FESTIVALS];
    int           solar_fest_count;

    festival_t    lunar_fest[MAX_FESTIVALS];
    int           lunar_fest_count;

    char          chuxi_name[64];   /* localized "New Year's Eve" string       */

    char          holiday_cache[32][64]; /* holiday names keyed by day-of-month */
    int           holiday_valid[32];     /* 1 = entry present, 0 = empty       */

    char          data_dir[512];  /* where todo.txt / .cal_config live         */
} AppContext;

/* --- Lifecycle -------------------------------------------------------------- */
void ctx_init(AppContext *ctx);
void ctx_destroy(AppContext *ctx);

/* --- Platform abstraction --------------------------------------------------- */
void   ctx_setup_console(void);
char **ctx_utf8_argv(int *out_argc);
void   ctx_free_utf8_argv(int argc, char **argv);

/* --- Path construction ------------------------------------------------------
 * Builds "{data_dir}/{filename}" in buf. On Windows the root is
 * %APPDATA%\cal\, on Unix it's ~/.config/cal/.                               */
void ctx_get_path(const AppContext *ctx, char *buf, size_t bufsz,
                  const char *filename);

#endif
