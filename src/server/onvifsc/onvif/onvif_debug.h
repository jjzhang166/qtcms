#ifndef ONVIF_DEBUG_H
#define ONVIF_DEBUG_H

#define FONT_COLOR_BLACK	"30"
#define FONT_COLOR_RED		"31"
#define FONT_COLOR_GREEN	"32"
#define FONT_COLOR_YELLOW	"33"
#define FONT_COLOR_BLUE		"34"
#define FONT_COLOR_WHITE	"37"

#define FONT_BKG_BLACK		"40"
#define FONT_BKG_RED		"41"
#define FONT_BKG_GREEN		"42"
#define FONT_BKG_YELLOW		"43"
#define FONT_BKG_BLUE		"44"
#define FONT_BKG_WHITE		"47"

#define CLOSE_ALL_ATTR		"0m"
#define SET_HEIGHT			"1m"
#define SET_UNDERLINE		"4m"
#define SET_FLICKER			"5m"
#define CLEAR_SCREEN		"2J"


//#define DEBUG
#define ONVIF_SYNTAX "1;35"
#ifdef DEBUG
#define ONVIF_TRACE(fmt, arg...) \
	do{\
		printf("\033["ONVIF_SYNTAX"m[%12s:%4d]\033[0m ", __FILE__, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)
#define ONVIF_INFO(fmt, arg...) \
			do{\
				printf("\033["FONT_COLOR_WHITE"m[%12s:%4d]\033[0m ", __FILE__, __LINE__);\
				printf(fmt, ##arg);\
				printf("\r\n");\
			}while(0)
#define ONVIF_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			printf("\033["ONVIF_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", __FILE__, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)
#else
#define ONVIF_TRACE(fmt, ...) 
#define ONVIF_INFO(fmt, ...) \
		do{\
			printf("\033["ONVIF_SYNTAX"m[%12s:%4d]\033[0m ", __FILE__, __LINE__);\
			printf(fmt, __VA_ARGS__);\
			printf("\r\n");\
		}while(0)
#define ONVIF_ASSERT(exp, fmt, ...) \
	do{\
		if(!(exp)){\
			printf("\033["FONT_COLOR_WHITE"m[%12s:%4d]\033[0m assert(\"%s\") ", __FILE__, __LINE__, #exp);\
			printf(fmt, __VA_ARGS__);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)
#endif



#endif
