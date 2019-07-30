#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <vector>
#include "Mutex.h"

struct QItem
{
	PBYTE pData;
	unsigned int size;
	unsigned int dataLen;
	QItem()
	{
		pData = NULL;
		size = 0;
		dataLen = 0;
	};
};

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();
	long Init(unsigned int needMemory,unsigned int MemoryNum);
	long UnInit();
	long ReadBuff(void*buff, unsigned int& buffLen);
	long WriteBuff(void*buff, unsigned int buffLen);

private:
	std::vector<QItem> m_writeVector;
	std::vector<QItem> m_readVector;
	Mutex m_hMutex;
};
#endif//MEMORY_MANAGER_H


