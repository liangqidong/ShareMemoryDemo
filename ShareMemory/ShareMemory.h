#ifndef SHARE_MEMORY_H
#define SHARE_MEMORY_H
#include <functional>
#include <windows.h>
#include <thread>

#include "MemoryManager.h"

enum DataType
{
	AUDIO_DATA,
	RGB,
	RGBA,
};
struct DataFormat
{
	unsigned int wide;
	unsigned int high;
	unsigned int bit;
	DataType	 dataType;

};

struct Data
{
	DataFormat dataFormt;
	unsigned int dataLen;
	void* data;

};

#define DataCallbackFun std::function<long(Data*)> 

class ShareMemory
{
public:
	ShareMemory();
	~ShareMemory();

	long InitShareMemory(char* shareMemoryName,
						bool isServer,
						unsigned int needMemory);
	long RegDataCallBack(DataCallbackFun funCallback);
	long SendShareData(void* data, unsigned long datalen);
	long UnInitShareMemory();
	long UpdateDataFormat(DataFormat dataFaormt);
	long GetDataFormat(DataFormat& dataFaormt);

private:
	long InitShareMemoryServer(char* shareMemoryName,
		unsigned int needMemory);
	long InitShareMemoryClient(char* shareMemoryName);
	void SendThread();
	void RecThread();

	std::thread		m_thread;
	DataCallbackFun m_fun;
	MemoryManager	m_memoryManager;
	bool			m_isServer;
	HANDLE			m_hMutex;
	HANDLE			m_hFileMapping;
	LPVOID			m_lpShareMemory;
	HANDLE			m_hServerWriteOver;
	HANDLE			m_hClientReadOver;
	HANDLE			m_threadQuit;
	DataFormat		m_dataFormt;
};
#endif//SHARE_MEMORY_H


