/*
 * config.c — Configuration Persistence & Date Argument Parsing
 *
 * Handles reading/writing .cal_config key-value pairs and parsing date
 * arguments from the command line (absolute, relative, today/tomorrow).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "context.h"
#include "calendar.h"
#include "config.h"
#include "i18n.h"

void config_read(AppContext *ctx) {
    config_t *c = &ctx->config;
    memset(c, 0, sizeof(*c));
    c->lang_idx = -1;
    strncpy(c->calendar, "default", 31);
    c->calendar[31] = '\0';

    char path[1024];
    ctx_get_path(ctx, path, sizeof(path), ".cal_config");
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fp = fopen(".cal_config", "r");
        if (!fp) return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(line, "week_start=", 11) == 0)
            c->week_start = (strcmp(line + 11, "mon") == 0) ? 1 : 0;
        else if (strncmp(line, "lang=", 5) == 0)
            c->lang_idx = lang_index_by_code(line + 5);
        else if (strncmp(line, "lunar=", 6) == 0) {
            if (strcmp(line + 6, "on") == 0) c->lunar_mode = 1;
            else if (strcmp(line + 6, "off") == 0) c->lunar_mode = 2;
        }
        else if (strncmp(line, "compact=", 8) == 0)
            c->compact_mode = (strcmp(line + 8, "on") == 0) ? 1 : 0;
        else if (strncmp(line, "tz=", 3) == 0) {
            if (strcmp(line + 3, "local") == 0) c->tz_is_set = 0;
            else { c->tz_offset = atoi(line + 3); c->tz_is_set = 1; }
        }
        else if (strncmp(line, "calendar=", 9) == 0) {
            strncpy(c->calendar, line + 9, 31);
            c->calendar[31] = '\0';
        }
    }
    fclose(fp);
}

void config_save(const AppContext *ctx) {
    char path[1024];
    ctx_get_path(ctx, path, sizeof(path), ".cal_config");
    FILE *fp = fopen(path, "w");
    if (!fp) return;
    const config_t *c = &ctx->config;
    fprintf(fp, "week_start=%s\n", c->week_start ? "mon" : "sun");
    if (c->lang_idx >= 0)
        fprintf(fp, "lang=%s\n", langs[c->lang_idx].code);
    const char *lunar_str[] = {"auto", "on", "off"};
    fprintf(fp, "lunar=%s\n", lunar_str[c->lunar_mode]);
    fprintf(fp, "compact=%s\n", c->compact_mode ? "on" : "off");
    if (c->tz_is_set) fprintf(fp, "tz=%+d\n", c->tz_offset);
    else fprintf(fp, "tz=local\n");
    fprintf(fp, "calendar=%s\n", c->calendar);
    fclose(fp);
}

void config_get_current_date(const AppContext *ctx, int *y, int *m, int *d) {
    time_t now = time(NULL);
    if (ctx->config.tz_is_set) {
        now += ctx->config.tz_offset * 3600;
        struct tm *t = gmtime(&now);
        *y = t->tm_year + 1900;
        *m = t->tm_mon + 1;
        *d = t->tm_mday;
    } else {
        struct tm *t = localtime(&now);
        *y = t->tm_year + 1900;
        *m = t->tm_mon + 1;
        *d = t->tm_mday;
    }
}

int config_parse_date_arg(const AppContext *ctx, const char *arg,
                          int *year, int *month, int *day) {
    if (strcmp(arg, "today") == 0) {
        config_get_current_date(ctx, year, month, day);
        return 1;
    }
    if (strcmp(arg, "tomorrow") == 0) {
        config_get_current_date(ctx, year, month, day);
        int jdn = date_to_jdn(*year, *month, *day) + 1;
        jdn_to_date(jdn, year, month, day);
        return 1;
    }
    if (arg[0] == '+' && strlen(arg) >= 3) {
        int n = atoi(arg + 1);
        char unit = arg[strlen(arg) - 1];
        config_get_current_date(ctx, year, month, day);
        if (unit == 'd') {
            int jdn = date_to_jdn(*year, *month, *day) + n;
            jdn_to_date(jdn, year, month, day);
        } else if (unit == 'w') {
            int jdn = date_to_jdn(*year, *month, *day) + n * 7;
            jdn_to_date(jdn, year, month, day);
        } else if (unit == 'm') {
            add_months(year, month, day, n);
        } else if (unit == 'y') {
            *year += n;
        }
        return 1;
    }
    if (sscanf(arg, "%d-%d-%d", year, month, day) == 3) return 1;
    return 0;
}
