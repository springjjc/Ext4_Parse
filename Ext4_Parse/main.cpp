#include<iostream>
#include "Ext4_Parse.h"
#include<Windows.h>
//#include "shlwapi.h"
//#pragma comment(lib,"shlwapi.lib")
using namespace std;


int main()
{
	string test="≤‚ ‘";
	cout<<test.size()<<endl;
	cout<<sizeof(long)<<endl;
	PartitionTableInfo *partitions=new PartitionTableInfo("E:\\hmnt3.bin");
	string filePath="E:\\hmnt3.bin";
	string storeDir=filePath.substr(0,filePath.find_last_of('.'));
	bool ret=CreateDirectory(storeDir.c_str(),nullptr);
	if(!partitions->hashPartition.count("userdata"))
		return 0;
	Int64 userdataPos=partitions->hashPartition["userdata"].partitionBegin;
	Parse_image *parse_image=new Parse_image(userdataPos);
	parse_image->startParse(filePath,"");
	delete parse_image;
	system("pause");
}