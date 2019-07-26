#include <Windows.h> 
#include <iostream> 
using namespace std;

int main()
{
	HANDLE hMutex = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpShareMemory = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;

	//create share memory 
	hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		1024 * 1024,
		L"ShareMemoryTest");

	if (NULL == hFileMapping)
	{
		cout << "CreateFileMapping fail:" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	lpShareMemory = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,   //memory start address 
		0);   //all memory space 
	if (NULL == lpShareMemory)
	{
		cout << "MapViewOfFile" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	hMutex = CreateMutex(NULL, FALSE, L"SM_Mutex");
	if (NULL == hMutex || ERROR_ALREADY_EXISTS == GetLastError())
	{
		cout << "CreateMutex" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}//多个线程互斥访问 

	 //send data 
	hServerWriteOver = CreateEvent(NULL,
		TRUE,
		FALSE,
		L"ServerWriteOver");
	hClientReadOver = CreateEvent(NULL,
		TRUE,
		FALSE,
		L"ClientReadOver");
	if (NULL == hServerWriteOver ||
		NULL == hClientReadOver)
	{
		cout << "CreateEvent" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	char p = 0;
	char* q = (char*)lpShareMemory;
	do
	{
		p = getchar();
		if (WaitForSingleObject(hClientReadOver, 5 * 1000) != WAIT_OBJECT_0)
			goto SERVER_SHARE_MEMORY_END;
		q[0] = p;
		if (!ResetEvent(hClientReadOver)) goto SERVER_SHARE_MEMORY_END;//把指定的事件对象设置为无信号状态 
		if (!SetEvent(hServerWriteOver)) goto SERVER_SHARE_MEMORY_END;//把指定的事件对象设置为有信号状态 
	} while (p != '\n');

SERVER_SHARE_MEMORY_END:
	//release share memory 
	if (NULL != hServerWriteOver)  CloseHandle(hServerWriteOver);
	if (NULL != hClientReadOver)  CloseHandle(hClientReadOver);
	if (NULL != lpShareMemory)   UnmapViewOfFile(lpShareMemory);
	if (NULL != hFileMapping)    CloseHandle(hFileMapping);
	if (NULL != hMutex)       ReleaseMutex(hMutex);
	return 0;
}