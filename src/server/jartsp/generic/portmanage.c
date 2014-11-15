#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "portmanage.h"
#include "gnu_win.h"

#define PM_APPLY_BY_INC

typedef struct _port_entry
{	
	struct _port_entry_data
	{
		union
		{
			unsigned int port;
			unsigned int entries;
		};
	}data;
	struct _port_entry *next;
}PortEntry_t,PortLink_t;


typedef struct _port_manage
{
	unsigned int m_min;
	unsigned int m_max;
	PortLink_t m_link;
	//
	unsigned int m_start;
	unsigned int m_cur;
	PortEntry_t *m_curEntry;
	Lock_t m_lock;
}PortManage_t;

static PortManage_t *gIpPortList=NULL;

static void port_manage_init_default_ports()
{
	PORT_MANAGE_add_port(0);//reserved
	PORT_MANAGE_add_port(1);//tcpmux
	PORT_MANAGE_add_port(7);//echo
	PORT_MANAGE_add_port(19);//character generator
	PORT_MANAGE_add_port(21);//ftp
	PORT_MANAGE_add_port(22);//ssh
	PORT_MANAGE_add_port(23);//telnet
	PORT_MANAGE_add_port(25);//smtp
	PORT_MANAGE_add_port(31);//msg authentication
	PORT_MANAGE_add_port(42);//wins replication
	PORT_MANAGE_add_port(53);//DNS
	PORT_MANAGE_add_port(67);//bootstrap protocol server
	PORT_MANAGE_add_port(69);//trival file transfer
	PORT_MANAGE_add_port(80);//http
	PORT_MANAGE_add_port(99);//metagram relay
	PORT_MANAGE_add_port(102);//MTA-x 400
	
	PORT_MANAGE_add_port(554);// RTSP
	PORT_MANAGE_add_port(1900);//SSDP
	PORT_MANAGE_add_port(1935);//RTMP
	PORT_MANAGE_add_port(3702);//ONVIF
	
}

static void PORT_MANAGE_dump()
{
	PortEntry_t *p;
	assert(gIpPortList != NULL);
	printf("### PORT MANGE min:%u max:%u cur:%u start:%u count:%d ######\n",
		gIpPortList->m_min,gIpPortList->m_max,
		gIpPortList->m_cur, gIpPortList->m_start,
		gIpPortList->m_link.data.entries);
	p = gIpPortList->m_link.next;
#ifdef PM_TEST
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		assert( p != NULL);
		printf("\t@%05d %u\n",i+1,p->data.port);
		p = p->next;
	}
#endif
}

int PORT_MANAGE_init(unsigned int min,unsigned int max)
{
	if(gIpPortList != NULL) return 0;
	gIpPortList = (PortManage_t *)calloc(1,sizeof(PortManage_t));
	assert(gIpPortList != NULL);
	assert((max - min) > 1);
	gIpPortList->m_min = min;
	gIpPortList->m_start = min + (time(NULL) % (max-min -1));
	if ((gIpPortList->m_start % 2) == 1) {
		gIpPortList->m_start++;
	}
	gIpPortList->m_cur = gIpPortList->m_start;
	gIpPortList->m_max = max;
	gIpPortList->m_link.data.entries = 0;
	gIpPortList->m_link.next = NULL;
	gIpPortList->m_curEntry = &gIpPortList->m_link;
	LOCK_init(&gIpPortList->m_lock);

	//port_manage_init_default_ports();
	return 0;
}

int PORT_MANAGE_destroy()
{
	unsigned int i;
	PortEntry_t *p,*tmp;
	if(gIpPortList == NULL) return 0;
	p = gIpPortList->m_link.next;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		assert( p != NULL);
		tmp = p->next;
		free(p);
		p = tmp;
	}
	LOCK_destroy(&gIpPortList->m_lock);
	gIpPortList->m_min = 0;
	gIpPortList->m_max = 0;
	free(gIpPortList);
	gIpPortList = NULL;
	return 0;
}


int PORT_MANAGE_add_port(unsigned int port)
{
	unsigned int i;
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p,*prev;
	
	if(!(port >= gIpPortList->m_min && port <= gIpPortList->m_max)){
		return 0;
	}
	assert(gIpPortList != NULL);

	LOCK_lock(&gIpPortList->m_lock);
	
	p = &gIpPortList->m_link;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		prev = p;
		p = p->next;
		assert( p != NULL);
		if(p->data.port == port){
			printf("this port is exist!\n");
			LOCK_unlock(&gIpPortList->m_lock);
			return 0;
		}else if(p->data.port > port){
			flag = 1;
			break;
		}
	}

	assert( p != NULL);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = port;
	if(flag == 1){
		s->next = p;
		prev->next = s;
	}else{
		s->next = NULL;
		p->next = s;
	}
	gIpPortList->m_link.data.entries++;
	PORT_MANAGE_dump();
	
	LOCK_unlock(&gIpPortList->m_lock);
	
	return 0;
}

int PORT_MANAGE_apply1_port(unsigned int * const port)
{
	unsigned int i;
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port != NULL);

	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_min;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else if(gIpPortList->m_link.data.entries  == 1){
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 0){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
		}else{
			ins_port = gIpPortList->m_link.next->data.port + 1;
			ins_p = gIpPortList->m_link.next;
		}
		flag = 1;
	}else{
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 0){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
			flag = 1;
		}else{
			p = gIpPortList->m_link.next;
			for( i = 0; i < (gIpPortList->m_link.data.entries -1); i++){
				assert( p != NULL || p->next != NULL);
				assert( p->next->data.port > p->data.port);
				if((p->next->data.port - p->data.port) > 1){
					ins_port = p->data.port + 1;
					ins_p = p;
					flag = 1;
					break;
				}
				p = p->next;
			}
		}
	}
	// 
	// new entry
	if(flag == 0){
		ins_p = p;
		ins_port = ins_p->data.port+1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	//
	*port = ins_port;
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);

	return 0;
}

// apply two continuous ports, the first port is port1, the second port would be {port1 + 1}
int PORT_MANAGE_apply2_port(unsigned int * const port1)
{
	unsigned int i;
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port1 != NULL);
	
	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_min;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else if(gIpPortList->m_link.data.entries  == 1){
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 1){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
		}else{
			ins_port = gIpPortList->m_link.next->data.port;
			ins_p = gIpPortList->m_link.next;
		}
		flag = 1;
	}else{
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 1){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
			flag = 1;
		}else{
			p = gIpPortList->m_link.next;
			for( i = 0; i < (gIpPortList->m_link.data.entries -1); i++){
				assert( p != NULL || p->next != NULL);
				assert( p->next->data.port > p->data.port);
				if((p->next->data.port - p->data.port) > 2){
					ins_port = p->data.port + 1;
					ins_p = p;
					flag = 1;
					break;
				}
				p = p->next;
			}
		}
	}
	// 
	// new entry 1
	if(flag == 0){
		ins_p = p;
		ins_port = ins_p->data.port+1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	// new entry 2
	ins_p = s;
	ins_port++;
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;

	//
	*port1 = ins_port - 1;
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);
	
	return 0;
}

int PORT_MANAGE_apply1_port2(unsigned int * const port)
{
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port != NULL);

	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_start;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else{
		// check cur ->  tail
		for (p = gIpPortList->m_curEntry; p->next != NULL; p = p->next) {
			if ((p->next->data.port - p->data.port) > 1){
				flag = 1;
				ins_p = p;
				ins_port = p->data.port + 1;
				break;
			}
		}
		// check tail -> max
		if (flag ==  0) {
			if ((gIpPortList->m_max - p->data.port) >= 1) {
				flag = 1;
				ins_p = p;
				ins_port = p->data.port + 1;
			}
		}
		// check min -> head
		if (flag ==  0) {
			if ((gIpPortList->m_link.next->data.port - gIpPortList->m_min) >= 1) {
				flag = 1;
				ins_p = &gIpPortList->m_link;
				ins_port =gIpPortList->m_min;
			}
		}
		// check head to current
		if (flag == 0) {
			for (p = gIpPortList->m_link.next; p != gIpPortList->m_curEntry && p->next != NULL; p = p->next) {
				if ((p->next->data.port - p->data.port) > 1){
					flag = 1;
					ins_p = p;
					ins_port = p->data.port + 1;
					break;
				}
			}
		}
	}
	// 
	// new entry
	if(flag == 0){
		LOCK_unlock(&gIpPortList->m_lock);
		printf("port is exhausted !!!!!!!!\n");
		assert(0);
		//return -1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	gIpPortList->m_cur = ins_port;
	gIpPortList->m_curEntry = s;
	//
	*port = ins_port;
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);

	return 0;
}

// apply two continuous ports, the first port is port1, the second port would be {port1 + 1}
int PORT_MANAGE_apply2_port2(unsigned int * const port1)
{
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port1 != NULL);
	
	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_start;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else{
		// check cur ->  tail
		for (p = gIpPortList->m_curEntry; p->next != NULL; p = p->next) {
			if ((p->next->data.port - p->data.port) > 2){
				flag = 1;
				ins_p = p;
				ins_port = p->data.port + 1;
				break;
			}
		}
		// check tail -> max
		if (flag ==  0) {
			if ((gIpPortList->m_max - p->data.port) >= 2) {
				flag = 1;
				ins_p = p;
				ins_port = p->data.port + 1;
			}
		}
		// check min -> head
		if (flag ==  0) {
			if ((gIpPortList->m_link.next->data.port - gIpPortList->m_min) >= 2) {
				flag = 1;
				ins_p = &gIpPortList->m_link;
				ins_port =gIpPortList->m_min;
			}
		}
		// check head to current
		if (flag == 0) {
			for (p = gIpPortList->m_link.next; p != gIpPortList->m_curEntry && p->next != NULL; p = p->next) {
				if ((p->next->data.port - p->data.port) > 2){
					flag = 1;
					ins_p = p;
					ins_port = p->data.port + 1;
					break;
				}
			}
		}
	}
	// 
	// new entry 1
	if(flag == 0){
		LOCK_unlock(&gIpPortList->m_lock);
		printf("port is exhausted !!!!!!!!\n");
		assert(0);
		//return -1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	// new entry 2
	ins_p = s;
	ins_port++;
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	gIpPortList->m_cur = ins_port;
	gIpPortList->m_curEntry = s;
	//
	*port1 = ins_port - 1;
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);
	
	return 0;
}

int PORT_MANAGE_apply1_port3(unsigned int * const port)
{
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port != NULL);

	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_cur;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else{
		// current -> next entry
		p = gIpPortList->m_curEntry;
		if (p->next) {
			if ((p->next->data.port - gIpPortList->m_cur) > 1){
				flag = 1;
				ins_p = p;
				ins_port = gIpPortList->m_cur + 1;
			}
		} else {
			if ((gIpPortList->m_max - gIpPortList->m_cur) >= 1){
				flag = 1;
				ins_p = p;
				ins_port = gIpPortList->m_cur + 1;
			}
		}
		// check cur ->  tail
		if (flag == 0) {
			for (p = gIpPortList->m_curEntry->next; p && p->next != NULL; p = p->next) {
				if ((p->next->data.port - p->data.port) > 1){
					flag = 1;
					ins_p = p;
					ins_port = p->data.port + 1;
					break;
				}
			}
		}
		// check min -> head
		if (flag ==  0) {
			if ((gIpPortList->m_link.next->data.port - gIpPortList->m_min) >= 1) {
				flag = 1;
				ins_p = &gIpPortList->m_link;
				ins_port =gIpPortList->m_min;
			}
		}
		printf("%s : %d\n", __FILE__, __LINE__);
		// check head to current
		if (flag == 0) {
			for (p = gIpPortList->m_link.next; p != gIpPortList->m_curEntry && p->next != NULL; p = p->next) {
				if ((p->next->data.port - p->data.port) > 1){
					flag = 1;
					ins_p = p;
					ins_port = p->data.port + 1;
					break;
				}
			}
		}
		printf("%s : %d\n", __FILE__, __LINE__);
	}
	// 
	// new entry
	if(flag == 0){
		LOCK_unlock(&gIpPortList->m_lock);
		printf("port is exhausted !!!!!!!!\n");
		assert(0);
		//return -1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	gIpPortList->m_cur = ins_port;
	gIpPortList->m_curEntry = s;
	//
	*port = ins_port;
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);

	return 0;
}

// apply two continuous ports, the first port is port1, the second port would be {port1 + 1}
int PORT_MANAGE_apply2_port3(unsigned int * const port1)
{
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port1 != NULL);
	
	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_cur;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else{		
		// current -> next entry
		p = gIpPortList->m_curEntry;
		if (p->next) {
			if ((p->next->data.port - gIpPortList->m_cur) > 2){
				flag = 1;
				ins_p = p;
				ins_port = gIpPortList->m_cur + 1;
			}
		} else {
			if ((gIpPortList->m_max - gIpPortList->m_cur) >= 2){
				flag = 1;
				ins_p = p;
				ins_port = gIpPortList->m_cur + 1;
			}
		}
		// check cur ->  tail		
		if (flag == 0) {
			for (p = gIpPortList->m_curEntry; p->next != NULL; p = p->next) {
				if ((p->next->data.port - p->data.port) > 2){
					flag = 1;
					ins_p = p;
					ins_port = p->data.port + 1;
					break;
				}
			}
		}
		// check min -> head
		if (flag ==  0) {
			if ((gIpPortList->m_link.next->data.port - gIpPortList->m_min) >= 2) {
				flag = 1;
				ins_p = &gIpPortList->m_link;
				ins_port =gIpPortList->m_min;
			}
		}
		// check head to current
		if (flag == 0) {
			for (p = gIpPortList->m_link.next; p != gIpPortList->m_curEntry && p->next != NULL; p = p->next) {
				if ((p->next->data.port - p->data.port) > 2){
					flag = 1;
					ins_p = p;
					ins_port = p->data.port + 1;
					break;
				}
			}
		}
	}
	// 
	// new entry 1
	if(flag == 0){
		LOCK_unlock(&gIpPortList->m_lock);
		printf("port is exhausted !!!!!!!!\n");
		assert(0);
		//return -1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	// new entry 2
	ins_p = s;
	ins_port++;
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	gIpPortList->m_cur = ins_port;
	gIpPortList->m_curEntry = s;
	//
	*port1 = ins_port - 1;
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);
	
	return 0;
}


int PORT_MANAGE_free_port(unsigned int port)
{
	unsigned int i;
	PortEntry_t *p,*prev;
	assert(gIpPortList != NULL);

	LOCK_lock(&gIpPortList->m_lock);

	prev = &gIpPortList->m_link;
	p = prev->next;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		assert( p != NULL);
		if(p->data.port == port) break;
		prev = p;
		p = p->next;
	}
	if(p && p->data.port == port){
		prev->next = p->next;
		gIpPortList->m_link.data.entries--;
		if (p == gIpPortList->m_curEntry) {
			if (gIpPortList->m_link.data.entries == 0){
			//	gIpPortList->m_cur = gIpPortList->m_start;
				gIpPortList->m_cur++;
			} 
			//else {
			//	gIpPortList->m_cur = prev->data.port;
			//}
			gIpPortList->m_curEntry = prev;
		}
		free(p);
	}
	
	PORT_MANAGE_dump();
	LOCK_unlock(&gIpPortList->m_lock);
	
	return 0;
}


#ifdef PM_TEST
int main(int argc,char *argv[])
{
	unsigned int port;
	int cmd = 0;
	int pos = 0;
	const char *usage=
		"*************************************************\r\n"\
		"**************PM DEBUG **************************\r\n"\
		"*********** 0 quit or destroy ********************\r\n"\
		"*********** 1 init *******************************\r\n"\
		"*********** 2 apply1 ****************************\r\n"\
		"*********** 3 apply2 ****************************\r\n"\
		"*********** 4 apply1_2 ****************************\r\n"\
		"*********** 5 apply2_2 ****************************\r\n"\
		"*********** 9 apply1_3 ****************************\r\n"\
		"*********** 10 apply2_3 ****************************\r\n"\
		"*********** 6 add ****************************\r\n"\
		"*********** 7 dump ****************************\r\n"\
		"*********** 8 free ******************************\r\n";
	
	while(1){
		printf(usage);
		printf("input your command: ");
		scanf("%d",&cmd);
		switch(cmd){
			case 0:
				PORT_MANAGE_destroy();
				exit(1);
			case 1:
				{
				unsigned int min, max;
				printf("input min and max port: ");
				scanf("%u %u",&min, &max);
				PORT_MANAGE_init(min, max);
				}
				break;
			case 2:
				PORT_MANAGE_apply1_port(&port);
				printf("Got 1 Port %u!!!!\n",port);
				break;
			case 3:
				PORT_MANAGE_apply2_port(&port);
				printf("Got 2 Port %u!!!!\n",port);
				break;
			case 4:
				PORT_MANAGE_apply1_port2(&port);
				printf("Got 1 Port %u!!!!\n",port);
				break;
			case 5:
				PORT_MANAGE_apply2_port2(&port);
				printf("Got 2 Port %u!!!!\n",port);
				break;		
			case 6:
				printf("input adding port: ");
				scanf("%u",&port);
				PORT_MANAGE_add_port(port);
				break;
			case 7:
				PORT_MANAGE_dump();
				break;
			case 8:
				printf("input free port: ");
				scanf("%u",&port);
				PORT_MANAGE_free_port(port);
				break;
			case 9:
				PORT_MANAGE_apply1_port3(&port);
				printf("Got 1 Port %u!!!!\n",port);
				break;
			case 10:
				PORT_MANAGE_apply2_port3(&port);
				printf("Got 2 Port %u!!!!\n",port);
				break;					
			default:
				PORT_MANAGE_dump();
				break;
		}
		printf("\r\n");
	}

	return 0;
}
#endif
