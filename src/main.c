/*
 * main.c — Entry Point & CLI Argument Dispatch
 *
 * Allocates the AppContext on the heap, parses command-line arguments,
 * and dispatches to the appropriate subcommand handlers (calendar views,
 * todo management, config, tools, calendar management).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "calendar.h"
#include "config.h"
#include "todo.h"
#include "ui.h"

/* === Subcommand: config ================================================= */

static void manage_config_cmd(AppContext *ctx, int argc, char **argv) {
    if (argc < 3) { printf("%s\n", ctx->lang->config_usage); return; }

    if (strcmp(argv[2], "mon") == 0) {
        ctx->config.week_start = 1; config_save(ctx);
        printf(ctx->lang->set_week_start_fmt, ctx->lang->weekdays[1]);
        printf("\n");
    } else if (strcmp(argv[2], "sun") == 0) {
        ctx->config.week_start = 0; config_save(ctx);
        printf(ctx->lang->set_week_start_fmt, ctx->lang->weekdays[0]);
        printf("\n");
    } else if (strcmp(argv[2], "lang") == 0) {
        if (argc < 4) {
            printf("Current: %s (%s)\nAvailable:", ctx->lang->name, ctx->lang->code);
            for (int i = 0; i < NUM_LANGS; i++)
                printf(" %s(%s)", langs[i].code, langs[i].name);
            printf("\n");
            return;
        }
        int idx = lang_index_by_code(argv[3]);
        if (idx < 0) { printf("Invalid language code: %s\n", argv[3]); return; }
        ctx->config.lang_idx = idx;
        ctx->lang = &langs[idx];
        config_save(ctx);
        ui_load_festivals(ctx);
        printf(ctx->lang->set_lang_fmt, ctx->lang->name);
        printf("\n");
    } else if (strcmp(argv[2], "lunar") == 0) {
        if (argc < 4) {
            const char *mn[] = {"auto", "on", "off"};
            printf("%s%s\nUsage: cal config lunar <auto|on|off>\n",
                   ctx->lang->lunar_prefix, mn[ctx->config.lunar_mode]);
            return;
        }
        if (strcmp(argv[3], "auto") == 0) ctx->config.lunar_mode = 0;
        else if (strcmp(argv[3], "on") == 0) ctx->config.lunar_mode = 1;
        else if (strcmp(argv[3], "off") == 0) ctx->config.lunar_mode = 2;
        else { printf("Invalid. Use: auto|on|off\n"); return; }
        config_save(ctx);
        const char *mn[] = {"auto", "on", "off"};
        printf("%s%s\n", ctx->lang->lunar_prefix, mn[ctx->config.lunar_mode]);
    } else if (strcmp(argv[2], "compact") == 0) {
        if (argc < 4) {
            printf("%s%s\nUsage: cal config compact <on|off>\n",
                   ctx->lang->compact_prefix, ctx->config.compact_mode ? "on" : "off");
            return;
        }
        ctx->config.compact_mode = (strcmp(argv[3], "on") == 0) ? 1 : 0;
        config_save(ctx);
        printf("%s%s\n", ctx->lang->compact_prefix, ctx->config.compact_mode ? "on" : "off");
    } else if (strcmp(argv[2], "tz") == 0) {
        if (argc < 4) {
            if (ctx->config.tz_is_set)
                printf("%sUTC%+d\n", ctx->lang->tz_prefix, ctx->config.tz_offset);
            else
                printf("%s%s\n", ctx->lang->tz_prefix, ctx->lang->tz_local_str);
            printf("Usage: cal config tz <+/-N> | local\n");
            return;
        }
        if (strcmp(argv[3], "local") == 0)
            ctx->config.tz_is_set = 0;
        else {
            ctx->config.tz_offset = atoi(argv[3]);
            ctx->config.tz_is_set = 1;
        }
        config_save(ctx);
        if (ctx->config.tz_is_set)
            printf("%sUTC%+d\n", ctx->lang->tz_prefix, ctx->config.tz_offset);
        else
            printf("%s%s\n", ctx->lang->tz_prefix, ctx->lang->tz_local_str);
    } else if (strcmp(argv[2], "calendar") == 0) {
        if (argc < 4) {
            printf("%s%s\nUsage: cal config calendar <name>\n",
                   ctx->lang->calendar_prefix, ctx->config.calendar);
            return;
        }
        strncpy(ctx->config.calendar, argv[3], 31); ctx->config.calendar[31] = 0;
        config_save(ctx);
        printf("%s%s\n", ctx->lang->calendar_prefix, ctx->config.calendar);
    } else {
        printf("%s\n", ctx->lang->config_usage);
    }
}

static void manage_todo_cmd(AppContext *ctx, int argc, char **argv) {
    if (argc < 3) { printf("%s\n", ctx->lang->todo_usage); return; }

    if (strcmp(argv[2], "add") == 0) {
        int priority = 0;
        char tags[128] = "";
        char calendar[32];
        strncpy(calendar, ctx->config.calendar, 31); calendar[31] = 0;
        char recur[16];
        strncpy(recur, "none", 15); recur[15] = 0;
        char deadline[16] = "";

        int i = 3;
        while (i < argc && argv[i][0] == '-') {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                if (strcmp(argv[i+1], "high") == 0) priority = 1;
                else if (strcmp(argv[i+1], "med") == 0) priority = 2;
                else if (strcmp(argv[i+1], "low") == 0) priority = 3;
                i += 2;
            } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
                strncpy(tags, argv[i+1], 127); tags[127] = 0; i += 2;
            } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
                strncpy(calendar, argv[i+1], 31); calendar[31] = 0; i += 2;
            } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
                strncpy(recur, argv[i+1], 15); recur[15] = 0; i += 2;
            } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
                strncpy(deadline, argv[i+1], 15); deadline[15] = 0; i += 2;
            } else break;
        }
        if (i >= argc) { printf("cal todo add [options] DATE <content>\n"); return; }
        int year, month, day;
        if (!config_parse_date_arg(ctx, argv[i], &year, &month, &day)) {
            printf(ctx->lang->invalid_date_str, argv[i]); printf("\n"); return;
        }
        i++;
        char content[512] = "";
        for (int j = i; j < argc; j++) {
            if (j > i) strncat(content, " ", 511 - strlen(content));
            strncat(content, argv[j], 511 - strlen(content));
        }
        char date_str[16];
        snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", year, month, day);

        todo_load(ctx);
        todo_undo_save(ctx);
        todo_add(ctx, date_str, priority, tags, calendar, recur, deadline, content);
        todo_save(ctx);
        printf("%s: %s [%d] %s\n", ctx->lang->added_str, date_str,
               ctx->todos[ctx->todo_count - 1].num, content);
    } else if (strcmp(argv[2], "del") == 0) {
        if (argc < 5) { printf("cal todo del YYYY-MM-DD <N>\n"); return; }
        todo_load(ctx);
        todo_undo_save(ctx);
        if (todo_del(ctx, argv[3], atoi(argv[4]))) { todo_save(ctx);
            printf("%s: %s [%s]\n", ctx->lang->deleted_str, argv[3], argv[4]); }
        else printf("%s: %s [%s]\n", ctx->lang->not_found_str, argv[3], argv[4]);
    } else if (strcmp(argv[2], "edit") == 0) {
        if (argc < 6) { printf("cal todo edit YYYY-MM-DD <N> [options] [new content]\n"); return; }
        todo_load(ctx);
        int priority = -1;
        char *tags = NULL, *calendar = NULL, *recur = NULL, *deadline = NULL;
        char content[512] = "";
        int content_set = 0;
        int i = 5;
        while (i < argc && argv[i][0] == '-') {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                if (strcmp(argv[i+1], "high") == 0) priority = 1;
                else if (strcmp(argv[i+1], "med") == 0) priority = 2;
                else if (strcmp(argv[i+1], "low") == 0) priority = 3;
                i += 2;
            } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) { tags = argv[i+1]; i += 2; }
            else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) { calendar = argv[i+1]; i += 2; }
            else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) { recur = argv[i+1]; i += 2; }
            else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) { deadline = argv[i+1]; i += 2; }
            else break;
        }
        if (i < argc) {
            content[0] = 0; content_set = 1;
            for (int j = i; j < argc; j++) {
                if (j > i) strncat(content, " ", 511 - strlen(content));
                strncat(content, argv[j], 511 - strlen(content));
            }
        }
        todo_undo_save(ctx);
        if (todo_edit(ctx, argv[3], atoi(argv[4]), priority, tags, calendar, recur, deadline,
                      content_set ? content : NULL)) {
            todo_save(ctx);
            printf("%s: %s [%s] %s\n", ctx->lang->edited_str, argv[3], argv[4],
                   content_set ? content : ctx->todos[0].content);
        } else printf("%s: %s [%s]\n", ctx->lang->not_found_str, argv[3], argv[4]);
    } else if (strcmp(argv[2], "list") == 0) {
        char *filter_month = NULL, *filter_cal = NULL, *filter_tag = NULL;
        int filter_pri = -1;
        int i = 3;
        while (i < argc) {
            if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) { filter_cal = argv[i+1]; i += 2; }
            else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) { filter_tag = argv[i+1]; i += 2; }
            else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                if (strcmp(argv[i+1], "high") == 0) filter_pri = 1;
                else if (strcmp(argv[i+1], "med") == 0) filter_pri = 2;
                else if (strcmp(argv[i+1], "low") == 0) filter_pri = 3;
                i += 2;
            } else if (argv[i][0] != '-') { filter_month = argv[i]; i++; }
            else i++;
        }
        todo_load(ctx);
        todo_list(ctx, filter_month, filter_cal, filter_tag, filter_pri);
    } else {
        printf("%s: %s\n", ctx->lang->unknown_subcmd, argv[2]);
    }
}

static void datecalc_cmd(AppContext *ctx, int argc, char **argv) {
    if (argc < 4) { printf("%s\n", ctx->lang->datecalc_usage_str); return; }
    int y1, m1, d1;
    if (!config_parse_date_arg(ctx, argv[2], &y1, &m1, &d1)) {
        printf(ctx->lang->invalid_date_str, argv[2]); printf("\n"); return;
    }
    if (argv[3][0] == '+' || argv[3][0] == '-') {
        int n = atoi(argv[3] + 1);
        char unit = argv[3][strlen(argv[3]) - 1];
        int ry = y1, rm = m1, rd = d1;
        if (argv[3][0] == '-') n = -n;
        if (unit == 'd') {
            int jdn = date_to_jdn(ry, rm, rd) + n;
            jdn_to_date(jdn, &ry, &rm, &rd);
        } else if (unit == 'w') {
            int jdn = date_to_jdn(ry, rm, rd) + n * 7;
            jdn_to_date(jdn, &ry, &rm, &rd);
        } else if (unit == 'm') {
            add_months(&ry, &rm, &rd, n);
        } else if (unit == 'y') {
            ry += n;
        }
        printf("%04d-%02d-%02d %s %s = %04d-%02d-%02d\n",
               y1, m1, d1, argv[3][0] == '+' ? "+" : "-", argv[3] + 1, ry, rm, rd);
    } else {
        int y2, m2, d2;
        if (!config_parse_date_arg(ctx, argv[3], &y2, &m2, &d2)) {
            printf(ctx->lang->invalid_date_str, argv[3]); printf("\n"); return;
        }
        int diff = date_to_jdn(y2, m2, d2) - date_to_jdn(y1, m1, d1);
        printf("%04d-%02d-%02d -> %04d-%02d-%02d: %d %s\n",
               y1, m1, d1, y2, m2, d2, diff, ctx->lang->days_unit);
    }
}

int main(int argc, char *argv[]) {
    ctx_setup_console();

#ifdef _WIN32
    char **migrated_argv = ctx_utf8_argv(&argc);
    if (migrated_argv) argv = migrated_argv;
#endif

    AppContext *ctx = (AppContext *)malloc(sizeof(AppContext));
    if (!ctx) return 1;
    memset(ctx, 0, sizeof(*ctx));
    ctx_init(ctx);

    if (argc == 1) {
        int y, m, d;
        config_get_current_date(ctx, &y, &m, &d);
        ui_print_month(ctx, y, m, ctx->config.week_start);
    } else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 ||
               strcmp(argv[1], "--help") == 0) {
        ui_print_help(ctx);
    } else if (strcmp(argv[1], "mon") == 0) {
        int y, m, d;
        config_get_current_date(ctx, &y, &m, &d);
        ui_print_month(ctx, y, m, 1);
    } else if (strcmp(argv[1], "sun") == 0) {
        int y, m, d;
        config_get_current_date(ctx, &y, &m, &d);
        ui_print_month(ctx, y, m, 0);
    } else if (strcmp(argv[1], "year") == 0) {
        int y, m, d;
        config_get_current_date(ctx, &y, &m, &d);
        if (argc >= 3) y = atoi(argv[2]);
        ui_print_year(ctx, y, ctx->config.week_start);
    } else if (strcmp(argv[1], "week") == 0) {
        int y, m, d;
        config_get_current_date(ctx, &y, &m, &d);
        if (argc >= 3) {
            if (!config_parse_date_arg(ctx, argv[2], &y, &m, &d)) {
                printf(ctx->lang->invalid_date_str, argv[2]); printf("\n");
#ifdef _WIN32
                ctx_free_utf8_argv(argc, migrated_argv);
#endif
                return 1;
            }
        }
        ui_print_week(ctx, y, m, d, ctx->config.week_start);
    } else if (strcmp(argv[1], "todo") == 0) {
        manage_todo_cmd(ctx, argc, argv);
    } else if (strcmp(argv[1], "config") == 0) {
        manage_config_cmd(ctx, argc, argv);
    } else if (strcmp(argv[1], "datecalc") == 0) {
        datecalc_cmd(ctx, argc, argv);
    } else if (strcmp(argv[1], "zodiac") == 0) {
        int y, m, d;
        config_get_current_date(ctx, &y, &m, &d);
        if (argc >= 3) {
            if (!config_parse_date_arg(ctx, argv[2], &y, &m, &d)) {
                printf(ctx->lang->invalid_date_str, argv[2]); printf("\n");
#ifdef _WIN32
                ctx_free_utf8_argv(argc, migrated_argv);
#endif
                return 1;
            }
        }
        ui_print_zodiac(ctx, y, m, d);
    } else if (strcmp(argv[1], "undo") == 0) {
        char undo_path[1024], redo_path[1024];
        ctx_get_path(ctx, undo_path, sizeof(undo_path), ".cal_undo");
        ctx_get_path(ctx, redo_path, sizeof(redo_path), ".cal_redo");
        FILE *ftest = fopen(undo_path, "r");
        if (!ftest) { printf("%s\n", ctx->lang->nothing_undo_str); }
        else {
            fclose(ftest);
            char todo_path[1024];
            ctx_get_path(ctx, todo_path, sizeof(todo_path), "todo.txt");
            todo_undo_do(todo_path, redo_path);
            todo_undo_do(undo_path, todo_path);
            remove(undo_path);
            printf("%s\n", ctx->lang->undo_done_str);
        }
    } else if (strcmp(argv[1], "redo") == 0) {
        char undo_path[1024], redo_path[1024];
        ctx_get_path(ctx, undo_path, sizeof(undo_path), ".cal_undo");
        ctx_get_path(ctx, redo_path, sizeof(redo_path), ".cal_redo");
        FILE *ftest = fopen(redo_path, "r");
        if (!ftest) { printf("%s\n", ctx->lang->nothing_redo_str); }
        else {
            fclose(ftest);
            char todo_path[1024];
            ctx_get_path(ctx, todo_path, sizeof(todo_path), "todo.txt");
            todo_undo_do(todo_path, undo_path);
            todo_undo_do(redo_path, todo_path);
            remove(redo_path);
            printf("%s\n", ctx->lang->redo_done_str);
        }
    } else if (strcmp(argv[1], "calendar") == 0) {
        if (argc < 3) { printf("cal calendar <list|rename|delete> ...\n"); }
        else if (strcmp(argv[2], "list") == 0) {
            todo_load(ctx);
            typedef struct { char name[32]; int count; } cs_t;
            cs_t stats[64]; int ns = 0;
            for (int i = 0; i < ctx->todo_count; i++) {
                const char *cn = ctx->todos[i].calendar;
                if (!cn[0]) cn = "default";
                int fi = -1;
                for (int j = 0; j < ns; j++) if (strcmp(stats[j].name, cn) == 0) { fi = j; break; }
                if (fi >= 0) stats[fi].count++;
                else if (ns < 64) { strncpy(stats[ns].name, cn, 31); stats[ns].name[31] = 0; stats[ns].count = 1; ns++; }
            }
            if (ns == 0) { printf("%s\n", ctx->lang->none_str); }
            else {
                for (int i = 0; i < ns; i++) {
                    int is_def = (strcmp(stats[i].name, ctx->config.calendar) == 0);
                    printf("  %s%s%s - %d", is_def ? COLOR_BOLD : "", stats[i].name, is_def ? COLOR_RESET : "", stats[i].count);
                    if (strcmp(stats[i].name, "default") == 0) printf(" (default)");
                    if (is_def) printf(" *");
                    printf("\n");
                }
            }
        } else if (strcmp(argv[2], "rename") == 0) {
            if (argc < 5) { printf("cal calendar rename <old-name> <new-name>\n"); }
            else {
                todo_load(ctx);
                todo_undo_save(ctx);
                int rn = 0;
                for (int i = 0; i < ctx->todo_count; i++) {
                    if (strcmp(ctx->todos[i].calendar, argv[3]) == 0) {
                        strncpy(ctx->todos[i].calendar, argv[4], 31); ctx->todos[i].calendar[31] = 0; rn++;
                    }
                }
                if (rn > 0) { todo_save(ctx); printf("Renamed %d todos\n", rn); }
                else printf("Calendar '%s' not found\n", argv[3]);
            }
        } else if (strcmp(argv[2], "delete") == 0) {
            if (argc < 4) { printf("cal calendar delete <name>\n"); }
            else {
                if (strcmp(argv[3], "default") == 0) { printf("Cannot delete default calendar\n"); }
                else {
                    todo_load(ctx);
                    todo_undo_save(ctx);
                    int nc = 0, del = 0;
                    for (int i = 0; i < ctx->todo_count; i++) {
                        if (strcmp(ctx->todos[i].calendar, argv[3]) == 0) del++;
                        else ctx->todos[nc++] = ctx->todos[i];
                    }
                    ctx->todo_count = nc;
                    if (del > 0) { todo_save(ctx); printf("Deleted %d todos\n", del); }
                    else printf("Calendar '%s' not found\n", argv[3]);
                }
            }
        } else printf("%s: %s\n", ctx->lang->unknown_subcmd, argv[2]);
    } else {
        int year = atoi(argv[1]);
        if (year > 0 && argc >= 3) {
            int month = atoi(argv[2]);
            if (month >= 1 && month <= 12) {
                int ws = ctx->config.week_start;
                if (argc >= 4) {
                    if (strcmp(argv[3], "mon") == 0) ws = 1;
                    else if (strcmp(argv[3], "sun") == 0) ws = 0;
                }
                ui_print_month(ctx, year, month, ws);
            } else printf("%s: %d\n", ctx->lang->invalid_month, month);
        } else if (year > 0 && argc == 2) {
            printf("%s\n", ctx->lang->specify_month);
        } else {
            printf("%s\n", ctx->lang->invalid_arg);
        }
    }

    ctx_destroy(ctx);
    free(ctx);
#ifdef _WIN32
    ctx_free_utf8_argv(argc, migrated_argv);
#endif
    return 0;
}
