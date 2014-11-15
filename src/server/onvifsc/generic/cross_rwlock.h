#ifndef __CROSS_RWLOCK_HEAD_FILE__
#define __CROSS_RWLOCK_HEAD_FILE__

typedef void * HRWLOCK;

HRWLOCK rwlock_create();

void rwlock_destroy(HRWLOCK rwlock);

int rwlock_rdlock(HRWLOCK rwlock);

int rwlock_wrlock(HRWLOCK rwlock);

int rwlock_unlock(HRWLOCK rwlock);

#endif