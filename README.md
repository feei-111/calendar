# cal — A Multilingual CLI Calendar & Todo Manager

A feature-rich command-line calendar application written in C99 with built-in
Chinese lunar calendar support, todo management, and 6-language
internationalisation.

> 📖 [中文 README](README_zh.md)

<img width="600" height="425" alt="{43116078-A855-4E4F-94C7-D0EE65E1287D}" src="https://github.com/user-attachments/assets/b0f5fd09-6e9f-4ed7-b101-4bff4f4b40b5" />

---

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Command Reference](#command-reference)
  - [Calendar Views](#calendar-views)
  - [Todo Management](#todo-management)
  - [Calendar Management](#calendar-management)
  - [Tools](#tools)
  - [Configuration](#configuration)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [Data Directory](#data-directory)
- [Todo File Format](#todo-file-format)
- [Language Support](#language-support)
- [Build & Development](#build--development)
- [License](#license)

---

## Features

- **Calendar Views**: month, year (3x4 grid), week (detailed with todos)
- **Lunar Calendar**: embedded 1900–2100 lookup table, accurate solar↔lunar conversion
- **6 Languages**: Chinese, English, Japanese, French, German, Russian
- **Todo System**: add, delete, edit, list with priorities, tags, multi-calendar, deadlines
- **Recurring Events**: weekly, monthly, yearly
- **Undo/Redo**: full operation history via file snapshots
- **Date Calculator**: `cal datecalc 2026-01-01 2026-12-31`
- **Zodiac**: Western zodiac sign + Chinese zodiac animal
- **Compact Mode**: `cal config compact on` — no lunar/festival line
- **Timezones**: `cal config tz +8`
- **ANS Colors**: red=today, green=holiday, blue=weekend, yellow=todo marker

## Quick Start

### Build
```bash
gcc -std=c99 -Wall -Wextra -O2 -Iinclude -o cal.exe src/*.c
```
Or with Make:
```bash
make clean && make
```

### Basic Usage
```bash
cal                   # current month
cal 2026 12           # December 2026
cal year 2026         # full year view
cal week 2026-05-20   # week view with todos
cal help              # full help
```

### Todo Management
```bash
# Add with priority, tags, calendar, deadline
cal todo add -p high -t work,urgent -c work -d 2026-06-01 2026-05-25 submit experimental report

# Quick add (relative dates)
cal todo add +3d read books
cal todo add tomorrow go shopping

# List with filters
cal todo list -c work
cal todo list -t urgent -p high
```

### Calendar Management
```bash
cal calendar list              # list all calendars
cal calendar rename old new    # rename a calendar
cal calendar delete name       # delete calendar & its todos
```

### Configuration
```bash
cal config mon                 # Monday as first day of week
cal config lang en             # switch to English
cal config lunar off           # hide lunar dates
cal config compact on          # compact display mode
cal config tz +8               # set timezone to UTC+8
cal config calendar work       # set default calendar
```

## Project Structure

```
claendar/
├── include/                   # Header files (6)
│   ├── context.h              # AppContext struct, types, macros
│   ├── i18n.h                 # Language system definitions
│   ├── calendar.h             # Lunar engine API
│   ├── todo.h                 # Todo CRUD API
│   ├── config.h               # Configuration persistence
│   └── ui.h                   # Rendering engine API
├── src/                       # Implementation files (7)
│   ├── main.c                 # CLI entry point & dispatch
│   ├── context.c              # Context lifecycle, platform paths
│   ├── i18n.c                 # 6-language data table
│   ├── calendar.c             # Lunar calendar engine
│   ├── todo.c                 # Todo CRUD & persistence
│   ├── config.c               # Config read/write, date parsing
│   └── ui.c                   # All terminal rendering
├── lang/                      # Festival & holiday translations (18 files)
│   ├── festival_solar_*.txt   # Solar festival names per language
│   ├── festival_lunar_*.txt   # Lunar festival names per language
│   └── holiday_*.txt          # User holiday definitions per language
├── Makefile
└── README.md
```

## Architecture

- **Zero global variables** — all state lives in `AppContext`, allocated on heap
- **No static buffer returns** — callers provide buffers to `ui_get_day_info()`
- **DRY config** — a single `config_save()` handles all config writes
- **Cross-platform paths** — `ctx_get_path()` uses `%APPDATA%/cal/` on Windows, `~/.config/cal/` on Unix
- **Memory safe** — `snprintf`/`strncpy` with manual NUL termination throughout

## Data Directory

| Platform    | Default Path                          |
|-------------|---------------------------------------|
| Windows     | `%APPDATA%\cal\` (e.g. `C:\Users\You\AppData\Roaming\cal\`) |
| Linux/macOS | `~/.config/cal/`                      |

Files stored: `todo.txt`, `.cal_config`, `.cal_undo`, `.cal_redo`, festival/holiday files.

## Language Support

Language is auto-detected from the system on first run and stored in `.cal_config`.

```bash
cal config lang zh    # 中文
cal config lang en    # English
cal config lang ja    # 日本語
cal config lang fr    # Français
cal config lang de    # Deutsch
cal config lang ru    # Русский
```

For languages beyond these six, English is used as the fallback.

## Todo File Format

```
YYYY-MM-DD|N|P|tags|calendar|recur|deadline|content
```

| Field    | Description                                       |
|----------|---------------------------------------------------|
| YYYY-MM-DD | Date                                             |
| N        | Sequence number (auto-incremented per date)        |
| P        | Priority: 0=none, 1=high, 2=medium, 3=low          |
| tags     | Comma-separated tags                               |
| calendar | Calendar name (default: "default")                 |
| recur    | `none`, `weekly`, `monthly`, or `yearly`           |
| deadline | `YYYY-MM-DD` or empty                              |
| content  | Todo text (may contain pipes — parsed from field 7)|

Legacy format (`YYYY-MM-DD|N|content`) is also supported.

## License

MIT
