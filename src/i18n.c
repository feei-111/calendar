/*
 * i18n.c — Internationalisation Data & Language Detection
 *
 * Contains the immutable langs[] table with all 6 language definitions and
 * the system language auto-detection logic (Windows GetUserDefaultLocaleName,
 * POSIX setlocale() + $LANG).
 */

#include <string.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "i18n.h"

const lang_t langs[] = {
    {
        "zh", "中文",
        "周首日",
        {"周日", "周一", "周二", "周三", "周四", "周五", "周六"},
        {"日", "一", "二", "三", "四", "五", "六"},
        {"一月", "二月", "三月", "四月", "五月", "六月",
         "七月", "八月", "九月", "十月", "十一月", "十二月"},
        {"正月", "二月", "三月", "四月", "五月", "六月",
         "七月", "八月", "九月", "十月", "冬月", "腊月"},
        {"初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
         "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
         "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"},
        "闰",
        "【本月待办】", "无",
        "使用 'cal help | -h | --help' 显示帮助",
        "用法: cal [选项] [子命令]\n\n"
        "显示日历:\n"
        "  cal                            显示当前月\n"
        "  cal mon / sun                  当前月 (周一/周日为首)\n"
        "  cal YYYY MM [mon|sun]          显示指定年月\n"
        "  cal year [YYYY]                年视图 (显示全年12个月)\n"
        "  cal week [YYYY-MM-DD]          周视图 (显示单周详情)\n\n"
        "待办事项:\n"
        "  cal todo add [-p high|med|low] [-t 标签] [-c 日历] [-r weekly|monthly|yearly] [-d 截止日期] <日期> <内容>  添加代办\n"
        "    日期支持: YYYY-MM-DD, +Nd(天), +Nw(周), +Nm(月), today, tomorrow\n"
        "  cal todo del YYYY-MM-DD <序号>  删除待办\n"
        "  cal todo edit YYYY-MM-DD <序号> [选项] [新内容]  修改待办\n"
        "  cal todo list [YYYY-MM] [-c 日历] [-t 标签] [-p 优先级]  列出待办\n\n"
        "日历管理:\n"
        "  cal calendar list              列出所有日历及待办数\n"
        "  cal calendar rename <旧名> <新名>  重命名日历\n"
        "  cal calendar delete <名称>      删除日历及其所有待办\n\n"
        "工具:\n"
        "  cal datecalc DATE1 DATE2       计算两日期间隔天数\n"
        "  cal datecalc DATE +Nd          计算N天后的日期\n"
        "  cal zodiac [YYYY-MM-DD]        显示星座和生肖\n"
        "  cal undo                       撤销上一步操作\n"
        "  cal redo                       重做上一步操作\n\n"
        "配置:\n"
        "  cal config mon / sun           设置周一/周日为首日\n"
        "  cal config lang <语言>         设置语言 (zh/en/ja/fr/de/ru)\n"
        "  cal config lunar <auto|on|off> 设置农历显示\n"
        "  cal config compact <on|off>    紧凑模式\n"
        "  cal config tz <+/-N>           设置时区偏移 (如 +8, -5)\n"
        "  cal config calendar <名称>     设置默认日历\n\n"
        "帮助:\n"
        "  cal help | -h | --help         显示此帮助\n",
        "已添加待办", "已删除待办", "已修改待办", "未找到待办", "无待办事项",
        "错误: 无法打开", "错误: 无法写入",
        "已设置%s为首日", "已设置语言: %s",
        "无效月份", "无效参数。使用 'cal help' 显示帮助。",
        "请指定月份。用法: cal YYYY MM", "未知子命令",
        {"白羊座", "金牛座", "双子座", "巨蟹座", "狮子座", "处女座",
         "天秤座", "天蝎座", "射手座", "摩羯座", "水瓶座", "双鱼座"},
        {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"},
        {"无", "高", "中", "低"},
        "还剩%d天", "今天", "已逾期",
        "已撤销", "已重做", "无可撤销操作", "无可重做操作",
        "无效日期: %s", "周", "星座", "生肖",
        "农历: ", "紧凑模式: ", "时区: ",
        "本地", "默认日历: ", "天",
        "cal config <mon|sun|lang|lunar|compact|tz|calendar> ...",
        "cal todo <add|del|edit|list> ...",
        "cal datecalc DATE1 DATE2 | cal datecalc DATE +Nd",
        "警告: 未找到节日文件，请将其放入数据目录或程序根目录"
    },
    {
        "en", "English",
        "Week start",
        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
        {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"},
        {"January", "February", "March", "April", "May", "June",
         "July", "August", "September", "October", "November", "December"},
        {"Jan(L)", "Feb(L)", "Mar(L)", "Apr(L)", "May(L)", "Jun(L)",
         "Jul(L)", "Aug(L)", "Sep(L)", "Oct(L)", "Nov(L)", "Dec(L)"},
        {"1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th",
         "11th", "12th", "13th", "14th", "15th", "16th", "17th", "18th", "19th", "20th",
         "21st", "22nd", "23rd", "24th", "25th", "26th", "27th", "28th", "29th", "30th"},
        "Leap ",
        "[This Month's Todos]", "None",
        "Use 'cal help | -h | --help' for help",
        "Usage: cal [options] [subcommand]\n\n"
        "Display calendar:\n"
        "  cal                            Show current month\n"
        "  cal mon / sun                  Current month (Mon/Sun first)\n"
        "  cal YYYY MM [mon|sun]          Show specified year/month\n"
        "  cal year [YYYY]                Year view (all 12 months)\n"
        "  cal week [YYYY-MM-DD]          Week view (single week details)\n\n"
        "Todos:\n"
        "  cal todo add [-p high|med|low] [-t tags] [-c calendar] [-r weekly|monthly|yearly] [-d deadline] <DATE> <content>  Add todo\n"
        "    DATE supports: YYYY-MM-DD, +Nd(days), +Nw(weeks), +Nm(months), today, tomorrow\n"
        "  cal todo del YYYY-MM-DD <N>  Delete todo\n"
        "  cal todo edit YYYY-MM-DD <N> [options] [new content]  Edit todo\n"
        "  cal todo list [YYYY-MM] [-c calendar] [-t tag] [-p priority]  List todos\n\n"
        "Calendars:\n"
        "  cal calendar list              List all calendars with counts\n"
        "  cal calendar rename <old> <new>  Rename a calendar\n"
        "  cal calendar delete <name>     Delete a calendar and its todos\n\n"
        "Tools:\n"
        "  cal datecalc DATE1 DATE2       Days between two dates\n"
        "  cal datecalc DATE +Nd          Date N days later\n"
        "  cal zodiac [YYYY-MM-DD]        Show zodiac sign and Chinese zodiac\n"
        "  cal undo                       Undo last operation\n"
        "  cal redo                       Redo last undone operation\n\n"
        "Config:\n"
        "  cal config mon / sun           Set Mon/Sun as first day\n"
        "  cal config lang <language>     Set language (zh/en/ja/fr/de/ru)\n"
        "  cal config lunar <auto|on|off> Set lunar calendar display\n"
        "  cal config compact <on|off>    Compact mode\n"
        "  cal config tz <+/-N>           Set timezone offset (e.g. +8, -5)\n"
        "  cal config calendar <name>     Set default calendar\n\n"
        "Help:\n"
        "  cal help | -h | --help         Show this help\n",
        "Added todo", "Deleted todo", "Edited todo", "Todo not found", "No todos",
        "Error: cannot open", "Error: cannot write",
        "Set %s as first day", "Language set to: %s",
        "Invalid month", "Invalid argument. Use 'cal help' for help.",
        "Please specify month. Usage: cal YYYY MM", "Unknown subcommand",
        {"Aries", "Taurus", "Gemini", "Cancer", "Leo", "Virgo",
         "Libra", "Scorpio", "Sagittarius", "Capricorn", "Aquarius", "Pisces"},
        {"Rat", "Ox", "Tiger", "Rabbit", "Dragon", "Snake",
         "Horse", "Goat", "Monkey", "Rooster", "Dog", "Pig"},
        {"none", "high", "med", "low"},
        "%d days left", "TODAY", "OVERDUE",
        "Undo done", "Redo done", "Nothing to undo", "Nothing to redo",
        "Invalid date: %s", "Week", "Zodiac", "Chinese Zodiac",
        "Lunar: ", "Compact: ", "Timezone: ",
        "local", "Calendar: ", "days",
        "cal config <mon|sun|lang|lunar|compact|tz|calendar> ...",
        "cal todo <add|del|edit|list> ...",
        "cal datecalc DATE1 DATE2 | cal datecalc DATE +Nd",
        "Warning: festival file not found, place it in data directory or program root"
    },
    {
        "ja", "日本語",
        "週開始",
        {"日曜", "月曜", "火曜", "水曜", "木曜", "金曜", "土曜"},
        {"日", "月", "火", "水", "木", "金", "土"},
        {"1月", "2月", "3月", "4月", "5月", "6月",
         "7月", "8月", "9月", "10月", "11月", "12月"},
        {"睦月", "如月", "弥生", "卯月", "皐月", "水無月",
         "文月", "葉月", "長月", "神無月", "霜月", "師走"},
        {"一日", "二日", "三日", "四日", "五日", "六日", "七日", "八日", "九日", "十日",
         "十一日", "十二日", "十三日", "十四日", "十五日", "十六日", "十七日", "十八日", "十九日", "二十日",
         "廿一日", "廿二日", "廿三日", "廿四日", "廿五日", "廿六日", "廿七日", "廿八日", "廿九日", "三十日"},
        "閏",
        "【今月の予定】", "なし",
        "'cal help | -h | --help' でヘルプ表示",
        "使い方: cal [オプション] [サブコマンド]\n\n"
        "カレンダー表示:\n"
        "  cal                            今月を表示\n"
        "  cal mon / sun                  今月 (月曜/日曜開始)\n"
        "  cal YYYY MM [mon|sun]          指定年月を表示\n"
        "  cal year [YYYY]                年ビュー (全12ヶ月)\n"
        "  cal week [YYYY-MM-DD]          週ビュー (単週詳細)\n\n"
        "予定:\n"
        "  cal todo add [-p high|med|low] [-t タグ] [-c カレンダー] [-r weekly|monthly|yearly] [-d 期限] <日付> <内容>  予定を追加\n"
        "    日付: YYYY-MM-DD, +Nd(日), +Nw(週), +Nm(月), today, tomorrow\n"
        "  cal todo del YYYY-MM-DD <番号>  予定を削除\n"
        "  cal todo edit YYYY-MM-DD <番号> [オプション] [新内容]  予定を編集\n"
        "  cal todo list [YYYY-MM] [-c カレンダー] [-t タグ] [-p 優先度]  予定一覧\n\n"
        "カレンダー管理:\n"
        "  cal calendar list              全カレンダー一覧\n"
        "  cal calendar rename <旧名> <新名>  カレンダー名変更\n"
        "  cal calendar delete <名前>      カレンダーと予定を削除\n\n"
        "ツール:\n"
        "  cal datecalc DATE1 DATE2       日付間の日数計算\n"
        "  cal datecalc DATE +Nd          N日後の日付\n"
        "  cal zodiac [YYYY-MM-DD]        星座と干支を表示\n"
        "  cal undo                       元に戻す\n"
        "  cal redo                       やり直す\n\n"
        "設定:\n"
        "  cal config mon / sun           月曜/日曜を最初の日に設定\n"
        "  cal config lang <言語>         言語設定 (zh/en/ja/fr/de/ru)\n"
        "  cal config lunar <auto|on|off> 旧暦表示の設定\n"
        "  cal config compact <on|off>    コンパクトモード\n"
        "  cal config tz <+/-N>           タイムゾーン設定\n"
        "  cal config calendar <名前>     デフォルトカレンダー設定\n\n"
        "ヘルプ:\n"
        "  cal help | -h | --help         このヘルプを表示\n",
        "予定を追加", "予定を削除", "予定を編集", "予定が見つかりません", "予定なし",
        "エラー: 開けません", "エラー: 書き込めません",
        "%sを最初の日に設定", "言語を設定: %s",
        "無効な月", "無効な引数。'cal help' でヘルプ表示。",
        "月を指定してください。使い方: cal YYYY MM", "不明なサブコマンド",
        {"牡羊座", "牡牛座", "双子座", "蟹座", "獅子座", "乙女座",
         "天秤座", "蠍座", "射手座", "山羊座", "水瓶座", "魚座"},
        {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"},
        {"なし", "高", "中", "低"},
        "残り%d日", "今日", "期限超過",
        "元に戻しました", "やり直しました", "元に戻せません", "やり直せません",
        "無効な日付: %s", "週", "星座", "干支",
        "旧暦: ", "コンパクト: ", "タイムゾーン: ",
        "ローカル", "カレンダー: ", "日",
        "cal config <mon|sun|lang|lunar|compact|tz|calendar> ...",
        "cal todo <add|del|edit|list> ...",
        "cal datecalc DATE1 DATE2 | cal datecalc DATE +Nd",
        "警告: 祭日ファイルが見つかりません。データディレクトリかルートに配置してください"
    },
    {
        "fr", "Français",
        "Début semaine",
        {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
        {"Di", "Lu", "Ma", "Me", "Je", "Ve", "Sa"},
        {"Janvier", "Février", "Mars", "Avril", "Mai", "Juin",
         "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"},
        {"Jan(L)", "Fév(L)", "Mar(L)", "Avr(L)", "Mai(L)", "Jun(L)",
         "Jul(L)", "Aoû(L)", "Sep(L)", "Oct(L)", "Nov(L)", "Déc(L)"},
        {"1er", "2", "3", "4", "5", "6", "7", "8", "9", "10",
         "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
         "21", "22", "23", "24", "25", "26", "27", "28", "29", "30"},
        "Bis ",
        "[Tâches du mois]", "Aucune",
        "Utilisez 'cal help | -h | --help' pour l'aide",
        "Usage: cal [options] [sous-commande]\n\n"
        "Afficher le calendrier:\n"
        "  cal                            Mois actuel\n"
        "  cal mon / sun                  Mois actuel (lun/dim premier)\n"
        "  cal YYYY MM [mon|sun]          Année et mois spécifiés\n"
        "  cal year [YYYY]                Vue annuelle (12 mois)\n"
        "  cal week [YYYY-MM-DD]          Vue hebdomadaire\n\n"
        "Tâches:\n"
        "  cal todo add [-p high|med|low] [-t tags] [-c calendrier] [-r weekly|monthly|yearly] [-d échéance] <DATE> <contenu>  Ajouter tâche\n"
        "    DATE: YYYY-MM-DD, +Nd(jours), +Nw(sem), +Nm(mois), today, tomorrow\n"
        "  cal todo del YYYY-MM-DD <N>  Supprimer tâche\n"
        "  cal todo edit YYYY-MM-DD <N> [options] [nouveau]  Modifier tâche\n"
        "  cal todo list [YYYY-MM] [-c calendrier] [-t tag] [-p priorité]  Lister tâche\n\n"
        "Calendriers:\n"
        "  cal calendar list              Lister tous les calendriers\n"
        "  cal calendar rename <anc> <nouv>  Renommer un calendrier\n"
        "  cal calendar delete <nom>      Supprimer un calendrier\n\n"
        "Outils:\n"
        "  cal datecalc DATE1 DATE2       Jours entre deux dates\n"
        "  cal datecalc DATE +Nd          Date dans N jours\n"
        "  cal zodiac [YYYY-MM-DD]        Signe zodiacal et zodiaque chinois\n"
        "  cal undo                       Annuler dernière opération\n"
        "  cal redo                       Rétablir\n\n"
        "Configuration:\n"
        "  cal config mon / sun           Lun/Dim comme premier jour\n"
        "  cal config lang <langue>       Langue (zh/en/ja/fr/de/ru)\n"
        "  cal config lunar <auto|on|off> Calendrier lunaire\n"
        "  cal config compact <on|off>    Mode compact\n"
        "  cal config tz <+/-N>           Fuseau horaire\n"
        "  cal config calendar <nom>      Calendrier par défaut\n\n"
        "Aide:\n"
        "  cal help | -h | --help         Afficher cette aide\n",
        "Tâche ajoutée", "Tâche supprimée", "Tâche modifiée", "Tâche introuvable", "Aucune tâche",
        "Erreur: ouverture impossible", "Erreur: écriture impossible",
        "%s défini comme premier jour", "Langue définie: %s",
        "Mois invalide", "Argument invalide. Utilisez 'cal help'.",
        "Spécifiez le mois. Usage: cal YYYY MM", "Sous-commande inconnue",
        {"Bélier", "Taureau", "Gémeaux", "Cancer", "Lion", "Vierge",
         "Balance", "Scorpion", "Sagittaire", "Capricorne", "Verseau", "Poissons"},
        {"Rat", "Bœuf", "Tigre", "Lapin", "Dragon", "Serpent",
         "Cheval", "Chèvre", "Singe", "Coq", "Chien", "Cochon"},
        {"aucune", "haute", "moy", "basse"},
        "%d jours restants", "AUJOURD'HUI", "EN RETARD",
        "Annulation faite", "Rétablissement fait", "Rien à annuler", "Rien à rétablir",
        "Date invalide: %s", "Semaine", "Zodiaque", "Zodiaque chinois",
        "Lunaire: ", "Compact: ", "Fuseau: ",
        "local", "Calendrier: ", "jours",
        "cal config <mon|sun|lang|lunar|compact|tz|calendar> ...",
        "cal todo <add|del|edit|list> ...",
        "cal datecalc DATE1 DATE2 | cal datecalc DATE +Nd",
        "Avertissement: fichier de fête introuvable, placez-le dans le répertoire de données ou la racine"
    },
    {
        "de", "Deutsch",
        "Wochenstart",
        {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"},
        {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"},
        {"Januar", "Februar", "März", "April", "Mai", "Juni",
         "Juli", "August", "September", "Oktober", "November", "Dezember"},
        {"Jan(M)", "Feb(M)", "Mär(M)", "Apr(M)", "Mai(M)", "Jun(M)",
         "Jul(M)", "Aug(M)", "Sep(M)", "Okt(M)", "Nov(M)", "Dez(M)"},
        {"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.",
         "11.", "12.", "13.", "14.", "15.", "16.", "17.", "18.", "19.", "20.",
         "21.", "22.", "23.", "24.", "25.", "26.", "27.", "28.", "29.", "30."},
        "Schalt ",
        "[Monatsaufgaben]", "Keine",
        "'cal help | -h | --help' für Hilfe",
        "Verwendung: cal [Optionen] [Unterbefehl]\n\n"
        "Kalender anzeigen:\n"
        "  cal                            Aktueller Monat\n"
        "  cal mon / sun                  Aktueller Monat (Mo/So zuerst)\n"
        "  cal YYYY MM [mon|sun]          Angegebenes Jahr/Monat\n"
        "  cal year [YYYY]                Jahresansicht (alle 12 Monate)\n"
        "  cal week [YYYY-MM-DD]          Wochenansicht\n\n"
        "Aufgaben:\n"
        "  cal todo add [-p high|med|low] [-t Tags] [-c Kalender] [-r weekly|monthly|yearly] [-d Frist] <Datum> <Inhalt>  Aufgabe hinzufügen\n"
        "    Datum: YYYY-MM-DD, +Nd(Tage), +Nw(Wochen), +Nm(Monate), today, tomorrow\n"
        "  cal todo del YYYY-MM-DD <N>  Aufgabe löschen\n"
        "  cal todo edit YYYY-MM-DD <N> [Optionen] [neu]  Aufgabe bearbeiten\n"
        "  cal todo list [YYYY-MM] [-c Kalender] [-t Tag] [-p Priorität]  Liste der Aufgaben\n\n"
        "Kalender:\n"
        "  cal calendar list              Alle Kalender auflisten\n"
        "  cal calendar rename <alt> <neu>  Kalender umbenennen\n"
        "  cal calendar delete <Name>     Kalender löschen\n\n"
        "Werkzeuge:\n"
        "  cal datecalc DATE1 DATE2       Tage zwischen zwei Daten\n"
        "  cal datecalc DATE +Nd          Datum in N Tagen\n"
        "  cal zodiac [YYYY-MM-DD]        Sternzeichen und chinesisches Tierzeichen\n"
        "  cal undo                       Letzte Operation rückgängig\n"
        "  cal redo                       Wiederherstellen\n\n"
        "Konfiguration:\n"
        "  cal config mon / sun           Mo/So als erster Tag\n"
        "  cal config lang <Sprache>      Sprache (zh/en/ja/fr/de/ru)\n"
        "  cal config lunar <auto|on|off> Mondkalender-Anzeige\n"
        "  cal config compact <on|off>    Kompaktmodus\n"
        "  cal config tz <+/-N>           Zeitzone\n"
        "  cal config calendar <Name>     Standardkalender\n\n"
        "Hilfe:\n"
        "  cal help | -h | --help         Diese Hilfe anzeigen\n",
        "Aufgabe hinzugefügt", "Aufgabe gelöscht", "Aufgabe bearbeitet",
        "Aufgabe nicht gefunden", "Keine Aufgaben",
        "Fehler: Öffnen nicht möglich", "Fehler: Schreiben nicht möglich",
        "%s als erster Tag gesetzt", "Sprache eingestellt: %s",
        "Ungültiger Monat", "Ungültiges Argument. 'cal help' für Hilfe.",
        "Monat angeben. Verwendung: cal YYYY MM", "Unbekannter Unterbefehl",
        {"Widder", "Stier", "Zwillinge", "Krebs", "Löwe", "Jungfrau",
         "Waage", "Skorpion", "Schütze", "Steinbock", "Wassermann", "Fische"},
        {"Ratte", "Ochse", "Tiger", "Hase", "Drache", "Schlange",
         "Pferd", "Ziege", "Affe", "Hahn", "Hund", "Schwein"},
        {"keine", "hoch", "mit", "niedrig"},
        "%d Tage übrig", "HEUTE", "ÜBERFÄLLIG",
        "Rückgängig gemacht", "Wiederhergestellt", "Nichts rückgängig", "Nichts wiederherzustellen",
        "Ungültiges Datum: %s", "Woche", "Sternzeichen", "Chinesisches Sternzeichen",
        "Mond: ", "Kompakt: ", "Zeitzone: ",
        "lokal", "Kalender: ", "Tage",
        "cal config <mon|sun|lang|lunar|compact|tz|calendar> ...",
        "cal todo <add|del|edit|list> ...",
        "cal datecalc DATE1 DATE2 | cal datecalc DATE +Nd",
        "Warnung: Festdatei nicht gefunden, ins Datenverzeichnis oder Wurzelverzeichnis legen"
    },
    {
        "ru", "Русский",
        "Начало недели",
        {"Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"},
        {"Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"},
        {"Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
         "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"},
        {"Янв(Л)", "Фев(Л)", "Мар(Л)", "Апр(Л)", "Май(Л)", "Июн(Л)",
         "Июл(Л)", "Авг(Л)", "Сен(Л)", "Окт(Л)", "Ноя(Л)", "Дек(Л)"},
        {"1-е", "2-е", "3-е", "4-е", "5-е", "6-е", "7-е", "8-е", "9-е", "10-е",
         "11-е", "12-е", "13-е", "14-е", "15-е", "16-е", "17-е", "18-е", "19-е", "20-е",
         "21-е", "22-е", "23-е", "24-е", "25-е", "26-е", "27-е", "28-е", "29-е", "30-е"},
        "Вис ",
        "[Задачи месяца]", "Нет",
        "'cal help | -h | --help' для справки",
        "Использование: cal [опции] [подкоманда]\n\n"
        "Показать календарь:\n"
        "  cal                            Текущий месяц\n"
        "  cal mon / sun                  Текущий месяц (Пн/Вс первый)\n"
        "  cal YYYY MM [mon|sun]          Указанный год и месяц\n"
        "  cal year [YYYY]                Годовой вид (все 12 месяцев)\n"
        "  cal week [YYYY-MM-DD]          Недельный вид\n\n"
        "Задачи:\n"
        "  cal todo add [-p high|med|low] [-t теги] [-c календарь] [-r weekly|monthly|yearly] [-d срок] <дата> <содержание>  Добавить задачу\n"
        "    дата: YYYY-MM-DD, +Nd(дни), +Nw(нед), +Nm(мес), today, tomorrow\n"
        "  cal todo del YYYY-MM-DD <N>  Удалить задачу\n"
        "  cal todo edit YYYY-MM-DD <N> [опции] [новое]  Изменить задачу\n"
        "  cal todo list [YYYY-MM] [-c календарь] [-t тег] [-p приоритет]  Список задачу\n\n"
        "Календари:\n"
        "  cal calendar list              Список всех календарей\n"
        "  cal calendar rename <стар> <нов>  Переименовать календарь\n"
        "  cal calendar delete <имя>      Удалить календарь и задачи\n\n"
        "Инструменты:\n"
        "  cal datecalc DATE1 DATE2       Дней между датами\n"
        "  cal datecalc DATE +Nd          Дата через N дней\n"
        "  cal zodiac [YYYY-MM-DD]        Знак зодиака и китайский зодиак\n"
        "  cal undo                       Отменить последнюю операцию\n"
        "  cal redo                       Повторить\n\n"
        "Настройки:\n"
        "  cal config mon / sun           Пн/Вс первым днём\n"
        "  cal config lang <язык>         Язык (zh/en/ja/fr/de/ru)\n"
        "  cal config lunar <auto|on|off> Лунный календарь\n"
        "  cal config compact <on|off>    Компактный режим\n"
        "  cal config tz <+/-N>           Часовой пояс\n"
        "  cal config calendar <имя>      Календарь по умолчанию\n\n"
        "Справка:\n"
        "  cal help | -h | --help         Показать справку\n",
        "Задача добавлена", "Задача удалена", "Задача изменена",
        "Задача не найдена", "Нет задач",
        "Ошибка: не удалось открыть", "Ошибка: не удалось записать",
        "%s установлен как первый день", "Язык установлен: %s",
        "Неверный месяц", "Неверный аргумент. 'cal help' для справки.",
        "Укажите месяц. Использование: cal YYYY MM", "Неизвестная подкоманда",
        {"Овен", "Телец", "Близнецы", "Рак", "Лев", "Дева",
         "Весы", "Скорпион", "Стрелец", "Козерог", "Водолей", "Рыбы"},
        {"Крыса", "Бык", "Тигр", "Кролик", "Дракон", "Змея",
         "Лошадь", "Коза", "Обезьяна", "Петух", "Собака", "Свинья"},
        {"нет", "высокий", "сред", "низкий"},
        "осталось %d дн.", "СЕГОДНЯ", "ПРОСРОЧЕНО",
        "Отменено", "Повторено", "Нечего отменять", "Нечего повторять",
        "Неверная дата: %s", "Неделя", "Зодиак", "Китайский зодиак",
        "Лунный: ", "Компактный: ", "Часовой пояс: ",
        "местный", "Календарь: ", "дн.",
        "cal config <mon|sun|lang|lunar|compact|tz|calendar> ...",
        "cal todo <add|del|edit|list> ...",
        "cal datecalc DATE1 DATE2 | cal datecalc DATE +Nd",
        "Предупреждение: файл праздников не найден, поместите в каталог данных или корень программы"
    }
};

const int NUM_LANGS = (int)(sizeof(langs) / sizeof(langs[0]));

int lang_index_by_code(const char *code) {
    for (int i = 0; i < NUM_LANGS; i++)
        if (strcmp(langs[i].code, code) == 0)
            return i;
    return -1;
}

int detect_system_lang(void) {
#ifdef _WIN32
    wchar_t wlocale[85];
    int len = GetUserDefaultLocaleName(wlocale, 85);
    if (len > 0) {
        char loc[85];
        WideCharToMultiByte(CP_UTF8, 0, wlocale, len, loc, 85, NULL, NULL);
        loc[2] = '\0';
        if (strcmp(loc, "zh") == 0) return 0;
        if (strcmp(loc, "ja") == 0) return 2;
        if (strcmp(loc, "fr") == 0) return 3;
        if (strcmp(loc, "de") == 0) return 4;
        if (strcmp(loc, "ru") == 0) return 5;
    }
#else
    const char *loc = setlocale(LC_ALL, "");
    if (loc) {
        if (strncmp(loc, "zh", 2) == 0) return 0;
        if (strncmp(loc, "ja", 2) == 0) return 2;
        if (strncmp(loc, "fr", 2) == 0) return 3;
        if (strncmp(loc, "de", 2) == 0) return 4;
        if (strncmp(loc, "ru", 2) == 0) return 5;
    }
    const char *env = getenv("LANG");
    if (env) {
        if (strncmp(env, "zh", 2) == 0) return 0;
        if (strncmp(env, "ja", 2) == 0) return 2;
        if (strncmp(env, "fr", 2) == 0) return 3;
        if (strncmp(env, "de", 2) == 0) return 4;
        if (strncmp(env, "ru", 2) == 0) return 5;
    }
#endif
    return 1;
}
