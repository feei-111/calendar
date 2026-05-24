/*
 * context.c — Application Context Implementation
 *
 * Initialises AppContext, determines the platform-specific data directory
 * (%APPDATA%/cal/ on Windows, ~/.config/cal/ on Unix), and provides
 * cross-platform console setup and UTF-8 argument handling for Windows.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#include "context.h"
#include "config.h"
#include "ui.h"

/* === Path helpers ======================================================= */

static void ctx_data_dir(char *buf, size_t bufsz) {
#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (!appdata) {
        char local[512];
        if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, local) == S_OK)
            snprintf(buf, bufsz, "%s\\cal", local);
        else
            snprintf(buf, bufsz, "cal");
    } else {
        snprintf(buf, bufsz, "%s\\cal", appdata);
    }
#else
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    snprintf(buf, bufsz, "%s/.config/cal", home);
#endif
}

static void ctx_mkdirs(const char *path) {
#ifdef _WIN32
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir \"%s\" 2>nul", path);
    system(cmd);
#else
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\" 2>/dev/null", path);
    system(cmd);
#endif
}

/* === Lifecycle ========================================================== */

void ctx_init(AppContext *ctx) {
    memset(ctx, 0, sizeof(*ctx));

    ctx_data_dir(ctx->data_dir, sizeof(ctx->data_dir));
    ctx_mkdirs(ctx->data_dir);

    config_read(ctx);

    if (ctx->config.lang_idx >= 0)
        ctx->lang = &langs[ctx->config.lang_idx];
    else
        ctx->lang = &langs[detect_system_lang()];

    ui_load_festivals(ctx);
}

void ctx_destroy(AppContext *ctx) {
    (void)ctx;
}

/* === Platform abstraction =============================================== */

void ctx_setup_console(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

char **ctx_utf8_argv(int *out_argc) {
#ifdef _WIN32
    int wargc;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (!wargv) { *out_argc = 0; return NULL; }
    char **argv = (char **)malloc((size_t)wargc * sizeof(char *));
    if (!argv) { LocalFree(wargv); *out_argc = 0; return NULL; }
    for (int i = 0; i < wargc; i++) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
        argv[i] = (char *)malloc((size_t)len);
        if (argv[i])
            WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], len, NULL, NULL);
    }
    LocalFree(wargv);
    *out_argc = wargc;
    return argv;
#else
    (void)out_argc;
    return NULL;
#endif
}

void ctx_free_utf8_argv(int argc, char **argv) {
    if (!argv) return;
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}

void ctx_get_path(const AppContext *ctx, char *buf, size_t bufsz, const char *filename) {
    snprintf(buf, bufsz, "%s/%s", ctx->data_dir, filename);
}
