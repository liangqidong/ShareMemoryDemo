#ifndef _Mutex_H
#define _Mutex_H


#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();
	void Unlock();
private:
#ifdef WIN32
	CRITICAL_SECTION m_mutex;
#else
	pthread_mutex_t m_mutex;
#endif
};

#endif // _Mutex_H
