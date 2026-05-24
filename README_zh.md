# cal — 多语言命令行日历与待办管理工具

一个用 C99 编写的功能丰富的命令行日历程序，内置中国农历支持、待办管理和 6 语言国际化。

> 📖 [English README](README.md)

<img width="1195" height="849" alt="图片" src="https://github.com/user-attachments/assets/a71003d0-ce58-4946-aed3-042a823e3ce4" />

---

## 目录

- [功能特性](#功能特性)
- [快速开始](#快速开始)
- [命令速查](#命令速查)
  - [日历视图](#日历视图)
  - [待办管理](#待办管理)
  - [日历管理](#日历管理)
  - [工具](#工具)
  - [配置](#配置)
- [项目结构](#项目结构)
- [架构设计](#架构设计)
- [数据目录](#数据目录)
- [待办文件格式](#待办文件格式)
- [语言支持](#语言支持)
- [编译与开发](#编译与开发)
- [许可证](#许可证)

---

## 功能特性

- **三种日历视图**：月视图（传统网格）、年视图（3×4 布局）、周视图（含待办详情）
- **农历支持**：内嵌 1900–2100 年查表引擎，精确的农历公历互转
- **六种语言**：中文、English、日本語、Français、Deutsch、Русский
- **待办系统**：添加/删除/编辑/列出，支持优先级、标签、多日历、截止日
- **重复事件**：支持每周、每月、每年重复
- **撤销/重做**：通过文件快照实现完整操作历史
- **日期计算器**：`cal datecalc 2026-01-01 2026-12-31`
- **星座生肖**：西方星座 + 中国生肖
- **紧凑模式**：`cal config compact on` — 隐藏农历/节日行
- **时区支持**：`cal config tz +8`
- **节日数据外置**：节日名称从文件加载，切换语言即切换翻译
- **ANSI 彩色输出**：红色=今天，绿色=节日/假日，蓝色=周末，黄色=待办标记

---

## 快速开始

### 编译
```bash
gcc -std=c99 -Wall -Wextra -O2 -Iinclude -o cal.exe src/*.c
```
或使用 Makefile：
```bash
make clean && make
```

### 基础用法
```bash
cal                   # 显示当月
cal 2026 12           # 显示 2026 年 12 月
cal year 2026         # 年视图
cal week 2026-05-20   # 周视图（含待办详情）
cal help              # 查看完整帮助
```

---

## 命令速查

### 日历视图
| 命令 | 功能 |
|------|------|
| `cal` | 显示当前月（遵循配置的首日设置） |
| `cal mon` | 当前月，周一为首日 |
| `cal sun` | 当前月，周日为首日 |
| `cal YYYY MM` | 显示指定年月 |
| `cal YYYY MM mon\|sun` | 指定年月及首日 |
| `cal year [YYYY]` | 年视图，显示全年 12 个月 |
| `cal week [YYYY-MM-DD]` | 周视图，显示单周详情 |

### 待办管理
```bash
# 添加（支持优先级、标签、日历、重复、截止日）
cal todo add -p high -t work,urgent -c work -r weekly -d 2026-06-01 2026-05-25 团队周会

# 快速添加（相对日期）
cal todo add +3d 读书           # 3天后
cal todo add +1w 周会           # 1周后
cal todo add +1m 月报           # 1月后
cal todo add today 买菜          # 今天
cal todo add tomorrow 看电影      # 明天

# 删除 / 修改
cal todo del YYYY-MM-DD <序号>
cal todo edit YYYY-MM-DD <序号> -p high 新内容

# 列出（支持筛选）
cal todo list                    # 全部
cal todo list 2026-05            # 按月份筛选
cal todo list -c work            # 按日历筛选
cal todo list -t urgent          # 按标签筛选
cal todo list -p high            # 按优先级筛选
```

### 日历管理
| 命令 | 功能 |
|------|------|
| `cal calendar list` | 列出所有日历及待办数量 |
| `cal calendar rename <旧名> <新名>` | 重命名日历 |
| `cal calendar delete <名称>` | 删除日历及其所有待办 |

### 工具
| 命令 | 功能 |
|------|------|
| `cal datecalc DATE1 DATE2` | 计算两日期之间的天数 |
| `cal datecalc DATE +Nd` | 计算 N 天后的日期 |
| `cal zodiac [YYYY-MM-DD]` | 显示星座和生肖 |
| `cal undo` | 撤销上一步操作 |
| `cal redo` | 重做上一步操作 |

### 配置
| 命令 | 功能 |
|------|------|
| `cal config mon` | 周一为首日 |
| `cal config sun` | 周日为首日 |
| `cal config lang <代码>` | 设置语言 (zh/en/ja/fr/de/ru) |
| `cal config lunar auto\|on\|off` | 农历显示模式 |
| `cal config compact on\|off` | 紧凑模式 |
| `cal config tz <+/-N>` | 时区偏移（如 +8 表示 UTC+8） |
| `cal config calendar <名称>` | 设置默认日历 |

---

## 项目结构

```
claendar/
├── include/                   # 头文件 (6 个)
│   ├── context.h              # AppContext 结构体、类型定义、平台宏
│   ├── i18n.h                 # 语言系统定义
│   ├── calendar.h             # 农历引擎接口
│   ├── todo.h                 # 待办 CRUD 接口
│   ├── config.h               # 配置持久化接口
│   └── ui.h                   # 渲染引擎接口
├── src/                       # 实现文件 (7 个)
│   ├── main.c                 # 入口点与 CLI 命令派发
│   ├── context.c              # 上下文生命周期、平台路径
│   ├── i18n.c                 # 6 语言数据表
│   ├── calendar.c             # 农历日历引擎
│   ├── todo.c                 # 待办 CRUD 与持久化
│   ├── config.c               # 配置读写、日期参数解析
│   └── ui.c                   # 终端渲染
├── lang/                      # 节日翻译文件 (18 个)
│   ├── festival_solar_*.txt   # 公历节日名（按语言）
│   ├── festival_lunar_*.txt   # 农历节日名（按语言）
│   └── holiday_*.txt          # 用户节假日定义（按语言）
├── Makefile
├── README.md                  # English README
└── README_zh.md               # 中文 README（本文件）
```

---

## 架构设计

| 原则 | 实现方式 |
|------|----------|
| **零全局变量** | 所有状态封装在 `AppContext` 中，堆上分配，指针传递 |
| **无静态缓冲区返回** | `ui_get_day_info()` 由调用方提供缓冲区 |
| **DRY 配置保存** | 单一的 `config_save()` 处理所有配置写入 |
| **跨平台路径** | `ctx_get_path()` 统一封装：Win→`%APPDATA%/cal/`，Unix→`~/.config/cal/` |
| **内存安全** | 全面使用 `snprintf`/`strncpy` + 手动 NUL 终止 |
| **UTF-8 参数安全** | Windows 下 `ctx_utf8_argv()` 转换 + `ctx_free_utf8_argv()` 释放 |

---

## 数据目录

| 平台 | 默认路径 |
|------|----------|
| Windows | `%APPDATA%\cal\`（例如 `C:\Users\你的用户名\AppData\Roaming\cal\`） |
| Linux/macOS | `~/.config/cal/` |

存储文件：`todo.txt`、`.cal_config`、`.cal_undo`、`.cal_redo`、节日文件。

---

## 待办文件格式

```
YYYY-MM-DD|序号|优先级|标签|日历|重复|截止日|内容
```

| 字段 | 说明 |
|------|------|
| YYYY-MM-DD | 日期 |
| 序号 | 自动递增（每天从 1 计） |
| 优先级 | 0=无, 1=高, 2=中, 3=低 |
| 标签 | 逗号分隔的标签 |
| 日历 | 日历名称（默认 "default"） |
| 重复 | `none`, `weekly`, `monthly`, `yearly` |
| 截止日 | `YYYY-MM-DD` 或留空 |
| 内容 | 待办文本（可含 `\|`，从第 7 个管道符后全部截取） |

兼容旧格式 `YYYY-MM-DD|序号|内容`。

---

## 语言支持

首次运行时自动检测系统语言，并存入 `.cal_config`。

```bash
cal config lang zh    # 中文
cal config lang en    # English
cal config lang ja    # 日本語
cal config lang fr    # Français
cal config lang de    # Deutsch
cal config lang ru    # Русский
```

非这六种语言的系统会自动回退到英语。

---

## 编译与开发

### 依赖
- GCC（支持 C99 标准）
- 可选：GNU Make

### 编译命令
```bash
# 直接编译
gcc -std=c99 -Wall -Wextra -O2 -Iinclude -o cal src/*.c

# 使用 Makefile
make clean && make

# Windows MSYS2/MinGW
mingw32-make clean && mingw32-make
```

### 添加新语言
1. 在 `src/i18n.c` 的 `langs[]` 数组中添加新的语言条目
2. 在 `lang/` 目录创建对应的 `festival_solar_XX.txt`、`festival_lunar_XX.txt`、`holiday_XX.txt`

### VSCode 配置
项目已包含 `.vscode/c_cpp_properties.json`，`includePath` 指向 `include/` 目录。重新加载窗口即可消除 IntelliSense 报错。

---

## 许可证

MIT
