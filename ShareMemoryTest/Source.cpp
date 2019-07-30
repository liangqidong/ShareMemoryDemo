#include "..\ShareMemory\ShareMemory.h"
#pragma comment(lib, "ShareMemory.lib")
int main()
{
	ShareMemory smServer;
	unsigned int SMSize = 100;
	smServer.InitShareMemory("SM_TEST", true, SMSize);
	
	DataFormat dataFormt;
	dataFormt.bit = 4;
	dataFormt.dataType = RGB;
	dataFormt.high = 1920;
	dataFormt.wide = 1080;
	smServer.UpdateDataFormat(dataFormt);
	while (true)
	{
		getchar();
		smServer.SendShareData("sdsdsdA", sizeof("sdsdsdA"));
	}

	Sleep(19999999);
	smServer.UnInitShareMemory();

	getchar();
	return 0;
}