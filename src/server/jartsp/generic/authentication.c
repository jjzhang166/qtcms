#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "authentication.h"
#include "_base64.h"
#include "_md5.h"

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif


#define AUTH_DEFAULT_USER	"admin"
#define AUTH_DEFAULT_PWD	"12345"

static void md5_hash( char **v, int count, char *hash )
{
	struct MD5Context md5;
	int i;
	unsigned char bin[16];

	MD5Init( &md5 );
	for( i = 0; i < count; ++i )
	{
		if( i > 0 ) MD5Update( &md5, (unsigned char const *)":", 1 );
		MD5Update( &md5, (unsigned char const *)v[i], strlen( v[i] ) );
	}
	MD5Final( bin, &md5 );
	for( i = 0; i < 16; ++i ) sprintf( hash + (i<<1), "%02x", bin[i] );
	hash[32] = 0;
}


static void digest_challenge(char *out)
{
	srand(time(NULL));
	sprintf(out,"%ld%ld%u",time(NULL)%15731,time(NULL),rand());
}

static int auth_user_validate(char *usr,char *pwd)
{
	if((strcmp(usr,AUTH_DEFAULT_USER)==0) &&
		(strcmp(pwd,AUTH_DEFAULT_PWD)==0)){
		return TRUE;
	}else{
		return FALSE;
	}
}


static int basic_setup(Authentication_t *auth)
{
	char tmp[128];
	int ret;
	sprintf(tmp,"%s:%s",auth->user,auth->pwd);
	if((ret=BASE64_encode(tmp,strlen(tmp),auth->responce,sizeof(auth->responce)))< 0)
		return AUTH_RET_FAIL;

	//printf("AUTH_DBG: Basic setup: user:%s pwd:%s %s\n",auth->user,auth->pwd,auth->responce);
	return AUTH_RET_OK;
}


static int basic_validate(Authentication_t *auth)
{
	char dst[128];
	int ret;
	
	ret = BASE64_decode(auth->responce,strlen(auth->responce),dst,sizeof(dst));
	if(ret< 0)
		return FALSE;
	dst[ret]=0;
	ret=sscanf(dst,"%[^:]:%s",auth->user,auth->pwd);
	if(ret != 2)
		return FALSE;
	//printf("AUTH_DBG: Basic Validate Pass: user:%s pwd:%s (%s)\n",auth->user,auth->pwd,auth->responce);

	return auth_user_validate(auth->user,auth->pwd);
}

static int digest_setup(Authentication_t *auth)
{
	char *elem[3], ha1[33], ha2[33];
	//printf("username:%s.\r\n",auth->user);
	//printf("nonce:%s.\r\n",auth->nonce);
	//printf("realm:%s.\r\n",auth->realm);
	//printf("uri:%s.\r\n",auth->url);
	//printf("pwd:%s.\r\n",auth->pwd);
	//printf("method:%s.\r\n",auth->method);
	elem[0] = auth->user;
	elem[1] = auth->realm;
	elem[2] = auth->pwd;
	md5_hash( elem, 3, ha1 );
	elem[0] = auth->method;
	elem[1] = auth->url;
	md5_hash( elem, 2, ha2 );
	elem[0] = ha1;
	elem[1] = auth->nonce;
	elem[2] = ha2;
	md5_hash( elem, 3, auth->responce);
	
	//printf("AUTH_DBG: Digest setup: user:%s pwd:%s %s\n",auth->user,auth->pwd,auth->responce);
	return AUTH_RET_OK;
}

static int digest_validate(Authentication_t *auth)
{
	char *elem[3], ha1[33], ha2[33];
	char dst[128];
	printf("nonce:%s.\r\n",auth->nonce);
	printf("realm:%s.\r\n",auth->realm);
	printf("uri:%s.\r\n",auth->url);
	printf("method:%s.\r\n",auth->method);
	// MD5(username:realm:password) ,   USERINFODIGEST
	elem[0] = AUTH_DEFAULT_USER;
	elem[1] = auth->realm;
	elem[2] = AUTH_DEFAULT_PWD;
	md5_hash( elem, 3, ha1 );
	// MD5(method:url) ,        MURLDIGEST
	elem[0] = auth->method;
	elem[1] = auth->url;
	md5_hash( elem, 2, ha2 );
	// MD5(USERINFODIGEST:nonce:MURLDIGEST)
	elem[0] = ha1;
	elem[1] = auth->nonce;
	elem[2] = ha2;
	md5_hash( elem, 3, dst);

	//printf("AUTH_DBG:Digest Validate: (%s) expected:%s\n",auth->responce,dst);

	if(strcmp(dst,auth->responce) != 0)
		return FALSE;
	printf("AUTH_INFO:Digest Validate Pass\n");
	return TRUE;
}

int 
HTTP_AUTH_client_init(
	struct _authentication **auth,
	char *www_authenticate/*the value in the domain of <WWW-Authenticate>,not contain CRLF*/)
{
	char *ptr = NULL;
	//
	if((*auth) != NULL) return AUTH_RET_OK;
	//
	*auth=(Authentication_t *)malloc(sizeof(Authentication_t));
	if((*auth) == NULL){
		printf("HTTP-AUTH ERROR: malloc for auth failed");
		assert(0);
	}
	memset(*auth,0,sizeof(Authentication_t));

	if(strncmp(www_authenticate,"Basic",strlen("Basic")) == 0){
		(*auth)->type = HTTP_AUTH_BASIC;
		ptr=strstr(www_authenticate,"realm");
		if(ptr) sscanf(ptr,"realm=\"%[^\"]",(*auth)->realm);
	}else if(strncmp(www_authenticate,"Digest",strlen("Digest")) == 0){
		(*auth)->type = HTTP_AUTH_DIGEST;
		ptr=strstr(www_authenticate,"realm");
		if(ptr) sscanf(ptr,"realm=\"%[^\"]",(*auth)->realm);
		ptr=strstr(www_authenticate,"nonce");
		if(ptr) sscanf(ptr,"nonce=\"%[^\"]",(*auth)->nonce);
	}else{
		printf("HTTP-AUTH ERROR: invalid WWW-Authenticate format\n");
		free(*auth);
		*auth = NULL;
		return AUTH_RET_FAIL;
	}

	return AUTH_RET_OK;
}

int 
HTTP_AUTH_setup(
	struct _authentication *auth,
	char *username,char *password,
	char *url,char *method, /* if use digest ,must given these two parameters,else ignore these*/
	char *out,int out_size)
{
	int ret;
	//
	if(auth == NULL || out == NULL || username==NULL || password == NULL){
		printf("HTTP-AUTH ERROR: invalid parameter!\n");
		return AUTH_RET_FAIL;
	}
	if(auth->type == HTTP_AUTH_DIGEST){
		if(url== NULL || method == NULL){
			printf("HTTP-AUTH ERROR: invalid parameter!\n");
			return AUTH_RET_FAIL;

		}
	}
	strcpy(auth->user,username);
	strcpy(auth->pwd,password);
	if(url) strcpy(auth->url,url);
	if(method) strcpy(auth->method,method);

	if(auth->type == HTTP_AUTH_BASIC){
		if(basic_setup(auth) == AUTH_RET_FAIL) return AUTH_RET_FAIL;
		ret=sprintf(out,"Basic %s",auth->responce);
		if(ret > out_size){
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}else{
		if(digest_setup(auth) == AUTH_RET_FAIL) return AUTH_RET_FAIL;
		ret=sprintf(out,"Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"",
			auth->user,auth->realm,auth->nonce,auth->url,auth->responce);
		if(ret > out_size){
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}
	return AUTH_RET_OK;
}
// use for server
int HTTP_AUTH_server_init(struct _authentication **auth,int type)
{
	if(*auth != NULL) return AUTH_RET_OK;
	//
	*auth=(Authentication_t *)malloc(sizeof(Authentication_t));
	if(*auth == NULL){
		printf("AUTH_ERR:malloc for auth failed");
		return AUTH_RET_FAIL;
	}
	memset(*auth,0,sizeof(Authentication_t));
	strcpy((*auth)->realm,HTTP_AUTH_REALM);
	(*auth)->type = type;
	return AUTH_RET_OK;
}

int HTTP_AUTH_chanllenge(struct _authentication *auth,char *out,int out_size)
{
	int ret;
	if(auth == NULL || out == NULL){
		printf("HTTP-AUTH ERROR: invalid parameter!\n");
		return AUTH_RET_FAIL;
	}
	if(auth->type == HTTP_AUTH_BASIC){
		ret = sprintf(out,"Basic realm=\"%s\"",auth->realm);
		if(ret > out_size){
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}else{
		digest_challenge(auth->nonce);
		ret = sprintf(out,"Digest realm=\"%s\", nonce=\"%s\"",auth->realm,auth->nonce);
		if(ret > out_size){
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}
	return AUTH_RET_OK;
}

int HTTP_AUTH_validate(struct _authentication *auth,
	char *authorization,/* the value in the domain of <Authorization>,not contain CRLF */
	char *method)
{
	char *ptr = NULL;
	char nonce[128];
	char realm[128];
	char algorithm[32];
	int ret;
	if(auth == NULL || authorization == NULL){
		printf("HTTP-AUTH ERROR: invalid parameter!\n");
		return FALSE;
	}
	
	if(strncmp(authorization,"Basic",strlen("Basic")) == 0){
		if(auth->type != HTTP_AUTH_BASIC){
			printf("HTTP-AUTH ERROR: invalid authorization type,expect Basic!\n");
			return FALSE;
		}
		ptr = authorization + strlen("Basic");
		ptr++; // empty space
		strcpy(auth->responce,ptr);
		// validate
		ret = basic_validate(auth);
	}else if(strncmp(authorization,"Digest",strlen("Digest")) == 0){
		printf("auth:%s.\n",authorization);
		if(auth->type != HTTP_AUTH_DIGEST){
			printf("HTTP-AUTH ERROR: invalid authorization type,expect Digest!\n");
			return FALSE;
		}
		if(method == NULL){
			printf("HTTP-AUTH ERROR: not given method!\n");
			return FALSE;
		}
		strcpy(auth->method,method);
		ptr=strstr(authorization,"realm");
		if(ptr) sscanf(ptr,"realm=\"%[^\"]",realm);
		ptr=strstr(authorization,"nonce");
		if(ptr) sscanf(ptr,"nonce=\"%[^\"]",nonce);
		if((strcmp(auth->realm,realm)!=0) || (strcmp(auth->nonce,nonce)!=0)){
			printf("HTTP-AUTH ERROR: invalid authorization value!\n");
			return FALSE;
		}
		ptr=strstr(authorization,"uri");
		if(ptr) sscanf(ptr,"uri=\"%[^\"]",auth->url);
		else{
			printf("HTTP-AUTH ERROR: invalid authorization value!\n");
			return FALSE;
		}
		ptr=strstr(authorization,"username");
		if(ptr) sscanf(ptr,"username=\"%[^\"]",auth->user);
		else{
			printf("HTTP-AUTH ERROR: invalid authorization value!\n");
			return FALSE;
		}
		ptr=strstr(authorization,"response");
		if(ptr) sscanf(ptr,"response=\"%[^\"]",auth->responce);
		else{			
			printf("HTTP-AUTH ERROR: invalid authorization value!\n");
			return FALSE;
		}
		ptr=strstr(authorization,"algorithm");
		if(ptr){
			sscanf(ptr,"algorithm=\"%[^\"]",algorithm);
			if(strcmp(algorithm,"MD5")!=0){
				printf("HTTP-AUTH ERROR: unsupport algorithm,expected MD5!\n");
				return FALSE;
			}
		}
		//validate
		ret = digest_validate(auth);
	}
	
	return ret;
}

int HTTP_AUTH_destroy(struct _authentication *auth)
{
	if(auth){
		free(auth);
	}
	return AUTH_RET_OK;
}

