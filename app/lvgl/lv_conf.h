#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Use 32-bit color depth */
#define LV_COLOR_DEPTH     32

#define LV_DPI_DEF         320

/* Use stdlib to allocate/free memory */
#define LV_MEM_CUSTOM      1
#define LV_MEM_CUSTOM_INCLUDE   <string.h>   /*Header for the dynamic memory function*/
#define LV_MEM_CUSTOM_ALLOC     malloc
#define LV_MEM_CUSTOM_FREE      free
#define LV_MEM_CUSTOM_REALLOC   realloc

/* Use the standard `memcpy` and `memset` instead of LVGL's own functions. */
#define LV_MEMCPY_MEMSET_STD    1

#define LV_DISP_DEF_REFR_PERIOD 17
#define LV_INDEV_DEF_READ_PERIOD 5

/* Let LVGL call SDL_GetTicks automatically so we can skip creating a separate timer thread. */
#define LV_TICK_CUSTOM     1
#define LV_TICK_CUSTOM_INCLUDE  <SDL.h>
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (SDL_GetTicks())

#define LV_DRAW_COMPLEX 1
#define LV_SHADOW_CACHE_SIZE    0
#define LV_IMG_CACHE_DEF_SIZE       0

#define LV_GRADIENT_MAX_STOPS 8

/* Use SDL draw backend */
#define LV_USE_GPU_SDL    1
#define LV_GPU_SDL_INCLUDE_PATH <SDL.h>
#define LV_GPU_SDL_CUSTOM_BLEND_MODE 0

/*Change the built in (v)snprintf functions*/
#define LV_SPRINTF_CUSTOM   1
#if LV_SPRINTF_CUSTOM
#  define LV_SPRINTF_INCLUDE <stdio.h>
#  define lv_snprintf     snprintf
#  define lv_vsnprintf    vsnprintf
#else   /*LV_SPRINTF_CUSTOM*/
#  define LV_SPRINTF_USE_FLOAT 0
#endif  /*LV_SPRINTF_CUSTOM*/

/*Enable the log module*/
#define LV_USE_LOG      1
#if LV_USE_LOG

/*How important log should be added:
 *LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 *LV_LOG_LEVEL_INFO        Log important events
 *LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 *LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 *LV_LOG_LEVEL_USER        Only logs added by the user
 *LV_LOG_LEVEL_NONE        Do not log anything*/
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_WARN

/*1: Print the log with 'printf';
 *0: User need to register a callback with `lv_log_register_print_cb()`*/
#  define LV_LOG_PRINTF   0

/*Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs*/
#  define LV_LOG_TRACE_MEM            1
#  define LV_LOG_TRACE_TIMER          1
#  define LV_LOG_TRACE_INDEV          1
#  define LV_LOG_TRACE_DISP_REFR      1
#  define LV_LOG_TRACE_EVENT          1
#  define LV_LOG_TRACE_OBJ_CREATE     1
#  define LV_LOG_TRACE_LAYOUT         1
#  define LV_LOG_TRACE_ANIM           1

#endif  /*LV_USE_LOG*/

/* Abort on assertion failure */
#define LV_ASSERT_HANDLER_INCLUDE   <stdlib.h>
#define LV_ASSERT_HANDLER   abort();

#define LV_USE_USER_DATA      1

/* Use 32bit coordinates */
#define LV_USE_LARGE_COORD  1

#define LV_USE_FREETYPE 1

#define LV_FONT_UNSCII_16    1

#define LV_FONT_DEFAULT &lv_font_unscii_16

#define LV_TXT_ENC LV_TXT_ENC_UTF8


/*==================
 *  WIDGET USAGE
 *================*/

/*Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html*/

#define LV_USE_ARC          1

#define LV_USE_ANIMIMG        0

#define LV_USE_BAR          1

#define LV_USE_BTN          1

#define LV_USE_BTNMATRIX    1

#define LV_USE_CANVAS       1

#define LV_USE_CHECKBOX     1


#define LV_USE_DROPDOWN     1   /*Requires: lv_label*/

#define LV_USE_IMG          1   /*Requires: lv_label*/

#define LV_USE_LABEL        1
#if LV_USE_LABEL
#  define LV_LABEL_TEXT_SELECTION         1   /*Enable selecting text of the label*/
#  define LV_LABEL_LONG_TXT_HINT    1   /*Store some extra info in labels to speed up drawing of very long texts*/
#endif

#define LV_USE_LINE         1

#define LV_USE_ROLLER       0   /*Requires: lv_label*/
#if LV_USE_ROLLER
#  define LV_ROLLER_INF_PAGES       7   /*Number of extra "pages" when the roller is infinite*/
#endif

#define LV_USE_SLIDER       1   /*Requires: lv_bar*/

#define LV_USE_SWITCH    0

#define LV_USE_TEXTAREA   0     /*Requires: lv_label*/
#if LV_USE_TEXTAREA != 0
#  define LV_TEXTAREA_DEF_PWD_SHOW_TIME     1500    /*ms*/
#endif

#define LV_USE_TABLE  0

/*==================
 * EXTRA COMPONENTS
 *==================*/

/*-----------
 * Widgets
 *----------*/
#define LV_USE_CALENDAR     0
#if LV_USE_CALENDAR
# define LV_CALENDAR_WEEK_STARTS_MONDAY 0
# if LV_CALENDAR_WEEK_STARTS_MONDAY
#  define LV_CALENDAR_DEFAULT_DAY_NAMES {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}
# else
#  define LV_CALENDAR_DEFAULT_DAY_NAMES {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"}
# endif

# define LV_CALENDAR_DEFAULT_MONTH_NAMES {"January", "February", "March",  "April", "May",  "June", "July", "August", "September", "October", "November", "December"}
# define LV_USE_CALENDAR_HEADER_ARROW       1
# define LV_USE_CALENDAR_HEADER_DROPDOWN    1
#endif  /*LV_USE_CALENDAR*/

#define LV_USE_CHART        0

#define LV_USE_COLORWHEEL   0

#define LV_USE_GRIDVIEW     1

#define LV_USE_IMGBTN       1

#define LV_USE_KEYBOARD     0

#define LV_USE_LED          1

#define LV_USE_LIST         1

#define LV_USE_METER        0

#define LV_USE_MSGBOX       1

#define LV_USE_SPINBOX      0

#define LV_USE_SPINNER      1

#define LV_USE_TABVIEW      1

#define LV_USE_TILEVIEW     0

#define LV_USE_WIN          1

#define LV_USE_SPAN         1
#if LV_USE_SPAN
/*A line text can contain maximum num of span descriptor */
#  define LV_SPAN_SNIPPET_STACK_SIZE   64
#endif

/*-----------
 * Themes
 *----------*/
/*A simple, impressive and very complete theme*/
#define LV_USE_THEME_DEFAULT    0
#if LV_USE_THEME_DEFAULT

/*0: Light mode; 1: Dark mode*/
# define LV_THEME_DEFAULT_DARK     0

/*1: Enable grow on press*/
# define LV_THEME_DEFAULT_GROW              0

/*Default transition time in [ms]*/
# define LV_THEME_DEFAULT_TRANSITON_TIME    80
#endif /*LV_USE_THEME_DEFAULT*/

/*An very simple them that is a good starting point for a custom theme*/
#define LV_USE_THEME_BASIC    1

/*A theme designed for monochrome displays*/
#define LV_USE_THEME_MONO       0

/*-----------
 * Layouts
 *----------*/

/*A layout similar to Flexbox in CSS.*/
#define LV_USE_FLEX     1

/*A layout similar to Grid in CSS.*/
#define LV_USE_GRID     1

/*-----------
 * Extras
 *----------*/

#define LV_USE_FRAGMENT  1
#define LV_USE_QRCODE    1

#endif /*LV_CONF_H*/