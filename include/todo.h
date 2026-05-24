/*
 * todo.h — Todo Item Management
 *
 * Handles parsing, loading, saving, CRUD operations, recurrence matching,
 * and undo/redo for todo items. All data lives in AppContext.todos[].
 *
 * File format (backward-compatible):
 *   YYYY-MM-DD|N|priority|tags|calendar|recur|deadline|content    (new)
 *   YYYY-MM-DD|N|content                                          (legacy)
 */

#ifndef TODO_H
#define TODO_H

#include "context.h"

/* --- Persistence ----------------------------------------------------------- */
void todo_load(AppContext *ctx);
void todo_save(const AppContext *ctx);

/* --- Undo / redo ----------------------------------------------------------- */
void todo_undo_save(AppContext *ctx);          /* backup before mutation       */
void todo_undo_do(const char *from, const char *to);  /* copy file from->to   */

/* --- Recurrence check -------------------------------------------------------
 * Returns 1 if todo 't' applies to the given solar date, taking its
 * recur field (none/weekly/monthly/yearly) into account.                      */
int  todo_matches_date(const todo_t *t, int y, int m, int d);
int  todo_has_for_date(const AppContext *ctx, int y, int m, int d);

/* --- CRUD ------------------------------------------------------------------ */
void todo_add(AppContext *ctx, const char *date, int priority,
              const char *tags, const char *calendar,
              const char *recur, const char *deadline,
              const char *content);
int  todo_del(AppContext *ctx, const char *date, int num);
int  todo_edit(AppContext *ctx, const char *date, int num,
               int priority, const char *tags, const char *calendar,
               const char *recur, const char *deadline,
               const char *content);
void todo_list(const AppContext *ctx, const char *filter_month,
               const char *filter_cal, const char *filter_tag,
               int filter_pri);

/* --- Parsing ----------------------------------------------------------------
 * Parses one line of todo.txt into a todo_t. Handles both current (7 pipes)
 * and legacy (2 pipes) formats.                                               */
void todo_parse_line(const char *line, todo_t *t);

#endif
