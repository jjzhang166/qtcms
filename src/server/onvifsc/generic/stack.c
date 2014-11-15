/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : stack.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/3/21
  Last Modified : 2014/3/21
  Description   : stack
 	
  History       : 
  1.Date        : 2014/3/21
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#include "stdinc.h"

#include"stack.h"

static int stack_default_comp(StackItem_t a, StackItem_t b)
{
	return -1;
}

JStack_t *STACK_init(int item_size)
{
	JStack_t *ps = (JStack_t *)malloc(sizeof(JStack_t));
	assert(ps != NULL);
	ps->top = NULL;
	ps->size = 0;
	ps->item_size = item_size;
	ps->f_comp = stack_default_comp;
	ps->mutex = rwlock_create();
	return ps;
}

void STACK_set_comp(JStack_t *ps, fItemCompare comp)
{
	if (ps) {
		ps->f_comp = comp;
	}
}

inline bool stack_is_empty(JStack_t *ps)
{
	bool flag;
	if (ps == NULL) return true;
	if(ps->top == NULL && ps->size == 0)
		flag = true;
	else
		flag = false;
	return flag;
}


bool STACK_is_empty(JStack_t *ps)
{
	bool flag;
	if (rwlock_rdlock(ps->mutex) != 0){
		return true;
	}
	flag = stack_is_empty(ps);
	rwlock_unlock(ps->mutex);
	return flag;
}

static inline int stack_get_size(JStack_t *ps)
{
	return ps->size;
}


int STACK_get_size(JStack_t *ps)
{
	int size;
	if (rwlock_rdlock(ps->mutex) != 0){
		return 0;
	}
	size =  stack_get_size(ps);
	rwlock_unlock(ps->mutex);
	return size;
}

PNode stack_get_top(JStack_t *ps,StackItem_t pitem)
{
	if(stack_is_empty(ps) != true && pitem!=NULL)
	{
		memcpy(pitem, ps->top->data, ps->item_size);
	}
	return ps->top;
}


PNode STACK_get_top(JStack_t *ps,StackItem_t pitem)
{
	PNode node = NULL;
	if (rwlock_rdlock(ps->mutex) != 0){
		return NULL;
	}
	node = stack_get_top(ps, pitem);
	rwlock_unlock(ps->mutex);
	return node;
}


PNode STACK_push(JStack_t *ps,StackItem_t item)
{
	PNode pnode =NULL;

	pnode = (PNode)malloc(sizeof(Node));
	assert(pnode != NULL);
	pnode->data = (StackItem_t)malloc(ps->item_size);
	assert(pnode->data != NULL);

	if (rwlock_wrlock(ps->mutex) != 0){
		return NULL;
	}
	memcpy(pnode->data, item, ps->item_size);
	pnode->down = stack_get_top(ps,NULL);
	ps->size++;
	ps->top = pnode;
	rwlock_unlock(ps->mutex);
		
	return pnode;
}

PNode STACK_push_to_bottom(JStack_t *ps,StackItem_t item)
{
	PNode pnode =NULL;
	PNode p = NULL;
	int i ;

	pnode = (PNode)malloc(sizeof(Node));
	assert(pnode != NULL);
	pnode->data = (StackItem_t)malloc(ps->item_size);
	assert(pnode->data != NULL);
	
	if (rwlock_wrlock(ps->mutex) != 0){
		printf("push to stask failed!, because lock failed!\n");
		return NULL;
	}
	p = ps->top;
	i = ps->size;
	while(i > 1)
	{
		p = p->down;
		i--;
	}
	memcpy(pnode->data, item, ps->item_size);
	pnode->down = NULL;
	ps->size++;
	if (p) p->down = pnode;
	if (ps->top == NULL) ps->top = pnode;
	//printf("stack current size: %d\n", ps->size);
	rwlock_unlock(ps->mutex);
		
	return pnode;
}

PNode STACK_push_by_inc(JStack_t *ps,StackItem_t item, bool allow_same /*keep multi copy in a stacks*/)
{
	PNode pnode =NULL;
	PNode p = NULL;
	PNode prev = NULL;
	int i ;

	pnode = (PNode)malloc(sizeof(Node));
	assert(pnode != NULL);
	pnode->data = (StackItem_t)malloc(ps->item_size);
	assert(pnode->data != NULL);
	
	if (rwlock_wrlock(ps->mutex) != 0){
		printf("push to stask failed!, because lock failed!\n");
		return NULL;
	}
	// got insert position
	p = ps->top;
	i = ps->size;
	prev = ps->top;
	while(i > 0)
	{
		if (ps->f_comp(item, p->data) < 0)  // item < p
		{
			break;
		}
		else if (ps->f_comp(item, p->data) == 0)  // item == p
		{
			if (allow_same == false) {
				free(pnode->data);
				free(pnode);
				rwlock_unlock(ps->mutex);
				return p;	
			} else {
				break;
			}
		}
		prev = p;
		p = p->down;
		i--;
	}
	//
	memcpy(pnode->data, item, ps->item_size);
	if (ps->top == NULL || i == ps->size) { // add to top
		pnode->down = ps->top;
		ps->top = pnode;
	} else{
		pnode->down = prev->down;
		prev->down = pnode;
	}
	//
	ps->size++;
	//printf("current size: %d\n", ps->size);
	rwlock_unlock(ps->mutex);
		
	return pnode;	
}

int STACK_pop(JStack_t *ps,StackItem_t pitem)
{
	PNode p = NULL;
	int ret = -1;
	
	if (ps == NULL) return -1;
	if (rwlock_wrlock(ps->mutex) != 0){
		printf("stack pop failed , because lock failed!\n");
		return -1;
	}
	p = ps->top;
	if(stack_is_empty(ps) != true && p != NULL)
	{
		if(pitem != NULL)
			memcpy(pitem, p->data, ps->item_size);
		ps->size--;
		ps->top = ps->top->down;	
		free(p->data);
		free(p);
		p = NULL;
		ret = 0;
	} 
	//printf("pop a item, current size: %d\n", ps->size);
	rwlock_unlock(ps->mutex);
	return ret;
}


void STACK_clear(JStack_t *ps)
{
	//printf("$$$ stack clear now $$$$$\n");
	while(STACK_is_empty(ps ) != true)
	{
		STACK_pop(ps,NULL);
	}
}



void STACK_destroy(JStack_t *ps)
{
	if (ps != NULL) {
		STACK_clear(ps);
		rwlock_destroy(ps->mutex);
		free(ps);
	}
}


void STACK_traverse(JStack_t *ps,void (*visit)())
{
	PNode p = NULL;
	int i ;
	if (ps == NULL) return;
	if (rwlock_rdlock(ps->mutex) != 0){
		return;
	}
	p = ps->top;
	i = ps->size;
	while(i--)
	{
		visit(p->data);
		p = p->down;
	}
	rwlock_unlock(ps->mutex);
}

JStack_t *STACK_dup(JStack_t *ps)
{
	JStack_t *stack;
	PNode p = NULL;
	int i ;
	if (ps == NULL) return NULL;

	stack = STACK_init(ps->item_size);
	if (stack == NULL) return NULL;
	if (rwlock_rdlock(ps->mutex) != 0){
		return NULL;
	}
	p = ps->top;
	i = ps->size;
	while(i--)
	{
		STACK_push(stack, p->data);
		p = p->down;
	}
	stack->f_comp = ps->f_comp;
	rwlock_unlock(ps->mutex);

	return stack;
}


bool STACK_find(JStack_t *ps, StackItem_t pitem)
{
	PNode p = NULL;
	int i ;
	bool found =false;
	if (ps == NULL) return false;
	if (rwlock_rdlock(ps->mutex) != 0){
		return false;
	}
	p = ps->top;
	i = ps->size;
	while(i--)
	{
		if ( 0 == memcmp(p->data, pitem,ps->item_size)) {
			found = true;
			break;
		}
		p = p->down;
	}
	rwlock_unlock(ps->mutex);
	return found;
}

int STACK_find2(JStack_t *ps, StackItem_t pitem)
{
	PNode p = NULL;
	int i , repeat_count = 0;
	if (ps == NULL) return false;
	if (rwlock_rdlock(ps->mutex) != 0){
		return -1;
	}
	p = ps->top;
	i = ps->size;
	while(i--)
	{
		if (ps->f_comp(pitem, p->data) == 0) { // item == p
			repeat_count++;
		}
		p = p->down;
	}
	rwlock_unlock(ps->mutex);
	return repeat_count;
}

int STACK_del(JStack_t *ps,StackItem_t pitem)
{
	PNode p = NULL;
	PNode prev = NULL;
	int i ;
	if (ps == NULL) return false;
	if (rwlock_wrlock(ps->mutex) != 0){
		return -1;
	}
	p = ps->top;
	i = ps->size;
	while(i--)
	{
		if (ps->f_comp(pitem, p->data) == 0) { 
			if (prev) {
				prev->down = p->down;
			} else {
				ps->top = p->down;
			}
			ps->size--;
			break;
		}
		prev = p;
		p = p->down;
	}
	rwlock_unlock(ps->mutex);
	return 0;
}

