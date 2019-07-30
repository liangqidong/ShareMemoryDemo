#include "..\ShareMemory\ShareMemory.h"
#pragma comment(lib, "ShareMemory.lib")
#include <iostream>
#include <string>
using namespace std;
long DataFunCall(Data* data)
{
	int dataLen = data->dataLen;
	char* data1 = (char*)data->data;
	int bit = data->dataFormt.bit;
	DataType dataType  = data->dataFormt.dataType;
	int high = data->dataFormt.high;
	int wide = data->dataFormt.wide;
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" ;
	printf("dataLen:%d\n", dataLen);
	printf("bit:%d\n", bit);
	printf("high:%d\n", high);
	printf("wide:%d\n", wide);
	cout<< "data:"<< data1;
	cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	return 0;
}

int main()
{

	ShareMemory smClient;
	unsigned int SMSize = 100;
	smClient.InitShareMemory("SM_TEST",false, SMSize);
	smClient.RegDataCallBack(std::bind(&DataFunCall, std::placeholders::_1));
	DataFormat dataFormt;
	smClient.GetDataFormat(dataFormt);
	getchar();
	smClient.UnInitShareMemory();
	return 0;
}
