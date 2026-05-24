/*
 * ui.c — Terminal Rendering & Display Engine
 *
 * All terminal output lives here: month/year/week calendar views, festival
 * data loading, text wrapping for narrow cells, and low-level display helpers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "context.h"
#include "calendar.h"
#include "todo.h"
#include "config.h"
#include "ui.h"

/* === Low-level display helpers ========================================== */

int ui_display_width(const char *s) {
    int w = 0;
    while (*s) {
        unsigned char c = (unsigned char)*s;
        if (c >= 0x80) {
            if      ((c & 0xE0) == 0xC0) { s += 2; w += 1; }
            else if ((c & 0xF0) == 0xE0) { s += 3; w += 2; }
            else if ((c & 0xF8) == 0xF0) { s += 4; w += 2; }
            else                         { s++;    w++;    }
        } else { s++; w++; }
    }
    return w;
}

void ui_spaces(int n) {
    for (int i = 0; i < n; i++) putchar(' ');
}

void ui_centered(const char *text, int width, const char *color) {
    if (!text || !*text) { ui_spaces(width); return; }
    int dw = ui_display_width(text);
    if (dw >= width) {
        if (color[0]) printf("%s", color);
        printf("%s", text);
        if (color[0]) printf("%s", COLOR_RESET);
        return;
    }
    int pad = width - dw;
    int left = pad / 2;
    ui_spaces(left);
    if (color[0]) printf("%s", color);
    printf("%s", text);
    if (color[0]) printf("%s", COLOR_RESET);
    ui_spaces(pad - left);
}

void ui_day_cell(int day, int has_todo, const char *color, int width) {
    char num[8];
    snprintf(num, sizeof(num), "%d", day);
    int nw = (int)strlen(num);
    int pad = width - nw;
    int left = pad / 2;
    int right = pad - left;
    ui_spaces(left);
    if (color[0]) printf("%s", color);
    printf("%s", num);
    if (color[0]) printf("%s", COLOR_RESET);
    if (has_todo && right > 0) {
        printf("%s*%s", COLOR_YELLOW, COLOR_RESET);
        right--;
    }
    ui_spaces(right);
}

wrap_t ui_wrap_text(const char *text, int max_w) {
    wrap_t w;
    w.count = 0;
    if (!text || !*text) { w.lines[0][0] = 0; w.count = 1; return w; }
    int dw = ui_display_width(text);
    if (dw <= max_w) {
        strncpy(w.lines[0], text, 127); w.lines[0][127] = 0;
        w.count = 1; return w;
    }
    const char *p = text;
    int line = 0;
    while (*p && line < MAX_WRAP_LINES) {
        int cur_w = 0;
        const char *seg_start = p;
        const char *seg_end = p;
        while (*p) {
            unsigned char c = (unsigned char)*p;
            int cw = 1, cb = 1;
            if (c >= 0x80) {
                if ((c & 0xE0) == 0xC0)      { cb = 2; cw = 1; }
                else if ((c & 0xF0) == 0xE0) { cb = 3; cw = 2; }
                else if ((c & 0xF8) == 0xF0) { cb = 4; cw = 2; }
                else                         { cb = 1; cw = 1; }
            }
            if (cur_w + cw > max_w) break;
            cur_w += cw;
            seg_end = p + cb;
            p += cb;
        }
        int len = (int)(seg_end - seg_start);
        if (len > 127) len = 127;
        strncpy(w.lines[line], seg_start, (size_t)len);
        w.lines[line][len] = 0;
        line++;
    }
    w.count = line > 0 ? line : 1;
    return w;
}

/* === Festival & holiday data loading ==================================== */

void ui_load_festivals(AppContext *ctx) {
    ctx->solar_fest_count = 0;
    ctx->lunar_fest_count = 0;
    strncpy(ctx->chuxi_name, "除夕", 63); ctx->chuxi_name[63] = 0;

    char path[1024], line[MAX_LINE];
    int solar_ok = 0, lunar_ok = 0;

    /* ---- solar festivals: data dir/lang → root lang/ fallback --------- */
    char rel[256], lang_rel[288];
    snprintf(rel, sizeof(rel), "festival_solar_%s.txt", ctx->lang->code);
    snprintf(lang_rel, sizeof(lang_rel), "lang/%s", rel);
    ctx_get_path(ctx, path, sizeof(path), lang_rel);
    FILE *fp = fopen(path, "r");
    if (!fp) fp = fopen(lang_rel, "r");
    solar_ok = (fp != NULL);
    if (fp) {
        while (fgets(line, sizeof(line), fp) && ctx->solar_fest_count < MAX_FESTIVALS) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) < 6 || line[2] != '-') continue;
            int m = atoi(line), d = atoi(line + 3);
            char *name = line + 6;
            while (*name == ' ') name++;
            if (m >= 1 && m <= 12 && d >= 1 && d <= 31 && *name) {
                ctx->solar_fest[ctx->solar_fest_count].month = m;
                ctx->solar_fest[ctx->solar_fest_count].day = d;
                strncpy(ctx->solar_fest[ctx->solar_fest_count].name, name, 63);
                ctx->solar_fest[ctx->solar_fest_count].name[63] = 0;
                ctx->solar_fest_count++;
            }
        }
        fclose(fp);
    }

    /* ---- lunar festivals: data dir/lang → root lang/ fallback --------- */
    snprintf(rel, sizeof(rel), "festival_lunar_%s.txt", ctx->lang->code);
    snprintf(lang_rel, sizeof(lang_rel), "lang/%s", rel);
    ctx_get_path(ctx, path, sizeof(path), lang_rel);
    fp = fopen(path, "r");
    if (!fp) fp = fopen(lang_rel, "r");
    lunar_ok = (fp != NULL);
    if (fp) {
        while (fgets(line, sizeof(line), fp) && ctx->lunar_fest_count < MAX_FESTIVALS) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) < 6 || line[2] != '-') continue;
            int m = atoi(line), d = atoi(line + 3);
            char *name = line + 6;
            while (*name == ' ') name++;
            if (*name) {
                if (m == 0 && d == 0) {
                    strncpy(ctx->chuxi_name, name, 63); ctx->chuxi_name[63] = 0;
                    continue;
                }
                if (m >= 1 && m <= 12 && d >= 1 && d <= 30) {
                    ctx->lunar_fest[ctx->lunar_fest_count].month = m;
                    ctx->lunar_fest[ctx->lunar_fest_count].day = d;
                    strncpy(ctx->lunar_fest[ctx->lunar_fest_count].name, name, 63);
                    ctx->lunar_fest[ctx->lunar_fest_count].name[63] = 0;
                    ctx->lunar_fest_count++;
                }
            }
        }
        fclose(fp);
    }

    /* ---- warning if both solar + lunar files are missing --------------- */
    if (!solar_ok && !lunar_ok) {
        snprintf(rel, sizeof(rel), "festival_*_%s.txt", ctx->lang->code);
        printf(ctx->lang->fest_missing_str, rel);
        printf("\n");
    }
}

void ui_load_holidays(AppContext *ctx, int year, int month) {
    memset(ctx->holiday_cache, 0, sizeof(ctx->holiday_cache));
    memset(ctx->holiday_valid, 0, sizeof(ctx->holiday_valid));

    char path[1024], rel[256], lang_rel[288];
    snprintf(rel, sizeof(rel), "holiday_%s.txt", ctx->lang->code);
    snprintf(lang_rel, sizeof(lang_rel), "lang/%s", rel);
    ctx_get_path(ctx, path, sizeof(path), lang_rel);
    FILE *fp = fopen(path, "r");
    if (!fp) fp = fopen(lang_rel, "r");
    if (!fp) return;

    char line[MAX_LINE], prefix[16];
    snprintf(prefix, sizeof(prefix), "%04d-%02d", year, month);
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(line, prefix, 7) == 0 && strlen(line) > 11 && line[7] == '-') {
            int d = atoi(line + 8);
            if (d >= 1 && d <= 31) {
                strncpy(ctx->holiday_cache[d], line + 11, 63);
                ctx->holiday_cache[d][63] = 0;
                ctx->holiday_valid[d] = 1;
            }
        }
    }
    fclose(fp);
}

static const char *ui_find_solar_festival(const AppContext *ctx, int month, int day) {
    for (int i = 0; i < ctx->solar_fest_count; i++)
        if (ctx->solar_fest[i].month == month && ctx->solar_fest[i].day == day)
            return ctx->solar_fest[i].name;
    return NULL;
}

static const char *ui_find_lunar_festival(const AppContext *ctx, int month, int day) {
    for (int i = 0; i < ctx->lunar_fest_count; i++)
        if (ctx->lunar_fest[i].month == month && ctx->lunar_fest[i].day == day)
            return ctx->lunar_fest[i].name;
    return NULL;
}

void ui_get_day_info(const AppContext *ctx, int sy, int sm, int sd,
                     char *buf, size_t bufsz, int *out_is_festival) {
    *out_is_festival = 0;

    if (sd >= 1 && sd <= 31 && ctx->holiday_valid[sd]) {
        *out_is_festival = 1;
        strncpy(buf, ctx->holiday_cache[sd], bufsz - 1);
        buf[bufsz - 1] = 0;
        return;
    }

    const char *sf = ui_find_solar_festival(ctx, sm, sd);
    if (sf) { *out_is_festival = 1; strncpy(buf, sf, bufsz - 1); buf[bufsz - 1] = 0; return; }

    int lmode = ctx->config.lunar_mode;
    int should_lunar;
    if (lmode == 1) should_lunar = 1;
    else if (lmode == 2) should_lunar = 0;
    else {
        int idx = (int)(ctx->lang - langs);
        should_lunar = (idx == 0 || idx == 2);
    }
    if (!should_lunar) { buf[0] = 0; return; }

    int ly, lm, ld, isl;
    solar_to_lunar(sy, sm, sd, &ly, &lm, &ld, &isl);
    if (lm == 0) { buf[0] = 0; return; }

    if (lm == 12 && !isl) {
        int mdays = get_lunar_month_days(ly, 12);
        if (ld == mdays) {
            *out_is_festival = 1;
            strncpy(buf, ctx->chuxi_name, bufsz - 1);
            buf[bufsz - 1] = 0;
            return;
        }
    }

    const char *lf = ui_find_lunar_festival(ctx, lm, ld);
    if (lf) { *out_is_festival = 1; strncpy(buf, lf, bufsz - 1); buf[bufsz - 1] = 0; return; }

    if (ld == 1) {
        if (isl) snprintf(buf, bufsz, "%s%s", ctx->lang->leap_prefix, ctx->lang->lunar_months[lm - 1]);
        else     snprintf(buf, bufsz, "%s", ctx->lang->lunar_months[lm - 1]);
        return;
    }
    snprintf(buf, bufsz, "%s", ctx->lang->lunar_days[ld - 1]);
}

int ui_calc_cell_width(const AppContext *ctx, int year, int month) {
    if (ctx->config.compact_mode) return MIN_CELL_W;
    int dim = get_days_in_month(year, month);
    int max_w = MIN_CELL_W;
    for (int d = 1; d <= dim; d++) {
        int is_fest = 0;
        char buf[128];
        ui_get_day_info(ctx, year, month, d, buf, sizeof(buf), &is_fest);
        int w = ui_display_width(buf);
        if (w > max_w) max_w = w;
    }
    int cw = max_w + 2;
    return cw > MAX_CELL_W ? MAX_CELL_W : cw;
}

/* === Calendar views ===================================================== */

void ui_print_month(AppContext *ctx, int year, int month, int week_start) {
    ui_load_holidays(ctx, year, month);
    todo_load(ctx);

    printf("%s: %s\n", ctx->lang->week_start_label,
           ctx->lang->weekdays[week_start == 0 ? 0 : 1]);

    int cell_w = ui_calc_cell_width(ctx, year, month);
    int total_w = cell_w * 7;
    char title[32];
    snprintf(title, sizeof(title), " %04d-%02d ", year, month);
    int tw = (int)strlen(title);
    int pad = total_w - tw;
    int pl = pad / 2, pr = pad - pl;
    printf("%s", COLOR_BOLD);
    for (int i = 0; i < pl; i++) putchar('=');
    printf("%s", title);
    for (int i = 0; i < pr; i++) putchar('=');
    printf("%s\n", COLOR_RESET);

    for (int i = 0; i < 7; i++) {
        int weekday = (week_start + i) % 7;
        const char *c = (weekday == 0 || weekday == 6) ? COLOR_BLUE : "";
        ui_centered(ctx->lang->weekdays[(week_start + i) % 7], cell_w, c);
    }
    printf("\n");

    int first_day = get_first_weekday(year, month);
    int dim = get_days_in_month(year, month);
    int offset = (first_day - week_start + 7) % 7;

    int cy, cm, cd;
    config_get_current_date(ctx, &cy, &cm, &cd);

    int day = 1;
    for (int week = 0; week < 6; week++) {
        if (day > dim) break;
        int wd_arr[7];
        for (int col = 0; col < 7; col++) {
            if (week == 0 && col < offset) wd_arr[col] = 0;
            else if (day > dim)           wd_arr[col] = 0;
            else                           wd_arr[col] = day++;
        }
        for (int col = 0; col < 7; col++) {
            if (wd_arr[col] == 0) { ui_spaces(cell_w); }
            else {
                int d = wd_arr[col];
                int is_today = (year == cy && month == cm && d == cd);
                int is_fest = 0;
                char buf[128];
                ui_get_day_info(ctx, year, month, d, buf, sizeof(buf), &is_fest);
                int weekday = (week_start + col) % 7;
                int is_wkend = (weekday == 0 || weekday == 6);
                const char *color = "";
                if (is_today) color = COLOR_RED;
                else if (is_fest) color = COLOR_GREEN;
                else if (is_wkend) color = COLOR_BLUE;
                ui_day_cell(d, todo_has_for_date(ctx, year, month, d), color, cell_w);
            }
        }
        printf("\n");
        if (!ctx->config.compact_mode) {
            wrap_t wraps[7];
            const char *colors[7];
            int has_wrap = 0;
            for (int col = 0; col < 7; col++) {
                if (wd_arr[col] == 0) {
                    memset(&wraps[col], 0, sizeof(wraps[col]));
                    wraps[col].lines[0][0] = 0; wraps[col].count = 1;
                    colors[col] = "";
                } else {
                    int d = wd_arr[col];
                    int is_today = (year == cy && month == cm && d == cd);
                    int is_fest = 0;
                    char buf[128];
                    ui_get_day_info(ctx, year, month, d, buf, sizeof(buf), &is_fest);
                    int weekday = (week_start + col) % 7;
                    int is_wkend = (weekday == 0 || weekday == 6);
                    if (is_today) colors[col] = COLOR_RED;
                    else if (is_fest) colors[col] = COLOR_GREEN;
                    else if (is_wkend) colors[col] = COLOR_BLUE;
                    else colors[col] = "";
                    wraps[col] = ui_wrap_text(buf, cell_w - 1);
                    if (wraps[col].count > 1) has_wrap = 1;
                }
            }
            int max_lines = 1;
            if (has_wrap)
                for (int c = 0; c < 7; c++)
                    if (wraps[c].count > max_lines) max_lines = wraps[c].count;
            for (int ln = 0; ln < max_lines; ln++) {
                for (int c = 0; c < 7; c++) {
                    if (ln < wraps[c].count && wraps[c].lines[ln][0])
                        ui_centered(wraps[c].lines[ln], cell_w, colors[c]);
                    else
                        ui_spaces(cell_w);
                }
                printf("\n");
            }
        }
    }

    printf("\n%s\n", ctx->lang->todo_header);
    int tcount = 0;
    for (int i = 0; i < ctx->todo_count; i++) {
        int ty, tm, td;
        if (sscanf(ctx->todos[i].date, "%d-%d-%d", &ty, &tm, &td) == 3) {
            if (ty == year && tm == month) {
                const char *pstr = "  ";
                if (ctx->todos[i].priority == 1) pstr = COLOR_RED "! " COLOR_RESET;
                else if (ctx->todos[i].priority == 2) pstr = COLOR_YELLOW "* " COLOR_RESET;
                else if (ctx->todos[i].priority == 3) pstr = COLOR_BLUE "- " COLOR_RESET;
                printf("  %s%s [%d] %s", pstr, ctx->todos[i].date,
                       ctx->todos[i].num, ctx->todos[i].content);
                if (ctx->todos[i].tags[0])
                    printf(" (%s)", ctx->todos[i].tags);
                if (ctx->todos[i].deadline[0]) {
                    int dly, dlm, dld;
                    if (sscanf(ctx->todos[i].deadline, "%d-%d-%d", &dly, &dlm, &dld) == 3) {
                        int djdn = date_to_jdn(dly, dlm, dld);
                        int cjdn = date_to_jdn(cy, cm, cd);
                        int diff = djdn - cjdn;
                        if (diff > 0) {
                            printf(" [");
                            printf(ctx->lang->days_left_fmt, diff);
                            printf("]");
                        } else if (diff == 0)
                            printf(" [%s]", ctx->lang->today_str);
                        else
                            printf(" [%s%s%s]", COLOR_RED, ctx->lang->overdue_str, COLOR_RESET);
                    }
                }
                printf("\n");
                tcount++;
            }
        }
    }
    if (tcount == 0) printf("  %s\n", ctx->lang->none_str);
    printf("\n%s\n", ctx->lang->help_hint);
}

void ui_print_year(AppContext *ctx, int year, int week_start) {
    int cy, cm, cd;
    config_get_current_date(ctx, &cy, &cm, &cd);
    todo_load(ctx);

    char title[16];
    snprintf(title, sizeof(title), " %d ", year);
    int tw = (int)strlen(title);
    int total_w = 22 * 3 + 4;
    int pad = total_w - tw;
    int pl = pad / 2, pr = pad - pl;
    printf("%s", COLOR_BOLD);
    for (int i = 0; i < pl; i++) putchar('=');
    printf("%s", title);
    for (int i = 0; i < pr; i++) putchar('=');
    printf("%s\n\n", COLOR_RESET);

    for (int row = 0; row < 4; row++) {
        int months[3];
        for (int c = 0; c < 3; c++) months[c] = row * 3 + c + 1;

        for (int c = 0; c < 3; c++) {
            ui_centered(ctx->lang->month_names[months[c] - 1], 22, COLOR_BOLD);
            if (c < 2) printf("  ");
        }
        printf("\n");

        for (int c = 0; c < 3; c++) {
            for (int i = 0; i < 7; i++) {
                int wd = (week_start + i) % 7;
                if (wd == 0 || wd == 6)
                    printf("%s%s%s", COLOR_BLUE, ctx->lang->weekdays_short[wd], COLOR_RESET);
                else
                    printf("%s", ctx->lang->weekdays_short[wd]);
                if (i < 6) putchar(' ');
            }
            if (c < 2) printf("  ");
        }
        printf("\n");

        for (int c = 0; c < 3; c++)
            ui_load_holidays(ctx, year, months[c]);

        int offsets[3], dims[3];
        for (int c = 0; c < 3; c++) {
            int fd = get_first_weekday(year, months[c]);
            offsets[c] = (fd - week_start + 7) % 7;
            dims[c] = get_days_in_month(year, months[c]);
        }

        for (int week = 0; week < 6; week++) {
            for (int c = 0; c < 3; c++) {
                for (int col = 0; col < 7; col++) {
                    int day_num = week * 7 + col - offsets[c] + 1;
                    if (day_num < 1 || day_num > dims[c]) {
                        printf("  ");
                    } else {
                        int is_today = (year == cy && months[c] == cm && day_num == cd);
                        int weekday = (week_start + col) % 7;
                        int is_wkend = (weekday == 0 || weekday == 6);
                        int is_fest = 0;
                        char buf[128];
                        ui_get_day_info(ctx, year, months[c], day_num, buf, sizeof(buf), &is_fest);
                        const char *color = "";
                        if (is_today) color = COLOR_RED;
                        else if (is_fest) color = COLOR_GREEN;
                        else if (is_wkend) color = COLOR_BLUE;
                        if (color[0]) printf("%s", color);
                        printf("%2d", day_num);
                        if (color[0]) printf("%s", COLOR_RESET);
                    }
                    if (col < 6) putchar(' ');
                }
                if (c < 2) printf("  ");
            }
            printf("\n");
        }
        printf("\n");
    }
}

void ui_print_week(AppContext *ctx, int year, int month, int day, int week_start) {
    todo_load(ctx);
    int jdn = date_to_jdn(year, month, day);
    int dow = (jdn + 1) % 7;
    int ws = (week_start == 0) ? 0 : 1;
    int diff = (dow - ws + 7) % 7;
    int start_jdn = jdn - diff;

    int sy, sm, sd, ey, em, ed;
    jdn_to_date(start_jdn, &sy, &sm, &sd);
    jdn_to_date(start_jdn + 6, &ey, &em, &ed);

    int cy, cm, cd;
    config_get_current_date(ctx, &cy, &cm, &cd);

    printf("%s%s: %04d-%02d-%02d ~ %04d-%02d-%02d%s\n\n",
        COLOR_BOLD, ctx->lang->week_label, sy, sm, sd, ey, em, ed, COLOR_RESET);

    for (int i = 0; i < 7; i++) {
        int djdn = start_jdn + i;
        int dy, dm, dd;
        jdn_to_date(djdn, &dy, &dm, &dd);
        ui_load_holidays(ctx, dy, dm);

        int weekday = djdn % 7;
        int is_today = (dy == cy && dm == cm && dd == cd);
        int is_wkend = (weekday == 0 || weekday == 6);

        const char *day_color = "";
        if (is_today) day_color = COLOR_RED;
        else if (is_wkend) day_color = COLOR_BLUE;

        if (day_color[0]) printf("%s", day_color);
        printf("%s %02d", ctx->lang->weekdays[weekday], dd);
        if (day_color[0]) printf("%s", COLOR_RESET);

        int is_fest = 0;
        char buf[128];
        ui_get_day_info(ctx, dy, dm, dd, buf, sizeof(buf), &is_fest);
        if (buf[0]) {
            const char *ic = is_fest ? COLOR_GREEN : "";
            if (ic[0]) printf("%s", ic);
            printf("  %s", buf);
            if (ic[0]) printf("%s", COLOR_RESET);
        }
        printf("\n");

        for (int t = 0; t < ctx->todo_count; t++) {
            if (todo_matches_date(&ctx->todos[t], dy, dm, dd)) {
                const char *pstr = "  ";
                if (ctx->todos[t].priority == 1) pstr = COLOR_RED " !" COLOR_RESET;
                else if (ctx->todos[t].priority == 2) pstr = COLOR_YELLOW " *" COLOR_RESET;
                else if (ctx->todos[t].priority == 3) pstr = COLOR_BLUE " -" COLOR_RESET;
                printf("    %s %s", pstr, ctx->todos[t].content);
                if (ctx->todos[t].tags[0])
                    printf(" %s[%s]%s", COLOR_CYAN, ctx->todos[t].tags, COLOR_RESET);
                if (ctx->todos[t].calendar[0] && strcmp(ctx->todos[t].calendar, "default") != 0)
                    printf(" %s{%s}%s", COLOR_MAGENTA, ctx->todos[t].calendar, COLOR_RESET);
                if (ctx->todos[t].deadline[0]) {
                    int dly, dlm, dld;
                    if (sscanf(ctx->todos[t].deadline, "%d-%d-%d", &dly, &dlm, &dld) == 3) {
                        int dljdn = date_to_jdn(dly, dlm, dld);
                        int dl_diff = dljdn - date_to_jdn(cy, cm, cd);
                        if (dl_diff > 0)
                            printf(" %s(%d)%s", COLOR_YELLOW, dl_diff, COLOR_RESET);
                        else if (dl_diff == 0)
                            printf(" %s(%s)%s", COLOR_RED, ctx->lang->today_str, COLOR_RESET);
                        else
                            printf(" %s(%s)%s", COLOR_RED, ctx->lang->overdue_str, COLOR_RESET);
                    }
                }
                if (strcmp(ctx->todos[t].recur, "none") != 0 && ctx->todos[t].recur[0])
                    printf(" %s[%s]%s", COLOR_DIM, ctx->todos[t].recur, COLOR_RESET);
                printf("\n");
            }
        }
    }
}

void ui_print_help(const AppContext *ctx) {
    printf("%s", ctx->lang->help_text);
}

void ui_print_zodiac(const AppContext *ctx, int year, int month, int day) {
    int zi = get_zodiac_index(month, day);
    printf("%04d-%02d-%02d\n", year, month, day);
    printf("  %s: %s\n", ctx->lang->zodiac_label, ctx->lang->zodiac_signs[zi]);

    int ly, lm, ld, isl;
    solar_to_lunar(year, month, day, &ly, &lm, &ld, &isl);
    if (ly > 0) {
        int czi = get_chinese_zodiac_index(ly);
        printf("  %s: %s (%d)\n", ctx->lang->chinese_zodiac_label,
               ctx->lang->zodiac_animals[czi], ly);
    }
}
