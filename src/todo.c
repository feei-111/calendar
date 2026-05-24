/*
 * todo.c — Todo Item Management
 *
 * Implements todo CRUD, file persistence (todo.txt), undo/redo via file
 * copying, recurrence matching logic, and formatted list output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "calendar.h"
#include "config.h"
#include "todo.h"

/* === Line parsing (handles legacy 2-pipe and current 7-pipe formats) === */

void todo_parse_line(const char *line, todo_t *t) {
    memset(t, 0, sizeof(*t));
    strncpy(t->calendar, "default", 31);
    t->calendar[31] = '\0';
    strncpy(t->recur, "none", 15);
    t->recur[15] = '\0';

    const char *p = line;
    const char *fields[8];
    int fc = 0;
    fields[0] = p;
    for (int i = 0; p[i] && fc < 7; i++) {
        if (p[i] == '|') {
            fields[fc + 1] = p + i + 1;
            fc++;
        }
    }
    if (fc >= 7) {
        int len;
        len = (int)(fields[1] - fields[0] - 1);
        if (len > 15) len = 15;
        strncpy(t->date, fields[0], (size_t)len);
        t->date[len] = '\0';
        t->num = atoi(fields[1]);
        t->priority = atoi(fields[2]);
        len = (int)(fields[4] - fields[3] - 1);
        if (len > 127) len = 127;
        strncpy(t->tags, fields[3], (size_t)len);
        t->tags[len] = '\0';
        len = (int)(fields[5] - fields[4] - 1);
        if (len > 31) len = 31;
        strncpy(t->calendar, fields[4], (size_t)len);
        t->calendar[len] = '\0';
        len = (int)(fields[6] - fields[5] - 1);
        if (len > 15) len = 15;
        strncpy(t->recur, fields[5], (size_t)len);
        t->recur[len] = '\0';
        len = (int)(fields[7] - fields[6] - 1);
        if (len > 15) len = 15;
        strncpy(t->deadline, fields[6], (size_t)len);
        t->deadline[len] = '\0';
        strncpy(t->content, fields[7], 511);
        t->content[511] = '\0';
        char *nl = strchr(t->content, '\n');
        if (nl) *nl = '\0';
        nl = strchr(t->content, '\r');
        if (nl) *nl = '\0';
    } else if (fc >= 2) {
        int len = (int)(fields[1] - fields[0] - 1);
        if (len > 15) len = 15;
        strncpy(t->date, fields[0], (size_t)len);
        t->date[len] = '\0';
        t->num = atoi(fields[1]);
        strncpy(t->content, fields[2], 511);
        t->content[511] = '\0';
        char *nl = strchr(t->content, '\n');
        if (nl) *nl = '\0';
        nl = strchr(t->content, '\r');
        if (nl) *nl = '\0';
    }
}

void todo_load(AppContext *ctx) {
    ctx->todo_count = 0;
    char path[1024];
    ctx_get_path(ctx, path, sizeof(path), "todo.txt");
    FILE *fp = fopen(path, "r");
    if (!fp) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp) && ctx->todo_count < MAX_TODOS) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0') continue;
        todo_parse_line(line, &ctx->todos[ctx->todo_count]);
        if (ctx->todos[ctx->todo_count].date[0])
            ctx->todo_count++;
    }
    fclose(fp);
}

void todo_save(const AppContext *ctx) {
    char path[1024];
    ctx_get_path(ctx, path, sizeof(path), "todo.txt");
    FILE *fp = fopen(path, "w");
    if (!fp) return;
    for (int i = 0; i < ctx->todo_count; i++) {
        fprintf(fp, "%s|%d|%d|%s|%s|%s|%s|%s\n",
            ctx->todos[i].date, ctx->todos[i].num, ctx->todos[i].priority,
            ctx->todos[i].tags, ctx->todos[i].calendar, ctx->todos[i].recur,
            ctx->todos[i].deadline, ctx->todos[i].content);
    }
    fclose(fp);
}

void todo_undo_save(AppContext *ctx) {
    char path_src[1024], path_dst[1024];
    ctx_get_path(ctx, path_src, sizeof(path_src), "todo.txt");
    ctx_get_path(ctx, path_dst, sizeof(path_dst), ".cal_undo");

    FILE *src = fopen(path_src, "r");
    if (!src) { src = fopen(path_src, "w"); if (src) fclose(src); return; }
    FILE *dst = fopen(path_dst, "w");
    if (!dst) { fclose(src); return; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    fclose(src); fclose(dst);

    char redo_path[1024];
    ctx_get_path(ctx, redo_path, sizeof(redo_path), ".cal_redo");
    remove(redo_path);
}

void todo_undo_do(const char *from, const char *to) {
    FILE *src = fopen(from, "r");
    if (!src) return;
    FILE *dst = fopen(to, "w");
    if (!dst) { fclose(src); return; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    fclose(src); fclose(dst);
}

int todo_matches_date(const todo_t *t, int y, int m, int d) {
    int ty, tm, td;
    if (sscanf(t->date, "%d-%d-%d", &ty, &tm, &td) != 3) return 0;
    if (strcmp(t->recur, "none") == 0 || t->recur[0] == '\0')
        return (ty == y && tm == m && td == d);
    int tjdn = date_to_jdn(ty, tm, td);
    int djdn = date_to_jdn(y, m, d);
    if (djdn < tjdn) return 0;
    if (strcmp(t->recur, "weekly") == 0)
        return ((djdn - tjdn) % 7 == 0);
    if (strcmp(t->recur, "monthly") == 0)
        return (td == d);
    if (strcmp(t->recur, "yearly") == 0)
        return (tm == m && td == d);
    return 0;
}

int todo_has_for_date(const AppContext *ctx, int y, int m, int d) {
    for (int i = 0; i < ctx->todo_count; i++)
        if (todo_matches_date(&ctx->todos[i], y, m, d))
            return 1;
    return 0;
}

void todo_add(AppContext *ctx, const char *date, int priority,
              const char *tags, const char *calendar,
              const char *recur, const char *deadline,
              const char *content) {
    int max_n = 0;
    for (int i = 0; i < ctx->todo_count; i++)
        if (strcmp(ctx->todos[i].date, date) == 0 && ctx->todos[i].num > max_n)
            max_n = ctx->todos[i].num;

    todo_t *nt = &ctx->todos[ctx->todo_count];
    memset(nt, 0, sizeof(*nt));
    strncpy(nt->date, date, 15);     nt->date[15] = '\0';
    nt->num = max_n + 1;
    nt->priority = priority;
    strncpy(nt->tags, tags, 127);        nt->tags[127] = '\0';
    strncpy(nt->calendar, calendar, 31); nt->calendar[31] = '\0';
    strncpy(nt->recur, recur, 15);       nt->recur[15] = '\0';
    strncpy(nt->deadline, deadline, 15); nt->deadline[15] = '\0';
    strncpy(nt->content, content, 511);  nt->content[511] = '\0';
    ctx->todo_count++;
}

int todo_del(AppContext *ctx, const char *date, int num) {
    int found = -1;
    for (int i = 0; i < ctx->todo_count; i++) {
        if (strcmp(ctx->todos[i].date, date) == 0 && ctx->todos[i].num == num) {
            found = i; break;
        }
    }
    if (found < 0) return 0;
    for (int i = found; i < ctx->todo_count - 1; i++)
        ctx->todos[i] = ctx->todos[i + 1];
    ctx->todo_count--;
    return 1;
}

int todo_edit(AppContext *ctx, const char *date, int num,
              int priority, const char *tags, const char *calendar,
              const char *recur, const char *deadline,
              const char *content) {
    int found = -1;
    for (int i = 0; i < ctx->todo_count; i++) {
        if (strcmp(ctx->todos[i].date, date) == 0 && ctx->todos[i].num == num) {
            found = i; break;
        }
    }
    if (found < 0) return 0;
    todo_t *t = &ctx->todos[found];
    if (priority >= 0) t->priority = priority;
    if (tags)          { strncpy(t->tags, tags, 127); t->tags[127] = '\0'; }
    if (calendar)      { strncpy(t->calendar, calendar, 31); t->calendar[31] = '\0'; }
    if (recur)         { strncpy(t->recur, recur, 15); t->recur[15] = '\0'; }
    if (deadline)      { strncpy(t->deadline, deadline, 15); t->deadline[15] = '\0'; }
    if (content)       { strncpy(t->content, content, 511); t->content[511] = '\0'; }
    return 1;
}

void todo_list(const AppContext *ctx, const char *filter_month,
               const char *filter_cal, const char *filter_tag,
               int filter_pri) {
    int cy, cm, cd;
    config_get_current_date(ctx, &cy, &cm, &cd);

    int count = 0;
    for (int i = 0; i < ctx->todo_count; i++) {
        const todo_t *t = &ctx->todos[i];
        if (filter_month && strncmp(t->date, filter_month, strlen(filter_month)) != 0)
            continue;
        if (filter_cal && strcmp(t->calendar, filter_cal) != 0) continue;
        if (filter_tag && !strstr(t->tags, filter_tag)) continue;
        if (filter_pri >= 0 && t->priority != filter_pri) continue;

        const char *pstr = "  ";
        if (t->priority == 1) pstr = COLOR_RED " !" COLOR_RESET;
        else if (t->priority == 2) pstr = COLOR_YELLOW " *" COLOR_RESET;
        else if (t->priority == 3) pstr = COLOR_BLUE " -" COLOR_RESET;

        printf("  %s %s [%d] %s", pstr, t->date, t->num, t->content);
        if (t->tags[0])
            printf(" %s(%s)%s", COLOR_CYAN, t->tags, COLOR_RESET);
        if (t->calendar[0] && strcmp(t->calendar, "default") != 0)
            printf(" %s{%s}%s", COLOR_MAGENTA, t->calendar, COLOR_RESET);
        if (strcmp(t->recur, "none") != 0 && t->recur[0])
            printf(" %s[%s]%s", COLOR_DIM, t->recur, COLOR_RESET);
        if (t->deadline[0]) {
            int dly, dlm, dld;
            if (sscanf(t->deadline, "%d-%d-%d", &dly, &dlm, &dld) == 3) {
                int dljdn = date_to_jdn(dly, dlm, dld);
                int cjdn = date_to_jdn(cy, cm, cd);
                int diff = dljdn - cjdn;
                if (diff > 0)
                    printf(" %s(%d)%s", COLOR_YELLOW, diff, COLOR_RESET);
                else if (diff == 0)
                    printf(" %s(%s)%s", COLOR_RED, ctx->lang->today_str, COLOR_RESET);
                else
                    printf(" %s(%s)%s", COLOR_RED, ctx->lang->overdue_str, COLOR_RESET);
            }
        }
        printf("\n");
        count++;
    }
    if (count == 0)
        printf("%s\n", ctx->lang->no_todo_str);
}
