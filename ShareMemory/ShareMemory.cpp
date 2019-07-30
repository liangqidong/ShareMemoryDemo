#include "ShareMemory.h"


#define ShareMemoryNum		2
ShareMemory::ShareMemory()
	: m_hMutex(nullptr)
	, m_hFileMapping(nullptr)
	, m_lpShareMemory(nullptr)
	, m_hServerWriteOver(nullptr)
	, m_hClientReadOver(nullptr)
	, m_isServer(false)
	, m_fun(nullptr)
{
}


ShareMemory::~ShareMemory()
{
}

long ShareMemory::InitShareMemory(char* shareMemoryName, bool isServer, unsigned int needMemory)
{
	long errorCode = 0;
	char* msg = nullptr;
	m_isServer = isServer;
	m_threadQuit = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (isServer)
	{
		msg = "init Share Memory Server\n";
		m_memoryManager.Init(needMemory, ShareMemoryNum);
		errorCode = InitShareMemoryServer(shareMemoryName, needMemory);

	}
	else
	{
		msg = "init Share Memory Client\n";
		errorCode = InitShareMemoryClient(shareMemoryName);
	}

	return errorCode;
}

long ShareMemory::RegDataCallBack(DataCallbackFun funCallback)
{
	m_fun = funCallback;
	return 0;
}

long ShareMemory::SendShareData(void* data, unsigned long datalen)
{
	m_memoryManager.WriteBuff(data, datalen);
	return 0;
}

long ShareMemory::UnInitShareMemory()
{
	//thread  quit
	SetEvent(m_threadQuit);//设置有信号
	if (m_thread.joinable())
	{
		m_thread.join();
	}

	if (NULL != m_hServerWriteOver)
	{
		CloseHandle(m_hServerWriteOver);
	}
	if (NULL != m_hClientReadOver)
	{
		CloseHandle(m_hClientReadOver);
	}
	if (NULL != m_lpShareMemory)
	{
		UnmapViewOfFile(m_lpShareMemory);
	}
	if (NULL != m_hFileMapping)
	{
		CloseHandle(m_hFileMapping);
	}
	if (NULL != m_hMutex)
	{
		ReleaseMutex(m_hMutex);
	}
	m_memoryManager.UnInit();
	return 0;
}

long ShareMemory::UpdateDataFormat(DataFormat dataFormt)
{
	memcpy(&m_dataFormt, &dataFormt, sizeof(DataFormat));
	return 0;
}

long ShareMemory::GetDataFormat(DataFormat& dataFormt)
{
	memcpy(&dataFormt, &m_dataFormt, sizeof(DataFormat));

	return 0;
}

long ShareMemory::InitShareMemoryServer(char* shareMemoryName, unsigned int needMemory)
{
	char* msg = nullptr;
	long errorCode = 0;
	char nameBuff[MAX_PATH] = { 0 };
	//create share memory 
	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "SM_Name_%s", shareMemoryName);
	m_hFileMapping = CreateFileMappingA(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		needMemory+ sizeof(Data),
		nameBuff);

	if (NULL == m_hFileMapping)
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "CreateFileMapping fail:%d\n", GetLastError());
		msg = arr;
		goto SERVER_SHARE_MEMORY_END;
	}

	m_lpShareMemory = MapViewOfFile(m_hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,   //memory start address 
		0);   //all memory space 
	if (NULL == m_lpShareMemory)
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "MapViewOfFile:%d\n", GetLastError());
		msg = arr;
		goto SERVER_SHARE_MEMORY_END;
	}

	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "%s_Mutex", shareMemoryName);
	m_hMutex = CreateMutexA(NULL, FALSE, nameBuff);
	if (NULL == m_hMutex )
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "CreateMutex:%d\n", GetLastError());
		msg = arr;
		goto SERVER_SHARE_MEMORY_END;
	}//多个线程互斥访问 

	 //send data 
	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "%s_ServerWriteOver", shareMemoryName);
	m_hServerWriteOver = CreateEventA(NULL,
		TRUE,
		FALSE,
		nameBuff);
	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "%s_ClientReadOver", shareMemoryName);
	m_hClientReadOver = CreateEventA(NULL,
		TRUE,
		FALSE,
		nameBuff);
	if (NULL == m_hServerWriteOver ||
		NULL == m_hClientReadOver)
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "CreateEvent:%d\n", GetLastError());
		msg = arr;
		goto SERVER_SHARE_MEMORY_END;
	}
	m_thread = std::thread(&ShareMemory::SendThread,this);
SERVER_SHARE_MEMORY_END:

	return errorCode;
}

long ShareMemory::InitShareMemoryClient(char* shareMemoryName)
{
	char* msg = nullptr;
	long errorCode = 0;
	char nameBuff[MAX_PATH] = { 0 };
	sprintf_s(nameBuff, "%s_Mutex", shareMemoryName);
	m_hMutex = OpenMutexA(MUTEX_ALL_ACCESS,
		FALSE,
		nameBuff);
	if (NULL == m_hMutex)
	{
		if (ERROR_FILE_NOT_FOUND == GetLastError())
		{
			errorCode = 0;
			msg = "OpenMutex fail: file not found!\n";
			goto CLIENT_SHARE_MEMORY_END;
		}
		else
		{
			errorCode = 0;
			char arr[MAX_PATH] = { 0 };
			sprintf_s(arr, "OpenMutex fail:%d\n", GetLastError());
			msg = arr;
			goto CLIENT_SHARE_MEMORY_END;
		}
	}

// 	if (WaitForSingleObject(m_hMutex, 5000) != WAIT_OBJECT_0)//hMutex 一旦互斥对象处于有信号状态，则该函数返回 
// 	{
// 		errorCode = 0;
// 		char arr[MAX_PATH] = { 0 };
// 		sprintf_s(arr, "WaitForSingleObject:%d\n", GetLastError());
// 		msg = arr;
// 		goto CLIENT_SHARE_MEMORY_END;
// 	}

	//open share memory 
	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "SM_Name_%s", shareMemoryName);
	m_hFileMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS,
		FALSE,
		nameBuff);
	if (NULL == m_hFileMapping)
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "OpenFileMapping:%d\n", GetLastError());
		msg = arr;
		goto CLIENT_SHARE_MEMORY_END;
	}

	m_lpShareMemory = MapViewOfFile(m_hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);
	if (NULL == m_lpShareMemory)
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "MapViewOfFile:%d\n", GetLastError());
		msg = arr;
		goto CLIENT_SHARE_MEMORY_END;
	}

	//read and write data 
	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "%s_ServerWriteOver", shareMemoryName);
	m_hServerWriteOver = CreateEventA(NULL,
		TRUE,
		FALSE,
		nameBuff);
	memset(nameBuff, 0, MAX_PATH);
	sprintf_s(nameBuff, "%s_ClientReadOver", shareMemoryName);
	m_hClientReadOver = CreateEventA(NULL,
		TRUE,
		FALSE,
		nameBuff);
	if (NULL == m_hServerWriteOver ||
		NULL == m_hClientReadOver)
	{
		errorCode = 0;
		char arr[MAX_PATH] = { 0 };
		sprintf_s(arr, "CreateEvent:%d\n", GetLastError());
		msg = arr;
		goto CLIENT_SHARE_MEMORY_END;
	}
	m_thread = std::thread(&ShareMemory::RecThread, this);

CLIENT_SHARE_MEMORY_END:

	return errorCode;
}

void ShareMemory::SendThread()
{
	Data data;
	data.data = (char*)m_lpShareMemory + sizeof(Data);
	
	while (WaitForSingleObject(m_threadQuit, 10) == WAIT_TIMEOUT)
	{
		m_memoryManager.ReadBuff(data.data, data.dataLen);
		if (0 == data.dataLen)
		{
			continue;
		}

		if (WaitForSingleObject(m_hClientReadOver, 30) == WAIT_TIMEOUT)
		{
			continue;
		}
		memcpy(&data.dataFormt, &m_dataFormt, sizeof(DataFormat));
		memcpy(m_lpShareMemory, &data, sizeof(Data));
		ResetEvent(m_hClientReadOver);//把指定的事件对象设置为无信号状态 
		SetEvent(m_hServerWriteOver);//把指定的事件对象设置为有信号状态 
	}
}

void ShareMemory::RecThread()
{
	
	while (WaitForSingleObject(m_threadQuit, 10) == WAIT_TIMEOUT)
	{
		SetEvent(m_hClientReadOver);
		if (WaitForSingleObject(m_hServerWriteOver, 40) == WAIT_TIMEOUT)
		{
			continue;
		}
		if (nullptr == m_fun)
		{
			continue;
		}
		Data* data = (Data*)m_lpShareMemory;
		if (nullptr == data)
		{
			continue;
		}
		data->data = (char*)m_lpShareMemory + sizeof(Data);
		m_fun(data);
		memcpy(&m_dataFormt, &data->dataFormt,  sizeof(DataFormat));
		ResetEvent(m_hServerWriteOver);
	}
}
