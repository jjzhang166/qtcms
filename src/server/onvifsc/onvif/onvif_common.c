
#include "onvif_common.h"

#include "onvif.nsmap"
#include "soapH.h"
#include "soapStub.h"
#include "wsddapi.h"
#include "onvif.h"
#include "onvifS.h"
#include "onvifC.h"
#include "generic.h"

#include "packbit.h"
#include "_base64.h"
#include "ezxml.h"
#include "onvif_debug.h"
#include "cross_thread.h"


#ifdef SOAP_SERVER
extern lpONVIF_S_CONTEXT g_OnvifServerCxt;
#endif
#ifdef SOAP_CLIENT
extern lpONVIF_C_CONTEXT g_OnvifClientCxt;
#endif

static THREAD_HANDLE g_search_pid = 0;
static bool g_search_trigger = false;
fON_WSDD_EVENT g_search_hook = NULL;
void *g_searchCustomCtx = NULL;

SOAP_NMAC struct Namespace discovery_namespaces[] =
{
	{"SOAP-ENV", "http://www.w3.org/2003/05/soap-envelope", "http://schemas.xmlsoap.org/soap/envelope/", NULL},
	{"SOAP-ENC", "http://www.w3.org/2003/05/soap-encoding", "http://schemas.xmlsoap.org/soap/encoding/", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"wsa", "http://schemas.xmlsoap.org/ws/2004/08/addressing", NULL, NULL},
	{"wsdd", "http://schemas.xmlsoap.org/ws/2005/04/discovery", NULL, NULL},
	{"tds", "http://www.onvif.org/ver10/device/wsdl", NULL, NULL},
	{"dn", "http://www.onvif.org/ver10/network/wsdl", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};



# define	tAGO	257
# define	tDST	258
# define	tDAY	259
# define	tDAY_UNIT	260
# define	tDAYZONE	261
# define	tHOUR_UNIT	262
# define	tLOCAL_ZONE	263
# define	tMERIDIAN	264
# define	tMINUTE_UNIT	265
# define	tMONTH	266
# define	tMONTH_UNIT	267
# define	tSEC_UNIT	268
# define	tYEAR_UNIT	269
# define	tZONE	270
# define	tSNUMBER	271
# define	tUNUMBER	272

#define HOUR(x) ((x) * 60)

/* An entry in the lexical lookup table.  */
typedef struct
{
  char const *name;
  int type;
  int value;
} tz_table_t;


static tz_table_t const time_zone_table[] =
{
{ "A",  tZONE,	 HOUR ( 1) }, /* Alpha Time Zone */
	{ "ACDT",	tDAYZONE,	 (HOUR ( 10) + 30) }, /* Australian Central Daylight Time */
	{ "ACST",	tZONE,	 (HOUR (9 ) + 30) }, /* Australian Central Daylight Time */
	{ "AEDT",	tDAYZONE,	 HOUR ( 11) }, /* Australian Central Standard Time  */
	{ "AEST",	tZONE,	 HOUR (10 ) }, /* Australian Eastern Standard Time */
	{ "AFT",	tZONE,	 (HOUR (4 ) + 30) }, /* Afghanistan Time */
	{ "ALMT",	tZONE,	 HOUR (6 ) }, /* Afghanistan Time */
	{ "AMST",	tDAYZONE,	 HOUR ( 5) }, /* Armenia Summer Time  */
	{ "AMT",	tZONE,	 HOUR (4 ) }, /* Armenia Time */
	{ "AMT",	tZONE,	 -HOUR (4) }, /* Amazon Time*/
	{ "ANAST",	tDAYZONE,	 HOUR (12 ) }, /*  Anadyr Summer Time */
	{ "ANAT",	tZONE,	 HOUR ( 12) }, /*  Anadyr Time */
	{ "AQTT",	tZONE,	 HOUR (5 ) }, /* Aqtobe Time */
	{ "AWDT",	tDAYZONE,	 HOUR ( 9 ) }, /* Australian Western Daylight Time */
	{ "AWST",	tZONE,	 HOUR (8 ) }, /* Australian Western Standard Time */
	{ "AZOST",	tDAYZONE,	 HOUR (0 ) }, /* Azores Summer Time  */
	{ "AZOT",	tZONE,	 -HOUR ( 1) }, /* Azores Time */
	{ "AZST",	tDAYZONE,	 HOUR (5 ) }, /* Azerbaijan Summer Time  */
	{ "AZT",	tZONE,	 HOUR ( 4) }, /* Azerbaijan Time */
	{ "B",	tZONE,	 HOUR (2 ) }, /* Bravo Time Zone */
	{ "BNT",	tZONE,	 HOUR ( 8) }, /* Brunei Darussalam Time */
	{ "BOT",	tZONE,	 -HOUR (4 ) }, /* Bolivia Time */
	{ "BTT",	tZONE,	 HOUR ( 6) }, /* Bhutan Time */
	{ "C",	tZONE,	 HOUR ( 3) }, /* Charlie Time Zone */
	{ "CAST",	tZONE,	 HOUR (8 ) }, /* Casey Time */
	{ "CCT",	tZONE,	 (HOUR (6 ) + 30) }, /* Cocos Islands Time */
	{ "CHADT",	tDAYZONE,	 (HOUR (13 ) + 45) }, /* Chatham Island Daylight Time */
	{ "CHAST",	tZONE,	 (HOUR ( 12) + 45) }, /* Chatham Island Standard Time */
	{ "CKT",	tZONE,	 -HOUR (10 ) }, /* Cook Island Time */
	{ "COT",	tZONE,	 -HOUR ( 5) }, /* Colombia Time */
	{ "CVT",	tZONE,	 -HOUR (1 ) }, /* Cape Verde Time */
	{ "CXT",	tZONE,	 HOUR ( 7) }, /* Christmas Island Time */
	{ "ChST",	tZONE,	 HOUR (10 ) }, /* Chamorro Standard Time */
	{ "D",	tZONE,	 HOUR (4 ) }, /* Delta Time Zone */
	{ "DAVT",	tZONE,	 HOUR ( 7) }, /* Davis Time */
	{ "E",	tZONE,	 HOUR (5 ) }, /* Echo Time Zone */
	{ "EASST",	tZONE,	 -HOUR (5 ) }, /* Easter Island Summer Time */
	{ "EAST",	tZONE,	 -HOUR ( 6) }, /* Easter Island Standard Time */
	{ "ECT",	tZONE,	 -HOUR (5 ) }, /* Ecuador Time */
	{ "EGST",	tDAYZONE,	 HOUR (0 ) }, /* Eastern Greenland Summer Time */
	{ "EGT",	tZONE,	 -HOUR ( 1) }, /* East Greenland Time */
	{ "ET",	tZONE,	 -HOUR ( 5) }, /* Tiempo del Este */
	{ "F",	tZONE,	 HOUR (6 ) }, /* Foxtrot Time Zone */
	{ "FJST",	tDAYZONE,	 HOUR (13 ) }, /* Fiji Summer Time */
	{ "FJT",	tZONE,	 HOUR (12 ) }, /* Fiji Time */
	{ "FKST",	tDAYZONE,	 -HOUR (3 ) }, /* Falkland Islands Summer Time */
	{ "FKT",	tZONE,	 -HOUR (4 ) }, /* Falkland Island Time */
	{ "FNT",	tZONE,	 -HOUR ( 2) }, /* Fernando de Noronha Time */
	{ "G",	tZONE,	 HOUR (7 ) }, /* Golf Time Zone */
	{ "GALT",	tZONE,	 -HOUR (6 ) }, /* Galapagos Time */
	{ "GAMT",	tZONE,	 -HOUR ( 9) }, /* Gambier Time */
	{ "GET",	tZONE,	 HOUR (4 ) }, /* Georgia Standard Time */
	{ "GFT",	tZONE,	 -HOUR (3 ) }, /* French Guiana Time */
	{ "GILT",	tZONE,	 HOUR (12 ) }, /* Gilbert Island Time */
	{ "GYT",	tZONE,	 -HOUR ( 4) }, /* Guyana Time */
	{ "H",	tZONE,	 HOUR ( 8) }, /* Hotel Time Zone */
	{ "HAA",	tZONE,	 -HOUR (3 ) }, /* Heure Avanc¨¦e de l'Atlantique */
	{ "HAC",	tZONE,	 -HOUR (5 ) }, /* Heure Avanc¨¦e du Centre */
	{ "HAE",	tZONE,	 -HOUR (4 ) }, /* Heure Avanc¨¦e de l'Est */
	{ "HAP",	tZONE,	 -HOUR (7 ) }, /* Heure Avanc¨¦e du Pacifique */
	{ "HAR",	tZONE,	 -HOUR (6 ) }, /* Heure Avanc¨¦e des Rocheuses */
	{ "HAT",	tZONE,	 -(HOUR ( 2) + 30) }, /* Heure Avanc¨¦e de Terre-Neuve */
	{ "HAY",	tZONE,	 -HOUR (8 ) }, /* Heure Avanc¨¦e du Yukon */
	{ "HKT",	tZONE,	 HOUR (8 ) }, /* Hong Kong Time */
	{ "HLV",	tZONE,	 -(HOUR ( 4) + 30) }, /* Hora Legal de Venezuela */
	{ "HNA",	tZONE,	 -HOUR (4 ) }, /* Heure Normale de l'Atlantique */
	{ "HNC",	tZONE,	 -HOUR ( 6) }, /* Heure Normale du Centre */
	{ "HNE",	tZONE,	 -HOUR (5 ) }, /* Heure Normale de l'Est */
	{ "HNP",	tZONE,	 -HOUR ( 8) }, /* Heure Normale du Pacifique */
	{ "HNR",	tZONE,	 -HOUR (7 ) }, /* Heure Normale des Rocheuses */
	{ "HNT",	tZONE,	 -(HOUR ( 3) + 30) }, /* Heure Normale de Terre-Neuve */
	{ "HNY",	tZONE,	 -HOUR (9 ) }, /* Heure Normale du Yukon */
	{ "HOVT",	tZONE,	 HOUR ( 7) }, /* Hovd Time */
	{ "I",	tZONE,	 HOUR (9 ) }, /* India Time Zone */
	{ "ICT",	tZONE,	 HOUR ( 7) }, /* Indochina Time */
	{ "IDT",	tDAYZONE,	 HOUR ( 3) }, /* Israel Daylight Time */
	{ "IOT",	tZONE,	 HOUR (6 ) }, /* Indian Chagos Time */
	{ "IRDT",	tDAYZONE,	 (HOUR (4 ) + 30) }, /* Iran Daylight Time */
	{ "IRKST",	tDAYZONE,	 HOUR (9 ) }, /* Irkutsk Summer Time */
	{ "IRKT",	tZONE,	 HOUR ( 9) }, /* Irkutsk Time */
	{ "IRST",	tZONE,	 (HOUR ( 3) + 30) }, /* Iran Standard Time */
	{ "K",	tZONE,	 HOUR ( 10) }, /* Kilo Time Zone */
	{ "KGT",	tZONE,	 HOUR ( 6) }, /* Kyrgyzstan Time */
	{ "KRAST",	tDAYZONE,	 HOUR ( 8) }, /* Krasnoyarsk Summer Time */
	{ "KRAT",	tZONE,	 HOUR ( 8) }, /* Krasnoyarsk Time */
	{ "KUYT",	tZONE,	 HOUR ( 4) }, /* Kuybyshev Time */
	{ "L",	tZONE,	 HOUR ( 11) }, /* Lima Time Zone */
	{ "LHDT",	tDAYZONE,	 HOUR ( 11) }, /* Lord Howe Daylight Time */
	{ "LHST",	tZONE,	 (HOUR (10 ) + 30) }, /* Lord Howe Standard Time */
	{ "LINT",	tZONE,	 HOUR (14 ) }, /* Line Islands Time */
	{ "M",	tZONE,	 HOUR ( 12) }, /* Mike Time Zone */
	{ "MAGST",	tDAYZONE,	 HOUR ( 12) }, /* Magadan Summer Time */
	{ "MAGT",	tZONE,	 HOUR ( 12) }, /* Magadan Time */
	{ "MART",	tZONE,	 -(HOUR (9 ) + 30) }, /*Marquesas Time  */
	{ "MAWT",	tZONE,	 HOUR ( 5) }, /* Mawson Time */
	{ "MHT",	tZONE,	 HOUR ( 12) }, /* Marshall Islands Time */
	{ "MMT",	tZONE,	 (HOUR (6 )  + 30) }, /* Myanmar Time */
	{ "MUT",	tZONE,	 HOUR (4 ) }, /* Mauritius Time */
	{ "MVT",	tZONE,	 HOUR (5 ) }, /*  Maldives Time */
	{ "MYT",	tZONE,	 HOUR (8 ) }, /* Malaysia Time */
	{ "N",	tZONE,	 -HOUR ( 1) }, /* November Time Zone */

  { "GMT",	tZONE,     HOUR ( 0) },	/* Greenwich Mean */
  { "UT",	tZONE,     HOUR ( 0) },	/* Universal (Coordinated) */
  { "UTC",	tZONE,     HOUR ( 0) },
  { "WET",	tZONE,     HOUR ( 0) },	/* Western European */
  { "WEST",	tDAYZONE,  HOUR ( 0) },	/* Western European Summer */
  { "BST",	tDAYZONE,  HOUR ( 0) },	/* British Summer */
  { "ART",	tZONE,	  -HOUR ( 3) },	/* Argentina */
  { "BRT",	tZONE,	  -HOUR ( 3) },	/* Brazil */
  { "BRST",	tDAYZONE, -HOUR ( 3) },	/* Brazil Summer */
  { "NST",	tZONE,	 -(HOUR ( 3) + 30) },	/* Newfoundland Standard */
  { "NDT",	tDAYZONE,-(HOUR ( 3) + 30) },	/* Newfoundland Daylight */
  { "AST",	tZONE,    -HOUR ( 4) },	/* Atlantic Standard */
 { "AST",  tZONE,	 HOUR ( 3) }, /* Arabia Standard Time */
  { "ADT",	tDAYZONE, -HOUR ( 4) },	/* Atlantic Daylight */
  { "CLT",	tZONE,    -HOUR ( 4) },	/* Chile */
  { "CLST",	tDAYZONE, -HOUR ( 4) },	/* Chile Summer */
  { "EST",	tZONE,    -HOUR ( 5) },	/* Eastern Standard */
  { "EDT",	tDAYZONE, -HOUR ( 5) },	/* Eastern Daylight */
  { "CST",	tZONE,    -HOUR ( 6) },	/* Central Standard */
{ "CST",  tZONE,	HOUR ( 8) }, /* China Standard Time */
{ "CST",  tZONE,	-HOUR ( 5) }, /* Cuba Standard Time*/
  { "CDT",	tDAYZONE, -HOUR ( 4) },	/* Cuba Daylight Time */
  { "CDT",  tDAYZONE, -HOUR ( 5) }, /* Central Daylight */
  { "MST",	tZONE,    -HOUR ( 7) },	/* Mountain Standard */
  { "MDT",	tDAYZONE, -HOUR ( 7) },	/* Mountain Daylight */
  { "PST",	tZONE,    -HOUR ( 8) },	/* Pacific Standard */
  { "PDT",	tDAYZONE, -HOUR ( 8) },	/* Pacific Daylight */
  { "AKST",	tZONE,    -HOUR ( 9) },	/* Alaska Standard */
  { "AKDT",	tDAYZONE, -HOUR ( 9) },	/* Alaska Daylight */
  { "HST",	tZONE,    -HOUR (10) },	/* Hawaii Standard */
  { "HAST",	tZONE,	  -HOUR (10) },	/* Hawaii-Aleutian Standard */
  { "HADT",	tDAYZONE, -HOUR (10) },	/* Hawaii-Aleutian Daylight */
  { "SST",	tZONE,    -HOUR (12) },	/* Samoa Standard */
  { "WAT",	tZONE,     HOUR ( 1) },	/* West Africa */
  { "CET",	tZONE,     HOUR ( 1) },	/* Central European */
 { "CEST",	tDAYZONE,  HOUR ( 2) },	/* Central European Summer */
  { "MET",	tZONE,     HOUR ( 1) },	/* Middle European */
  { "MEZ",	tZONE,     HOUR ( 1) },	/* Middle European */
  { "MEST",	tDAYZONE,  HOUR ( 1) },	/* Middle European Summer */
  { "MESZ",	tDAYZONE,  HOUR ( 2) },	/* Middle European Summer */
  { "EET",	tZONE,     HOUR ( 2) },	/* Eastern European */
  { "EEST",	tDAYZONE,  HOUR ( 2) },	/* Eastern European Summer */
  { "CAT",	tZONE,	   HOUR ( 2) },	/* Central Africa */
  { "SAST",	tZONE,	   HOUR ( 2) },	/* South Africa Standard */
  { "EAT",	tZONE,	   HOUR ( 3) },	/* East Africa */
	{ "EAT",  tZONE,	 HOUR ( 3) }, /* Eastern Africa Time */
  { "MSK",	tZONE,	   HOUR ( 4) },	/* Moscow */
  { "MSD",	tDAYZONE,  HOUR ( 4) },	/* Moscow Daylight */
  { "IST",	tZONE,	  (HOUR ( 5) + 30) },	/* India Standard */
	{ "IST",  tZONE,	HOUR ( 2)  },   /* Israel Standard Time*/
	{ "IST",  tZONE,	HOUR ( 1) },   /* Irish Standard Time */
  { "SGT",	tZONE,     HOUR ( 8) },	/* Singapore */
  { "KST",	tZONE,     HOUR ( 9) },	/* Korea Standard */
  { "JST",	tZONE,     HOUR ( 9) },	/* Japan Standard */
  { "GST",	tZONE,     HOUR (10) },	/* Guam Standard */
  { "NZST",	tZONE,     HOUR (12) },	/* New Zealand Standard */
  { "NZDT",	tDAYZONE,  HOUR (12) },	/* New Zealand Daylight */
  { 0, 0, 0  }
};


int tzone_s2int(char *szone, int *value /* unit: second */)
{
#define TZ_MAX_LEN	(128)
	/* tzn[+|-]hh[:mm[:ss]][dzn] ....*/
	char tzn[TZ_MAX_LEN + 1];
	char dzn[TZ_MAX_LEN + 1];
	char tzinfo[TZ_MAX_LEN + 1];
	char toffset[TZ_MAX_LEN + 1];
	int i;
	char *ptr = szone;
	tz_table_t *pt = NULL;
	int sign = 1;
	int ret , t1 = 0, t2 = 0, t3 = 0;
	
	if (strlen(szone) > TZ_MAX_LEN) {
		return -1;
	}
	i = 0;
	while(isalpha(*ptr)) tzn[i++] = *ptr++;
	tzn[i] = 0;
	// check tzn...
	for (pt = (tz_table_t *)time_zone_table; pt->name; pt++) {
		if (strcmp(tzn, pt->name) == 0) {
			break;
		}
	}
	if (pt->name == NULL) {
		printf("invalid tzname abbreviations:%s is invalid\n", tzn);
		return -1;
	}
	printf("tzname: %s\n", tzn);
	// parse sign 
	if(strcmp(tzn, "CST") == 0){//fix me  :CST for china standar time
		if (*ptr) {
			if (*ptr == '+') {
				sign = -1;
				ptr++;
			} else if (*ptr == '-') {
				sign  = 1;
				ptr++;
			} else{
				sign = 1;
			}
		}
	}else{
		if (*ptr) {
			if (*ptr == '+') {
				sign = 1;
				ptr++;
			} else if (*ptr == '-') {
				sign  = -1;
				ptr++;
			} else{
				sign = 1;
			}
		}
	}
	// parse time offset
	if (*ptr) {
		i = 0;
		while(isdigit(*ptr) || (*ptr == ':')) toffset[i++] = *ptr++;
		toffset[i] = 0;
		ret = sscanf(toffset, "%d:%d:%d", &t1, &t2, &t3);
		if (ret == 3) {
		} else if (ret == 2) {

		} else if ( ret  == 1) {
			
		}
		if ((t1 < 0) || (t1 > 12)
			|| (t2 < 0) || (t2 >= 60)
			|| (t3 < 0) || (t3 >= 60)) {
			printf("invalid time offset, %d:%d:%d (%s)\n", t1, t2, t3 , toffset);
			return -1;
		}
	}
	// dzn	
	if(*ptr) {
		i = 0;
		while(isalpha(*ptr)) dzn[i++] = *ptr++;
		dzn[i] = 0;
		printf("dzn: %s\n", dzn);
	}
	// time info
	if(*ptr) {
		i = 0;
		while(*ptr) tzinfo[i++] = *ptr++;
		tzinfo[i] = 0;
		printf("tzinfo: %s\n", tzinfo);
	}

	//*value = t1 * 100 + t2;
	*value  =  sign * (t1 * 3600 + t2 * 60 + t3);
	
	return 0;
}

char *tzone_int2s(int gmt, char *result, int size)
{
	if(NULL != result){
		if(0 == gmt){
			SNPRINTF(result, size, "GMT0");
		}else{
			SNPRINTF(result, size, "GMT%c%02d:%02d", gmt >= 0 ? '+' : '-',
				abs(gmt) / 3600, (abs(gmt) % 3600)/60);
		}
		return result;
	}
	return NULL;
}

int tzone_value_to_second(int gmt)
{
	int hour = 0; 
	int minute = 0;
	int offset_sec = 0;

	hour = abs(gmt) / 100;
	minute = abs(gmt) % 100;

	offset_sec = hour * 3600 + minute * 60;
	if (gmt < 0) {
		offset_sec *= -1;
	}
	return offset_sec;
}

int tzone_value_from_second(int second)
{
	int gmt = 0;
	int hour = 0, minute = 0;

	hour  = abs(second) / 3600;
	minute = (abs(second) % 3600)/60;
	gmt = hour * 100 + minute;
	if (second < 0) 
		gmt *= -1;
	return gmt;
}


int get_int_string_format(char *src, char *format)
{
	/*
	* for example : the source string is: Preset-0, Preset-1, this string format is : Preset-%d and basebum is 0
	* return value: 
	* 		error , return -1;
	*		else return number of digit_occur
	*/
	char *ptr  = src;
	int i = 0;
	int digit_occur = 0;
	
	if (src == NULL || format == NULL)
		return -1;

	while(*ptr) {
		if (!isdigit(*ptr)) {
			// check not digit
			while(*ptr && (!isdigit(*ptr))) format[i++] = *ptr++;
			if (*ptr == 0) {
				break;
			}else {
			}
		} else {
			// check digit
			format[i++] = '%';
			format[i++] = 'd';
			digit_occur++;
			while(*ptr && (isdigit(*ptr))) ptr++;
			if (*ptr == 0) {
				break;
			}
		}
	}
	format[i] = 0;

	return digit_occur;
}


int md_celllayout_s2hex(char *sz_layout, uint8_t *out, int size)
{
	int ret;
	uint8_t pack_buf[1024];
	// base64 decode -> packbit decode
	memset(out, 0, size);
	memset(pack_buf, 0, sizeof(pack_buf));
	if (( ret = BASE64_decode(sz_layout, strlen(sz_layout), pack_buf, sizeof(pack_buf))) > 0) {
		return PACKBITS_decode(out, pack_buf, size, ret);
	}
	return 0;
}

void md_celllayout_hex2s(char *sz_layout, int out_size, uint8_t *input, int in_size)
{
	int ret;
	uint8_t pack_buf[1024];
	// packbit encode -> base64 encode
	memset(sz_layout, 0, out_size);
	memset(pack_buf, 0, sizeof(pack_buf));
	ret = PACKBITS_encode(input, pack_buf, in_size);
	ret = BASE64_encode(pack_buf, ret, sz_layout, out_size);
	if (ret > 0)
		sz_layout[ret] = '\0';
}

void md_celllayout_hex2s_ex(char *sz_layout, int out_size, uint8_t *input, int row, int column)
{
	int ret;
	uint8_t pack_buf[1024];
	int size = 0;
	// packbit encode -> base64 encode
	size = (row * column + 7) / 8;
	memset(sz_layout, 0, out_size);
	memset(pack_buf, 0, sizeof(pack_buf));
	ret = PACKBITS_encode(input, pack_buf, size);
	ret = BASE64_encode(pack_buf, ret, sz_layout, out_size);
	if (ret > 0)
		sz_layout[ret] = '\0';
}

int netmask_to_prefixlength(unsigned char *netmask)
{
	int i = 0;
	unsigned int u32mask = 0;
	u32mask |=  (netmask[0] << 24) | (netmask[1] << 16) | (netmask[2] << 8) | netmask[3];
	for ( i = 0; i < 32; i++){
		if (!( u32mask & ( 1 << (32 - 1 -i)))){
			break;
		}
	}
	return i;
}

int netmask_to_prefixlength2(char *netmask)
{
	int i = 0;
	int tmp[4];
	int u8mask[4];
	unsigned int u32mask = 0;
	if (sscanf(netmask, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) == 4) {
		u8mask[0] = tmp[0];
		u8mask[1] = tmp[1];
		u8mask[2] = tmp[2];
		u8mask[3] = tmp[3];
	} else {
		return 24;
	}
	u32mask |=  (u8mask[0] << 24) | (u8mask[1] << 16) | (u8mask[2] << 8) | u8mask[3];
	for ( i = 0; i < 32; i++){
		if (!( u32mask & ( 1 << (32 - 1 -i)))){
			break;
		}
	}
	return i;
}

void netmask_from_prefixlength(int length, unsigned char *netmask)
{
	int i;
	unsigned int u32mask = 0;
	for ( i = 31; i >= 0 && length > 0; i--,length--)
	{
		u32mask |= 1 << i;
	}
	netmask[0] = (u32mask >> 24) & 0xff;
	netmask[1] = (u32mask >> 16) & 0xff;
	netmask[2] = (u32mask >> 8)  & 0xff;
	netmask[3] = u32mask & 0xff;
}

void netmask_from_prefixlength2(int length, char *sznetmask)
{
	int i;
	unsigned char netmask[4];
	unsigned int u32mask = 0;
	for ( i = 31; i >= 0 && length > 0; i--,length--)
	{
		u32mask |= 1 << i;
	}
	netmask[0] = (u32mask >> 24) & 0xff;
	netmask[1] = (u32mask >> 16) & 0xff;
	netmask[2] = (u32mask >> 8)  & 0xff;
	netmask[3] = u32mask & 0xff;
	sprintf(sznetmask, "%d.%d.%d.%d", netmask[0], netmask[1], netmask[2], netmask[3]);
}

/* @brief Check if IP is valid */
int isValidIp4 (char *str) 
{
	int segs = 0;   /* Segment count. */     
	int chcnt = 0;  /* Character count within segment. */     
	int accum = 0;  /* Accumulator for segment. */      
	/* Catch NULL pointer. */      
	if (str == NULL) return 0;      
	/* Process every character in string. */      
	while (*str != '\0') 
	{         
		/* Segment changeover. */          
		if (*str == '.') 
		{             
			/* Must have some digits in segment. */              
			if (chcnt == 0) return 0;              
			/* Limit number of segments. */              
			if (++segs == 4) return 0;              
			/* Reset segment values and restart loop. */              
			chcnt = accum = 0;             
			str++;             
			continue;         
		}  

		/* Check numeric. */          
		if ((*str < '0') || (*str > '9')) return 0;
		/* Accumulate and check segment. */          
		if ((accum = accum * 10 + *str - '0') > 255) return 0;
		/* Advance other segment specific stuff and continue loop. */          
		chcnt++;         
		str++;     
	}      
	/* Check enough segments and enough characters in last segment. */      
	if (segs != 3) return 0;      
	if (chcnt == 0) return 0;      
	/* Address okay. */      
	return 1; 
} 


int sztime_to_int(char *sz_time)
{
	/*
	* PT[%dH][%dM][%dS].[....]
	*/
	char *ptr = sz_time;
	char tmp[32];
	int i, ret;
	int y = 0, m = 0, s = 0;
	if (sz_time[0] != 'P' || sz_time[1] != 'T') {
		return 0;
	}
	ptr += 2;
	for (;;) {
		i = 0;
		while(isdigit(*ptr) && *ptr != '\0'  && i < 10) tmp[i++] = *ptr++;
		if (i >= 10)
			break;
		else if (i > 0) {
			tmp[i] = '\0';
			if ( *ptr == 'H')
				sscanf(tmp, "%d", &y);
			else if (*ptr == 'M')
				sscanf(tmp, "%d", &m);
			else if (*ptr == 'S') 
				sscanf(tmp, "%d", &s);
			else if (*ptr == '\0')
				break;
			//printf("tmp:%s ptr=%c (%d,%d,%d)\n", tmp, *ptr, y, m, s);
		} else {
			break;
		}
		while((!isdigit(*ptr)) && (*ptr != '\0')) {
			ptr++;
		}
		if (*ptr == '\0') break;
	}
	ret = 3600*y + 60*m + s;
	//printf("%s -> %d(%d,%d,%d)\n", sz_time, ret, y, m, s);
	return ret;
}

void sztime_from_int(int time, char *sz_time)
{
	char tmp[64];
	if(time >= 3600){
		sprintf(sz_time,  "PT%dH",time/3600);
		if ((time % 3600) >= 60) {
			sprintf(tmp, "%dM", (time % 3600)/60);
			strcat(sz_time, tmp);
		}
		if ((time % 60) > 0) {
			sprintf(tmp, "%dS", time % 60);
			strcat(sz_time, tmp);
		}
	}else if(time >= 60){
		if((time % 60) == 0)
			sprintf(sz_time, "PT%dM",time/60);
		else
			sprintf(sz_time, "PT%dM%dS",time/60,time % 60);
	}else{
		sprintf(sz_time, "PT%dS", time);
	}
}

void get_current_time(char *sztime)
{
	/* 2014-02-21T15:29:07Z */
	time_t t_now;
	struct tm *ptm;
	time(&t_now);
	ptm = localtime(&t_now);
	sprintf(sztime, "%d-%02d-%02dT%02d:%02d:%02dZ", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

void get_terminal_time(int64_t timeout, char *sztime)
{
	/* 2014-02-21T15:29:07Z */
	time_t t_now;
	struct tm ptm;
	time(&t_now);
	printf("current %ld timout: %lld\n", t_now, timeout);
	t_now += timeout;
	printf("current %ld timout: %lld  --\n", t_now, timeout);
	localtime_c(&t_now, &ptm);
	//memcpy(&ptm, localtime(&t_now), sizeof(ptm));
	sprintf(sztime, "%d-%02d-%02dT%02d:%02d:%02dZ", ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday,
		ptm.tm_hour, ptm.tm_min, ptm.tm_sec);
}

int datetime_s2tm(char *sztime, struct tm *ptm, time_t *timet)
{
	/* 2014-02-21T15:29:07Z */
	const char tchar[6] = {'-', '-', 'T', ':' , ':', 'Z' };
	int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;
	int *dt_i[6];
	int n = 0;
	int i;
	char *ptr = sztime;
	char tmp[20];
	time_t t_now;
	struct tm datetime, *pdt= NULL;
	
	dt_i[0] = &y;
	dt_i[1] = &M;
	dt_i[2] = &d;
	dt_i[3] = &h;
	dt_i[4] = &m;
	dt_i[5] = &s;
	while (*ptr ) {
		i = 0;
		while(isdigit(*ptr) && *ptr != '\0'  && i < 10) tmp[i++] = *ptr++;
		if (i == 0 || i >= 5)
			return -1;
		else if (i > 0) {
			tmp[i] = 0;
			//printf("%s %d,%d\n", tmp, i, n);
			if (sscanf(tmp, "%d", dt_i[n]) != 1) {
				printf("not exptected format\n");
				return -1;
			}
			if (*ptr != tchar[n]) {
				printf("not expected char ...\n");
				return -1;
			}
			ptr++;
			if (*ptr!='\0') {
				if (!isdigit(*ptr)) {
					printf("not digit\n");
					return -1;
				}
			} else {
				break;
			}
			n++;
		}
	}

	time(&t_now);
	printf("%d-%d-%d %d:%d:%d /%s", y, M, d, h, m, s, ctime(&t_now));
	if (y < 1900
		|| (M <=0 || M > 12)
		|| (d <=0 || d > 31)
		|| (h < 0 || h > 24)
		|| (m < 0 || m > 60)
		|| (s < 0 || s > 60)){
		printf("datetime is invalid\n");
		return -1;
	}

	pdt = &datetime;
	pdt->tm_year = y - 1900;
	pdt->tm_mon = M -1;
	pdt->tm_mday = d;
	pdt->tm_hour = h;
	pdt->tm_min  = m;
	pdt->tm_sec = s;
	if (ptm) {
		memcpy(ptm, pdt, sizeof(struct tm));
	}
	if (timet) *timet = mktime(pdt);
	
	return 0;
}


int ONVIF_check_uri(char *http_uri, int uri_size)
{
	if(STR_CASE_THE_SAME(http_uri, "/onvif/device_service")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/media")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/imaging")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/device")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/events")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/analytics")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/ptz")
		|| STR_CASE_THE_SAME(http_uri, "/onvif/services")
		|| strstr(http_uri, ONVIF_EVENT_PULLPOINT_URI_PREFIX)){
		return 0;
	}
	return -1;
}

int ONVIF_SERVER_daemon(int sock)
{
	int ret = 0;
	struct soap *add_soap = soap_new();
	if(add_soap == NULL){
		ONVIF_INFO("soap new failed!");
		return -1;
	}
	
	soap_set_namespaces(add_soap, namespaces);
	add_soap->bind_flags = add_soap->bind_flags | SO_REUSEADDR;
	add_soap->connect_timeout = 8;
	add_soap->recv_timeout = 8;
	add_soap->send_timeout = 8;
	add_soap->socket = sock;

	ret = soap_serve(add_soap);
	if (ret != SOAP_OK) {
		ONVIF_INFO("soap serve error : %d(%s)!", ret, add_soap->endpoint);
		return -1;
	}
	soap_destroy(add_soap);
	soap_end(add_soap);
	soap_free(add_soap);

	return 0;
}

static int parse_msg_content(char *msg, char *sznode, char *name, char *value)
{
	char msg_bak[5000];
	ezxml_t msg_xml = NULL, node = NULL, item = NULL;
	int ret = -1;

	strncpy(msg_bak, msg, sizeof(msg_bak) - 1);
	if((msg_xml= ezxml_parse_str(msg_bak, strlen(msg_bak))) != NULL){
		node = ezxml_child(msg_xml, sznode);
		if (node ) {
			item = ezxml_child(node, "tt:SimpleItem");
			//item = ezxml_idx(node, 0);
			while(item) {
				char *p_n = NULL;
				char *p_v = NULL;

				p_n = (char *)ezxml_attr(item, "Name");
				p_v = (char *)ezxml_attr(item, "Value");
				if (p_n && p_v && strcmp(p_n, name) == 0){
					strcpy(value, p_v);
					ret = 0;
					break;
				}

				item = ezxml_next(item);
			}
		}
	}

	if (msg_xml) 
		ezxml_free(msg_xml);

	return ret;
	/*
#ifdef WIN32
	return 0;
#else
	char msg_bak[5000];
	ezxml_t msg_xml = NULL, node = NULL, item = NULL;
	int ret = -1;

	strncpy(msg_bak, msg, sizeof(msg_bak) - 1);
	if((msg_xml= ezxml_parse_str(msg_bak, strlen(msg_bak))) != NULL){
		node = ezxml_child(msg_xml, sznode);
		if (node ) {
			item = ezxml_child(node, "tt:SimpleItem");
			//item = ezxml_idx(node, 0);
			while(item) {
				char *p_n = NULL;
				char *p_v = NULL;
				
				p_n = (char *)ezxml_attr(item, "Name");
				p_v = (char *)ezxml_attr(item, "Value");
				if (p_n && p_v && strcmp(p_n, name) == 0){
					strcpy(value, p_v);
					ret = 0;
					break;
				}
				
				item = ezxml_next(item);
			}
		}
	}

	if (msg_xml) 
		ezxml_free(msg_xml);
	
	return ret;
#endif
	*/
}

static int get_msg_str_value(char *msg, char *sznode, char *name, char *value)
{
	return parse_msg_content(msg, sznode, name, value);
}

static int get_msg_int_value(char *msg, char *sznode, char *name, int *value)
{
	char szvalue[64];
	if (parse_msg_content(msg, sznode, name, szvalue) == 0) {
		*value = atoi(szvalue);
		return 0;
	} else {
		return -1;
	}
}

static int get_msg_boolean_value(char *msg, char *sznode, char *name, bool *value)
{
	char szvalue[64];
	if (parse_msg_content(msg, sznode, name, szvalue) == 0) {
		if (strcmp(szvalue, "true") == 0) 
			*value = true;
		else
			*value =false;
		return 0;
	} else {
		return -1;
	}
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify(struct soap *soap, struct _wsnt__Notify *wsnt__Notify) 
{
#ifdef SOAP_CLIENT
	int i;
	stONVIF_EVENT_SUBSCRIBE subscribe;

	memset(&subscribe, 0, sizeof(subscribe));
	
	if (wsnt__Notify->__sizeNotificationMessage <= 0 || wsnt__Notify->NotificationMessage == NULL) {
		ONVIF_INFO("event notify, but no event carried");
		return SOAP_FAULT;
	}
	
	ONVIF_INFO("event notify, reference: %s", soap->endpoint);

	for ( i = 0; i < wsnt__Notify->__sizeNotificationMessage; i++) {
		if (wsnt__Notify->NotificationMessage[i].ProducerReference) {
			ONVIF_TRACE("peer reference: %s", wsnt__Notify->NotificationMessage[i].ProducerReference->Address);
		}
		if (wsnt__Notify->NotificationMessage[i].Topic) {
			ONVIF_TRACE("dialect:%s", wsnt__Notify->NotificationMessage[i].Topic->Dialect);
			ONVIF_TRACE("topic:%s", wsnt__Notify->NotificationMessage[i].Topic->__any);
			ONVIF_TRACE("message:\r\n%s", wsnt__Notify->NotificationMessage[i].Message.__any);
			if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "RuleEngine/CellMotionDetector/Motion") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_MD)) {
						char source[32], van[32];
						bool ismotion;
						get_msg_str_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Source","VideoSourceConfigurationToken", source);
						get_msg_str_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Source","VideoAnalyticsConfigurationToken", van);
						get_msg_boolean_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Data","IsMotion", &ismotion);
						ONVIF_INFO("source:%s van:%s ismotion:%s", source, van, ismotion ? "true" : "false");
						if (ismotion == true)
							subscribe.hook(NVP_EVENT_MD, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "VideoSource/MotionAlarm") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_MD)) {
						char source[32];
						bool state;
						get_msg_str_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Source","VideoSourceToken", source);
						get_msg_boolean_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Data","State", &state);
						if (state == true)
							subscribe.hook(NVP_EVENT_MD, 1, (unsigned int)source, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "VideoSource/SignalLoss") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_VIDEO_LOSS)) {
						subscribe.hook(NVP_EVENT_VIDEO_LOSS, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "VideoSource/ImageTooBlurry") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_TOO_BLURRY)) {
						subscribe.hook(NVP_EVENT_TOO_BLURRY, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "VideoSource/ImageTooDark") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_TOO_DARK)) {
						subscribe.hook(NVP_EVENT_TOO_DARK, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "VideoSource/ImageTooBright") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_TOO_BRIGHT)) {
						subscribe.hook(NVP_EVENT_TOO_BRIGHT, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}		
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "VideoSource/GlobalSceneChange") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_SCENE_CHANGE)) {
						subscribe.hook(NVP_EVENT_SCENE_CHANGE, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "Configuration/Profile") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_PROFILE_CHANGED)) {
						subscribe.hook(NVP_EVENT_PROFILE_CHANGED, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "Configuration/VideoSourceConfiguration/MediaService") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_VSOURCE_CONF_CHANGED)) {
						subscribe.hook(NVP_EVENT_VSOURCE_CONF_CHANGED, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "Configuration/VideoEncoderConfiguration") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_VENC_CONF_CHANGED)) {
						subscribe.hook(NVP_EVENT_VENC_CONF_CHANGED, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "Configuration/VideoOutputConfiguration/MediaService") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_VOUT_CONF_CHANGED)) {
						subscribe.hook(NVP_EVENT_VOUT_CONF_CHANGED, 1, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "Device/Trigger/DigitalInput") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_IO_IN)) {
						char token[32];
						bool state;
						get_msg_str_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Source","InputToken", token);
						get_msg_boolean_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Data","LogicalState", &state);
						ONVIF_TRACE("%s token:%s state:%s", wsnt__Notify->NotificationMessage[i].Topic->__any, token, state ? "true" : "false");
						subscribe.hook(NVP_EVENT_IO_IN, state, (unsigned int)token, subscribe.hook_custom);
					}
				}
			}
			else if (strstr(wsnt__Notify->NotificationMessage[i].Topic->__any, "Device/Trigger/Relay") ) {
				if (ONVIF_C_event_find(soap->endpoint, &subscribe) == 0) {
					if (subscribe.hook && (subscribe.event_type == 0 || subscribe.event_type == NVP_EVENT_IO_OUT)) {
						char token[32];
						bool state;
						get_msg_str_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Source","RelayToken", token);
						get_msg_boolean_value(wsnt__Notify->NotificationMessage[i].Message.__any, "tt:Data","LogicalState", &state);
						ONVIF_TRACE("%s token:%s state:%s", wsnt__Notify->NotificationMessage[i].Topic->__any, token, state ? "true" : "false");
						subscribe.hook(NVP_EVENT_IO_OUT, state, (unsigned int)subscribe.peer_reference, subscribe.hook_custom);
					}
				}
			}
			

		}
		////
	}
	return SOAP_OK;
#else 
	return SOAP_FAULT;
#endif
}


#ifndef SOAP_CLIENT // kaga

SOAP_FMAC5 int SOAP_FMAC6 soap_send___tev__Notify(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct _wsnt__Notify *wsnt__Notify)
{	struct __tev__Notify soap_tmp___tev__Notify;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify";
	soap->encodingStyle = NULL;
	soap_tmp___tev__Notify.wsnt__Notify = wsnt__Notify;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___tev__Notify(soap, &soap_tmp___tev__Notify);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___tev__Notify(soap, &soap_tmp___tev__Notify, "-tev:Notify", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___tev__Notify(soap, &soap_tmp___tev__Notify, "-tev:Notify", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}
#endif

#ifndef SOAP_SERVER
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_SOAP_ENV__Fault(struct soap *soap)
{	struct SOAP_ENV__Fault soap_tmp_SOAP_ENV__Fault;
	soap_default_SOAP_ENV__Fault(soap, &soap_tmp_SOAP_ENV__Fault);
	soap->encodingStyle = NULL;
	if (!soap_get_SOAP_ENV__Fault(soap, &soap_tmp_SOAP_ENV__Fault, "SOAP-ENV:Fault", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = SOAP_ENV__Fault(soap, soap_tmp_SOAP_ENV__Fault.faultcode, soap_tmp_SOAP_ENV__Fault.faultstring, soap_tmp_SOAP_ENV__Fault.faultactor, soap_tmp_SOAP_ENV__Fault.detail, soap_tmp_SOAP_ENV__Fault.SOAP_ENV__Code, soap_tmp_SOAP_ENV__Fault.SOAP_ENV__Reason, soap_tmp_SOAP_ENV__Fault.SOAP_ENV__Node, soap_tmp_SOAP_ENV__Fault.SOAP_ENV__Role, soap_tmp_SOAP_ENV__Fault.SOAP_ENV__Detail);
	if (soap->error)
		return soap->error;
	return soap_closesock(soap);
}


SOAP_FMAC5 int SOAP_FMAC6 soap_serve___tev__Notify(struct soap *soap)
{	struct __tev__Notify soap_tmp___tev__Notify;
	soap_default___tev__Notify(soap, &soap_tmp___tev__Notify);
	soap->encodingStyle = NULL;
	if (!soap_get___tev__Notify(soap, &soap_tmp___tev__Notify, "-tev:Notify", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __tev__Notify(soap, soap_tmp___tev__Notify.wsnt__Notify);
	if (soap->error)
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___wsdd__Hello(struct soap *soap)
{	struct __wsdd__Hello soap_tmp___wsdd__Hello;
	soap_default___wsdd__Hello(soap, &soap_tmp___wsdd__Hello);
	soap->encodingStyle = NULL;
	if (!soap_get___wsdd__Hello(soap, &soap_tmp___wsdd__Hello, "-wsdd:Hello", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __wsdd__Hello(soap, soap_tmp___wsdd__Hello.wsdd__Hello);
	if (soap->error)
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___wsdd__Bye(struct soap *soap)
{	struct __wsdd__Bye soap_tmp___wsdd__Bye;
	soap_default___wsdd__Bye(soap, &soap_tmp___wsdd__Bye);
	soap->encodingStyle = NULL;
	if (!soap_get___wsdd__Bye(soap, &soap_tmp___wsdd__Bye, "-wsdd:Bye", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __wsdd__Bye(soap, soap_tmp___wsdd__Bye.wsdd__Bye);
	if (soap->error)
		return soap->error;
	return soap_closesock(soap);
}


SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap *soap)
{
	soap_peek_element(soap);
	if (!soap_match_tag(soap, soap->tag, "SOAP-ENV:Fault"))
		return soap_serve_SOAP_ENV__Fault(soap);
	if (!soap_match_tag(soap, soap->tag, "wsnt:Notify"))
		return soap_serve___tev__Notify(soap);
	if (!soap_match_tag(soap, soap->tag, "wsdd:Hello"))
		return soap_serve___wsdd__Hello(soap);
	if (!soap_match_tag(soap, soap->tag, "wsdd:Bye"))
		return soap_serve___wsdd__Bye(soap);
	if (!soap_match_tag(soap, soap->tag, "wsdd:Probe")) 
		return soap->error = SOAP_STOP;
	if (!soap_match_tag(soap, soap->tag, "wsdd:ProbeMatch")) 
		return soap->error = SOAP_STOP;
	
	return soap->error = SOAP_NO_METHOD;
}


SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	unsigned int k = soap->max_keep_alive;
#endif
	do
	{
#ifndef WITH_FASTCGI
		if (soap->max_keep_alive > 0 && !--k)
			soap->keep_alive = 0;
#endif
		if (soap_begin_serve(soap))
		{	if (soap->error >= SOAP_STOP)
				continue;
			return soap->error;
		}
		if (soap_serve_request(soap) || (soap->fserveloop && soap->fserveloop(soap)))
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}

#ifdef WITH_FASTCGI
		soap_destroy(soap);
		soap_end(soap);
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

#endif


#ifdef SOAP_SERVER
const char *g_onvif_topic[NVP_EVENT_CNT] = {
	"",
	"tns1:RuleEngine/CellMotionDetector/Motion",
	"tns1:VideoSource/MotionAlarm",
	"tns1:Device/Trigger/DigitalInput",
	"tns1:Device/Trigger/Relay",
	"tns1:VideoSource/SignalLoss",
};
const char *g_event_property[] = {"Initialized", "Changed", "Deleted"};
#define ONVIF_MD_EVENT_MSG_FMT \
	"<tt:Message UtcTime=\"%s\" PropertyOperation=\"%s\">"\
	"<tt:Source>"\
	"<tt:SimpleItem Name=\"VideoSourceConfigurationToken\" Value=\"%s\"></tt:SimpleItem>"\
	"<tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"%s\"></tt:SimpleItem>"\
	"<tt:SimpleItem Name=\"Rule\" Value=\"%s\"></tt:SimpleItem>"\
	"</tt:Source>"\
	"<tt:Data>"\
	"<tt:SimpleItem Name=\"IsMotion\" Value=\"%s\"></tt:SimpleItem>"\
	"</tt:Data>"\
	"</tt:Message>"

#define ONVIF_MD_EX_EVENT_MSG_FMT \
	"<tt:Message UtcTime=\"%s\" PropertyOperation=\"%s\">"\
	"<tt:Source>"\
	"<tt:SimpleItem Name=\"VideoSourceToken\" Value=\"%s\"></tt:SimpleItem>"\
	"</tt:Source>"\
	"<tt:Data>"\
	"<tt:SimpleItem Name=\"State\" Value=\"%s\"></tt:SimpleItem>"\
	"</tt:Data>"\
	"</tt:Message>"
	
const char  * g_onvif_event_msg[NVP_EVENT_CNT] = {"", 
	ONVIF_MD_EVENT_MSG_FMT, ONVIF_MD_EX_EVENT_MSG_FMT, };

int ONVIF_send_notify(int event, int state, char *local_reference, char *peer_reference)
{
	extern void get_current_time(char *sztime);
	int ret = -1;
	char sztime[40];
	struct soap *soap = NULL;
	struct _wsnt__Notify req_uri;
	struct wsnt__NotificationMessageHolderType msg[1];
	struct SOAP_ENV__Header header;
	lpNVP_PROFILE_CHN profile;
	int chn, id;
	
	// FIM me
	chn = 0;
	id  = 0;
	profile =  &g_OnvifServerCxt->env.profiles.profile[chn];

	if((soap = soap_new())	== NULL){
		printf("soap new failed!\n");
		return -1;
	}
	soap->send_timeout = 5;
	soap->recv_timeout = 5;
	soap->connect_timeout = 5;
	soap_set_namespaces(soap, namespaces);
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;

	memset(&req_uri,0,sizeof(req_uri));
	memset(msg, 0, sizeof(msg));
	memset(&header, 0, sizeof(header));
	req_uri.__sizeNotificationMessage = 1;
	msg[0].Topic = soap_malloc(soap, sizeof(struct wsnt__TopicExpressionType));
	msg[0].Topic->Dialect = "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
	msg[0].Topic->__any = (char *)soap_malloc(soap, 128);
	strcpy(msg[0].Topic->__any, g_onvif_topic[event]);
	msg[0].Topic->__mixed = NULL;
	msg[0].Topic->__anyAttribute = NULL;
	get_current_time(sztime);
	msg[0].Message.__any = (char *)soap_malloc(soap, 2000);
	if (event == NVP_EVENT_MD) {
		SNPRINTF(msg[0].Message.__any, 2000, ONVIF_MD_EVENT_MSG_FMT,
			sztime, g_event_property[state],
			profile->vin[id].token, profile->van.token, profile->md.rule_name,
			(state >= ONVIF_EVENT_STATE_DELETE) ? "false" :  "true");
	} else if (event == NVP_EVENT_MD_EX) {
		SNPRINTF(msg[0].Message.__any, 2000, ONVIF_MD_EX_EVENT_MSG_FMT,
			sztime, g_event_property[state], profile->v_source.token, (state >= ONVIF_EVENT_STATE_DELETE) ? "false" :  "true");
	} else {
		goto ERR_EXIT;
	}
	msg[0].SubscriptionReference = NULL;
	msg[0].ProducerReference = NULL;
	req_uri.NotificationMessage = msg;
	req_uri.__size = 0;
	req_uri.__any = 0;
	header.wsa5__Action = "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify";
	//header.wsa__Action = "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify";
	soap->header = &header;
	
	soap_send___tev__Notify( soap, peer_reference , NULL, &req_uri);
	if(soap->error)
	{
		ONVIF_INFO("soap error:%d,code:%s\n\tsub:%s\n\treason:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap), *soap_faultstring(soap) );
		if (soap->error == 28 && (*soap_faultstring(soap) )) {
			if (strcmp(*soap_faultstring(soap) , "Connection refused") == 0) {
				ONVIF_INFO("send notify failed, because peer port not open!");
				//ONVIF_event_unsubscribe(local_reference);
				ret = NVP_RET_CONNECT_REFUSED;
			}
		}
		goto ERR_EXIT;
	}else{
		ONVIF_INFO("notify event(%d/%d) to %s ok!", event, state, peer_reference);
	}

	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return 0;

ERR_EXIT:
	
	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return ret;		
}


int ONVIF_send_hello()
{
	int i;
	struct soap *soap = NULL;
	struct wsdd__HelloType req_uri;
	struct SOAP_ENV__Header header;
	char sz_scopes[1024* 5];
	char _hostip[128];
	const char *uuid=NULL;
	struct ip_mreq mcast;

	
	NVP_env_load(&g_OnvifServerCxt->env, OM_NET, 0);
	_ip_2string(g_OnvifServerCxt->env.ether.ip, _hostip);
	
#if 1

	soap = soap_new2(SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH, SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH);
	if (soap == NULL) {
		ONVIF_INFO("soap_new failed!");
		return -1;
	}
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;
	if(!soap_valid_socket(soap_bind(soap, _hostip, 0, 4)))
	{ 
		soap_print_fault(soap, stderr);
		soap_free(soap);
		return -1;
	}
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = inet_addr(_hostip) ;
	if(setsockopt(soap->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
		soap_free(soap);
		return -1;
	}

	soap->peer.sin_family = AF_INET;
	soap->peer.sin_addr.s_addr = inet_addr("239.255.255.250") ;
	soap->peer.sin_port = htons(3702);
	soap->peerlen = sizeof(struct sockaddr_in);
	soap->keep_alive = 1; //not forget
#endif
	
	soap->send_timeout = 3;
	soap->recv_timeout = 3;
	soap->connect_timeout = 3;
	soap_set_namespaces(soap, discovery_namespaces);

	memset(&req_uri,0,sizeof(req_uri));
	memset(&header, 0, sizeof(header));
	uuid = soap_wsa_rand_uuid(soap);
	soap_default_SOAP_ENV__Header(soap, &header);
	header.wsa__MessageID = (char *)uuid;
	header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
	header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello";
	soap->header = &header;
	req_uri.Types = g_OnvifServerCxt->dev_type;
	
	memset(sz_scopes, 0, sizeof(sz_scopes));
	for ( i = 0; i < g_OnvifServerCxt->scope_nr; i++) {
		strcat(sz_scopes, g_OnvifServerCxt->scope_list[i]);
		if ( (i + 1) != g_OnvifServerCxt->scope_nr)
			strcat(sz_scopes, " ");
	}
	req_uri.Scopes = (struct wsdd__ScopesType *)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
	soap_default_wsdd__ScopesType(soap, req_uri.Scopes);
	req_uri.Scopes->__item = (char *)soap_malloc(soap, 2024);	
	strcpy(req_uri.Scopes->__item, sz_scopes);
	req_uri.wsa__EndpointReference.Address =(char *)uuid;
	req_uri.XAddrs = (char *)soap_malloc(soap, 128);
	sprintf(req_uri.XAddrs, "http://%s:%d/onvif/device_service", _hostip, g_OnvifServerCxt->env.ether.http_port);
	req_uri.MetadataVersion = 10;
	
	soap_send___wsdd__Hello( soap, "soap.udp://239.255.255.250:3702/" , NULL, &req_uri);
	if(soap->error)
	{
		ONVIF_INFO("soap error:%d,code:%s\n\tsub:%s\n\treason:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap), *soap_faultstring(soap) );
		goto ERR_EXIT;
	}else{
		ONVIF_INFO("send hello(%s) ok!", req_uri.XAddrs);
		Sleep_c(1);
	}

	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return 0;

ERR_EXIT:
	
	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return -1;		
}

int ONVIF_send_bye()
{
	int i;
	struct soap *soap = NULL;
	struct wsdd__ByeType req_uri;
	struct SOAP_ENV__Header header;
	char sz_scopes[1024* 5];
	char _hostip[128];
	const char *uuid=NULL;
	struct ip_mreq mcast;

	
	NVP_env_load(&g_OnvifServerCxt->env, OM_NET, 0);
	_ip_2string(g_OnvifServerCxt->env.ether.ip, _hostip);
	
#if 1

	soap = soap_new2(SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH, SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH);
	if (soap == NULL) {
		ONVIF_INFO("soap_new failed!");
		return -1;
	}
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;
	if(!soap_valid_socket(soap_bind(soap, _hostip, 0, 4)))
	{ 
		soap_print_fault(soap, stderr);
		soap_free(soap);
		return -1;
	}
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = inet_addr(_hostip) ;
	if(setsockopt(soap->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
		soap_free(soap);
		return -1;
	}

	soap->peer.sin_family = AF_INET;
	soap->peer.sin_addr.s_addr = inet_addr("239.255.255.250") ;
	soap->peer.sin_port = htons(3702);
	soap->peerlen = sizeof(struct sockaddr_in);
	soap->keep_alive = 1; //not forget
#endif

	soap->send_timeout = 3;
	soap->recv_timeout = 3;
	soap->connect_timeout = 3;
	soap_set_namespaces(soap, discovery_namespaces);

	memset(&req_uri,0,sizeof(req_uri));
	memset(&header, 0, sizeof(header));
	uuid = soap_wsa_rand_uuid(soap);
	soap_default_SOAP_ENV__Header(soap, &header);
	header.wsa__MessageID = (char *)uuid;
	header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
	header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Bye";
	soap->header = &header;
	req_uri.Types = g_OnvifServerCxt->dev_type;
	
	memset(sz_scopes, 0, sizeof(sz_scopes));
	for ( i = 0; i < g_OnvifServerCxt->scope_nr; i++) {
		strcat(sz_scopes, g_OnvifServerCxt->scope_list[i]);
		if ( (i + 1) != g_OnvifServerCxt->scope_nr)
			strcat(sz_scopes, " ");
	}
	req_uri.Scopes = (struct wsdd__ScopesType *)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
	soap_default_wsdd__ScopesType(soap, req_uri.Scopes);
	req_uri.Scopes->__item = (char *)soap_malloc(soap, 2024);	
	strcpy(req_uri.Scopes->__item, sz_scopes);
	req_uri.wsa__EndpointReference.Address =(char *)uuid;
	req_uri.XAddrs = (char *)soap_malloc(soap, 128);
	sprintf(req_uri.XAddrs, "http://%s:%d/onvif/device_service", _hostip, g_OnvifServerCxt->env.ether.http_port);
	req_uri.MetadataVersion = NULL;
	
	soap_send___wsdd__Bye( soap, "soap.udp://239.255.255.250:3702/" , NULL, &req_uri);
	if(soap->error)
	{
		ONVIF_INFO("soap error:%d,code:%s\n\tsub:%s\n\treason:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap), *soap_faultstring(soap) );
		goto ERR_EXIT;
	}else{
		ONVIF_INFO("send bye(%s) ok!", req_uri.XAddrs);
	}

	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return 0;

ERR_EXIT:
	
	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return -1;	
}



#endif

#ifdef SOAP_CLIENT
int ONVIF_MULTICAST_get_devinfo(lpNVP_DEV_INFO info)
{
	struct soap *soap = NULL;
	struct _tds__GetDeviceInformation req_uri;
	struct _tds__GetDeviceInformationResponse resp_uri;
	struct SOAP_ENV__Header header;
	const char *uuid=NULL;
	
	if((soap = soap_new())==NULL){
	ONVIF_INFO("soap_new failed!");
	return -1;
	}
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;
	soap->connect_flags = SO_BROADCAST;
	
	soap->send_timeout = 3;
	soap->recv_timeout = 3;
	soap->connect_timeout = 3;
	soap_set_namespaces(soap, discovery_namespaces);

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	memset(&header, 0, sizeof(header));
	uuid = soap_wsa_rand_uuid(soap);
	soap_default_SOAP_ENV__Header(soap, &header);
	header.wsa__MessageID = (char *)uuid;
	//header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
	header.wsa__To = (char *)uuid;
	header.wsa__Action = "http://www.onvif.org/ver10/device/wsdl/GetDeviceInformation";
	soap->header = &header;	
	
	soap_call___tds__GetDeviceInformation( soap, "soap.udp://239.255.255.250:3702/" , NULL, &req_uri, &resp_uri);
	if(soap->error)
	{
		ONVIF_INFO("soap error:%d,code:%s\n\tsub:%s\n\treason:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap), *soap_faultstring(soap) );
		goto ERR_EXIT;
	}else{
		ONVIF_INFO("send multicast get devinfo ok!");
		ONVIF_INFO("Brand: %s model:%s FW:%s SN:%s HWID:%s", resp_uri.Manufacturer, resp_uri.Model, resp_uri.FirmwareVersion, resp_uri.SerialNumber,
			resp_uri.HardwareId);
		if(info) {
			strcpy(info->manufacturer, resp_uri.Manufacturer);
			strcpy(info->model, resp_uri.Model);
			strcpy(info->firmware, resp_uri.FirmwareVersion);
			strcpy(info->sn, resp_uri.SerialNumber);
			strcpy(info->hwid, resp_uri.HardwareId);
		}
	}

	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return 0;

ERR_EXIT:
	
	soap_destroy(soap);
	soap_end(soap);
	soap_free(soap);
	return -1;	
}
#endif


THREAD_RETURN onvif_search_server_proc(void * arg)
{
	bool *trigger = (bool *)arg;
	struct soap soap;
	struct ip_mreq mcast;
	fd_set rset;
	struct timeval timeout;
	int ret;

	soap_init2(&soap, SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH, SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH);
	soap.bind_flags |= SO_REUSEADDR;
	//soap_set_mode(&soap, SOAP_C_UTFSTRING);
	if(!soap_valid_socket(soap_bind(&soap, NULL, 3702, 100)))
	{ 
		soap_print_fault(&soap, stderr);
		ONVIF_INFO("soap udp bind failed");
		exit(1);
	}
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = htonl(INADDR_ANY);
	if(setsockopt(soap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
		return (THREAD_RETURN)0;
	}

	ONVIF_INFO("onvif serch daemon start, pid(0x%lx)...", currentThreadId_c());

	soap.recv_timeout = 3;
	soap.send_timeout = 3;
	
	soap_set_namespaces(&soap, discovery_namespaces); 

	while((*trigger) == true) {
		FD_ZERO(&rset);
		FD_SET(soap.master, &rset);

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		ret  = select(soap.master + 1, &rset, NULL, NULL, &timeout);
		if (ret < 0) {
			//ONVIF_TRACE("select error.");
		}else if (ret == 0) {
			//ONVIF_TRACE("select timeout, trigger: %d.", (int)onvif->search_trigger);
		} else {
			//ONVIF_TRACE("select available.");
			if (FD_ISSET(soap.master, &rset)) {
				if((ret = soap_serve(&soap)) != SOAP_OK){
					ONVIF_INFO("onvif search serve failed, %d", ret);
					soap_print_fault(&soap, stderr);
				}
				//soap_destroy(&soap);
				soap_end(&soap);
			}
		}
	}

	soap_done(&soap);

	ONVIF_INFO("onvif search daemon stop!");
	
	exitThread_c(((THREAD_RETURN)NULL));
	return (THREAD_RETURN)NULL;
}

void ONVIF_search_daemon_start(fON_WSDD_EVENT hook, void *customCtx)
{
	g_search_hook = hook;
	g_searchCustomCtx = customCtx;
	if (g_search_pid == 0) {
		g_search_trigger = true;
		initThread_c(&g_search_pid,onvif_search_server_proc,&g_search_trigger);
	}
}

void ONVIF_search_daemon_set_hook(fON_WSDD_EVENT hook, void *customCtx)
{
	g_search_hook = hook;
	g_searchCustomCtx = customCtx;
}

void ONVIF_search_daemon_stop()
{
	if (g_search_pid) {
		g_search_trigger = false;
		joinThread_c(g_search_pid);
		g_search_pid = 0;
	}
}


