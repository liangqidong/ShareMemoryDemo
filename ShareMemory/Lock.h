#ifndef _Lock_H
#define _Lock_H

template <class T> 
class AutoLock
{
public:
	AutoLock(T& lock);
	~AutoLock();
private:
	T& m_Lock;
};

template <class T>
AutoLock<T>::AutoLock(T& lock)
: m_Lock(lock)
{
	m_Lock.Lock();
}

template <class T>
AutoLock<T>::~AutoLock()
{
	m_Lock.Unlock();
}

#endif // _Lock_H
