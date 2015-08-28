#ifndef _SALDL_VT100_H
#define _SALDL_VT100_H
#else
#error redefining _SALDL_VT100_H
#endif

/* vt100 color and formatting defs  */

/* FG */
#define BLACK_FG "\033[30m"
#define RED_FG "\033[31m"
#define GREEN_FG "\033[32m"
#define YELLOW_FG "\033[33m"
#define BLUE_FG "\033[34m"
#define PURPLE_FG "\033[35m" /* magenta */
#define SKY_FG "\033[36m" /* cyan */
#define WHITE_FG "\033[37m"
#define DEFAULT_FG "\033[39m"

/* BG */
#define BLACK_BG "\033[40m"
#define RED_BG "\033[41m"
#define GREEN_BG "\033[42m"
#define YELLOW_BG "\033[43m"
#define BLUE_BG "\033[44m"
#define PURPLE_BG "\033[45m" /* magenta */
#define SKY_BG "\033[46m" /* cyan */
#define WHITE_BG "\033[47m"
#define DEFAULT_BG "\033[49m"

/* Formatting  */
#define END "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"

/* Color Descriptions */
#define OK_COLOR GREEN_FG
#define DEBUG_EVENT_COLOR SKY_BG WHITE_FG
#define DEBUG_COLOR BLUE_BG WHITE_FG
#define INFO_COLOR SKY_FG
#define WARN_COLOR YELLOW_FG
#define ERROR_COLOR RED_FG
#define FATAL_COLOR RED_BG WHITE_FG
#define FINISH_COLOR GREEN_BG WHITE_FG

/* predefined msg prefixes */
#define DEBUG_EVENT_MSG_TXT "[debug-event]"
#define DEBUG_MSG_TXT "[debug]"
#define INFO_MSG_TXT "[info]"
#define WARN_MSG_TXT "[warning]"
#define ERROR_MSG_TXT "[error]"
#define FATAL_MSG_TXT "[fatal]"
#define FINISH_MSG_TXT "Download Finished"

/* Other vt100 escape sequences */
#define ERASE_AFTER "\033[0K"
#define ERASE_SCREEN_BEFORE "\033[1J"
#define ERASE_SCREEN_AFTER "\033[0J"
#define ERASE_BEFORE "\033[1K"
#define SAVE "\0337"
#define RESTORE "\0338"
#define UP "\033[1A"
#define DOWN "\033[1B"
#define RIGHT "\033[1C"
#define LEFT "\033[1D"

/* other */
#define RET_CHAR "\r"

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
