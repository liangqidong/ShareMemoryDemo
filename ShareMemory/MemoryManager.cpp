#include "MemoryManager.h"
#include "Lock.h"


MemoryManager::MemoryManager()
{
}


MemoryManager::~MemoryManager()
{

}

long MemoryManager::Init(unsigned int needMemory, unsigned int MemoryNum)
{
	for (size_t i = 0; i < MemoryNum; i++)
	{
		QItem item;
		item.pData = new unsigned char[needMemory];
		item.size = needMemory;
		m_writeVector.push_back(item);
	}

	return 0;
}

long MemoryManager::UnInit()
{
	for (auto it = m_writeVector.begin(); it != m_writeVector.end();)
	{
		QItem item = *it;
		delete[] it->pData;
		it->pData = nullptr;
		m_writeVector.erase(m_writeVector.begin());
		it = m_writeVector.begin();
	}
	for (auto it = m_readVector.begin(); it != m_readVector.end();)
	{
		QItem item = *it;
		delete[] it->pData;
		it->pData = nullptr;
		m_readVector.erase(m_readVector.begin());
		it = m_readVector.begin();
	}
	return 0;
}

long MemoryManager::ReadBuff(void*buff, unsigned int& buffLen)
{
	AutoLock<Mutex> lock(m_hMutex);
	QItem item;
	auto it = m_readVector.rbegin();
	if (it == m_readVector.rend())
	{
		buffLen = 0;
		return -1;
	}
	item = *it;
	m_readVector.pop_back();
	memcpy(buff, item.pData, item.dataLen);
	buffLen = item.dataLen;
	item.dataLen = 0;
	m_writeVector.insert(m_writeVector.begin(), item);
	return 0;
}

long MemoryManager::WriteBuff(void*buff, unsigned int buffLen)
{
	AutoLock<Mutex> lock(m_hMutex);
	QItem item;
	auto it = m_writeVector.rbegin();
	
	if (buffLen > it->size)
	{
		delete[] it->pData;
		it->pData = new unsigned char[buffLen];
		it->size = buffLen;
	}
	item = *it;
	m_writeVector.pop_back();
	
	item.dataLen = buffLen;
	memcpy(item.pData, buff, buffLen);
	m_readVector.insert(m_readVector.begin(), item);
	return 0;
}
