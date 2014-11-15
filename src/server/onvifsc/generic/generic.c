#include "cross.h"
#include "generic.h"


#ifndef WIN32
ssize_t get_file_size(const char* file_nane)
{
	struct stat file_stat;

	if(stat(file_nane, &file_stat) < 0){
		return -1;
	}
	return (ssize_t)(file_stat.st_size);
}
#endif

int check_ipv4_addr(const char *pstrip)
{
	const	char	*ptr;
	int	count = 0;

	if (pstrip == NULL || *pstrip == 0)
		return(-1);

	ptr = pstrip;
	if(*ptr == '.')		/* the first char should not be '.' */
		return(-1);
	while(*ptr) {
		if (*ptr == '.') {
			if (!isdigit((int)*(ptr + 1)))
				return(-1);
			count++;
		} else if (!isdigit((int)*ptr))
			return(-1);
		ptr++;
	}
	if(*(ptr - 1) == '.')	/* the last char should not be '.' */
		return(-1);
	if(count != 3)		/* 192.168.0.1 has the number '.' is 4 */
		return(-1);

	return(0);
}

int http_parse_url(unsigned char *_u8ip, unsigned short *_port, char *_uri, char *_default) 
{ 
	int ret;
	char *ppp= NULL; 
	char ipport[32];
	int tmp[4]; 
	int tmp_port;
	char _uri_temp[128];
	if (_u8ip) {
		_u8ip[0] = 0; 
		_u8ip[1] = 0; 
		_u8ip[2] = 0; 
		_u8ip[3] = 0; 
	}
	_uri_temp[0] = 0; 
	if (_port) *_port = 0;
	if (strncmp(_default, "http://", strlen("http://")) !=0 ) {
		return -1;
	}
	ppp = _default + strlen("http://");
	ret  = sscanf(ppp, "%[^/]%s", ipport, _uri_temp);
	if(ret == 1 || ret == 2){
		if(strstr(ipport,":")!=NULL){
			if(sscanf(ipport, "%d.%d.%d.%d:%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3],&tmp_port) == 5) 
			{ 
				if (_u8ip) {
					_u8ip[0] = tmp[0]; 
					_u8ip[1] = tmp[1]; 
					_u8ip[2] = tmp[2]; 
					_u8ip[3] = tmp[3]; 
				}
				if (_port) *_port = tmp_port;
			}else{
				printf("ERR: parse ip address failed1 : %s!\n", ipport);
				return -1;
			}
		}else{
			if(sscanf(ipport, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) == 4) 
			{ 
				if (_u8ip) {
					_u8ip[0] = tmp[0]; 
					_u8ip[1] = tmp[1]; 
					_u8ip[2] = tmp[2]; 
					_u8ip[3] = tmp[3]; 
				}
				if (_port) *_port = 80;
			}else{
				printf("ERR: parse ip address failed2: %s!\n", ipport);
				return -1;
			}
		}
	}else{
		printf("ERR: parse ip address failed4 !\n", ppp);
		return -1;
	}
	if (_uri) {
		strcpy(_uri, _uri_temp);
	}
	return 0;
}

char  *_ip_2string(unsigned char *ip, char *saveptr)
{
	static char ___temp_ip[64];
	uint8_t *ptr = ip;
	if (saveptr ) {
		sprintf(saveptr, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
		return saveptr;
	}else {
		sprintf(___temp_ip, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
		return ___temp_ip;
	}
}

char  *_mac_2string(unsigned char *mac, char *saveptr)
{
	static char ___temp_mac[64];
	uint8_t *ptr = mac;
	if (saveptr ) {
		sprintf(saveptr, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
		return saveptr;
	}else {
		sprintf(___temp_mac, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
		return ___temp_mac;
	}
}

int ipstr2uint8(unsigned char *_u8ip,  char *_default)
{
	int tmp[4]; 
	_u8ip[0] = 0; 
	_u8ip[1] = 0; 
	_u8ip[2] = 0; 
	_u8ip[3] = 0; 
	if(sscanf(_default, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) == 4) 
	{ 
		_u8ip[0] = tmp[0]; 
		_u8ip[1] = tmp[1]; 
		_u8ip[2] = tmp[2]; 
		_u8ip[3] = tmp[3]; 
	}else{
		printf("ERR: parse ip address failed3 : %s!\n", _default);
		return -1;
	}
	return 0;
}

int macstr2uint8(unsigned char *_u8mac, char *_default)
{
	int tmp[6]; 
	int ret;
	_u8mac[0] = 0; 
	_u8mac[1] = 0; 
	_u8mac[2] = 0; 
	_u8mac[3] = 0; 
	_u8mac[4] = 0; 
	_u8mac[5] = 0; 
	STR_TO_UPPER(_default);
	if (strstr(_default , ":")) 
		ret = sscanf(_default, "%X:%X:%X:%X:%X:%X", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]) ;
	else
		ret = sscanf(_default, "%X-%X-%X-%X-%X-%X", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]) ;
	if(ret == 6) 
	{ 
		_u8mac[0] = tmp[0]; 
		_u8mac[1] = tmp[1]; 
		_u8mac[2] = tmp[2]; 
		_u8mac[3] = tmp[3]; 
		_u8mac[4] = tmp[4]; 
		_u8mac[5] = tmp[5]; 
	}else{
		printf("ERR: parse mac address(%s) failed!\n", _default);
		return -1;
	}
	return 0;
}



