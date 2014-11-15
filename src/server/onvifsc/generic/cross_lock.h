#ifndef __CROSS_LOCK_HEAD_FILE__
#define __CROSS_LOCK_HEAD_FILE__

typedef void * HMUTEX;

HMUTEX mutex_create();

void mutex_destroy(HMUTEX mutex);

void mutex_lock(HMUTEX mutex);

void mutex_unlock(HMUTEX mutex);

#endif