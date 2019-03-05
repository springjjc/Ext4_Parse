#include"Ext4_Parse.h"
#include<math.h>
#include<Windows.h>
#include <direct.h> //_mkdir函数的头文件
#include <io.h>     //_access函数的头文件
void CreateDir( const char *dir )
{
	int m = 0, n;
	string str1, str2;   
	str1 = dir;
	str2 = str1.substr( 0, 2 );
	str1 = str1.substr( 3, str1.size() );
	while( m >= 0 )
	{
		m = str1.find('\\');
		str2 += '\\' + str1.substr( 0, m );    
		n = _access( str2.c_str(), 0 ); //判断该目录是否存在
		if( n == -1 )
		{
			_mkdir( str2.c_str() );     //创建目录
		}  
		str1 = str1.substr( m+1, str1.size() );
	}
}

int reverse_bytes(byte *p, char c) {
	int r = 0;
	int i;
	for (i=c-1; i>=0; i--) 
		r |= ( *(p+i) <<8*i);
	return r;
}
Int64 Bit2Int64(byte *Bytes,int index)
{
	Int64 r=0;
	Bytes+=index;
	for(int i=7;i>=0;i--)
		r|=(*(Bytes+i)<<8*i);
	return r;
}
UInt64 Bit2UInt64(byte *Bytes,int index)
{
	UInt64 r=0;
	Bytes+=index;
	for(int i=7;i>=0;i--)
		r|=(*(Bytes+i)<<8*i);
	return r;
}
Int32 Bit2Int32(byte* Bytes,int index)
{
	Int32 r=0;
	Bytes+=index;
	for(int i=3;i>=0;i--)
		r|=(*(Bytes+i)<<8*i);
	return r;
}
UInt32 Bit2UInt32(byte* Bytes,int index)
{
	UInt32 r=0;
	Bytes+=index;
	for(int i=3;i>=0;i--)
		r|=(*(Bytes+i)<<8*i);
	return r;
}
Int16 Bit2Int16(byte* Bytes,int index)
{
	Int16 r=0;
	Bytes+=index;
	for(int i=1;i>=0;i--)
		r|=(*(Bytes+i)<<8*i);
	return r;
}
UInt16 Bit2UInt16(byte* Bytes,int index)
{
	UInt16 r=0;
	Bytes+=index;
	for(int i=1;i>=0;i--)
		r|=(*(Bytes+i)<<8*i);
	return r;
}
string Bit2Str(byte *Bytes,int index,int count)
{
	return string((const char*)Bytes,index,count);
}
string UTF8Bit2str(const char *Bytes,int index,int count)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, Bytes+index, -1, NULL, 0); 
	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
	memset(pwBuf, 0, nwLen * 2 + 2); 
	MultiByteToWideChar(CP_UTF8, 0, Bytes+index, count, pwBuf, nwLen); 
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL); 
	char * pBuf = new char[nLen + 1]; 
	memset(pBuf, 0, nLen + 1); 
	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL); 
	std::string retStr = pBuf; 
	delete []pBuf; 
	delete []pwBuf; 
	pBuf = NULL; 
	pwBuf = NULL; 
	return retStr; 

}
string Wchar_tToString(wchar_t *wchar,int len)
{
	setlocale(LC_CTYPE, "");
	len*=4;
	char *p = new char[len];
	wcstombs(p,wchar,len);
	std::string str(p);
	delete[] p;
	return str;
} 
string UTF16Bit2Str(byte *Bytes,int index,int count) 
{
	wchar_t* wchar=(wchar_t*)malloc(count);
	memcpy(wchar,Bytes+index,count);
	string res=Wchar_tToString(wchar,count);
	free(wchar);
	return res;
}

PartitionTableInfo::PartitionTableInfo(string imgPath)
{
	FILE *fp=fopen(imgPath.c_str(),"rb+");
	fseek(fp,0x200,SEEK_SET);
	byte *tempBytes=(byte*)malloc(512);
	fread(tempBytes,1,512,fp);
	cout<<Bit2Str(tempBytes,0,8)<<endl;
	Int32 partionOfCount=Bit2Int32(tempBytes,80);
	free(tempBytes);
	for(int i=0,offset=1024;i<partionOfCount;i++)
	{
		PartitionInfo partition;
		offset+=128;
		fseek(fp,offset,SEEK_SET);
		byte Bytes[128];
		fread(Bytes,128,1,fp);
		partition.partitionBegin=Bit2Int64(Bytes,0x20)*0x200;
		partition.partitionEnd=Bit2Int64(Bytes,0x28)*0x200;
		partition.partitionLen=partition.partitionEnd-partition.partitionBegin;
		string partitionName=UTF16Bit2Str(Bytes,0x38,72);
		if(!partitionName.empty()&&!hashPartition.count(partitionName))
			hashPartition[partitionName]=partition; 
		cout<<partitionName<<endl;
	}
	fclose(fp);

}

void Parse_image::initSuperBlock()
{
	const UInt16 sizeOfSuperBlock=1024;
	Int64 offset =1024+userdataPos; //起始偏移量为两个扇区 512*2
	_fseeki64(fp,offset,SEEK_SET);
	byte Bytes[sizeOfSuperBlock]; //超级块大小为1024
	fread(Bytes,sizeOfSuperBlock,1,fp);
	superblock=new SuperBlock(Bytes);
}
void Parse_image::initVecOfBGDS()
{
	const UInt16 sizeOfBGD=32;
	Int64 offset=userdataPos+superblock->block_size_in_byte;
	double temp=superblock->total_block_count*1.0/superblock->blocks_per_group;
	UInt32 countOfBGD = temp - (UInt32)temp < 0.00000001 ? (UInt32)temp : (UInt32)temp + 1; //消除小数点
	UInt32 sizeOfBGDS=sizeOfBGD*countOfBGD;
	_fseeki64(fp,offset,SEEK_SET);
	byte *totalBytes=(byte*)malloc(sizeOfBGDS);
	fread(totalBytes,sizeOfBGDS,1,fp);
	for(int i=0;i<countOfBGD;i++)
	{
		byte Bytes[sizeOfBGD];
		memcpy(Bytes,totalBytes+i*sizeOfBGD,sizeOfBGD);
		vecOfBGDS.push_back(BlockGroupDescriptor(Bytes));
	}
}
void Parse_image::traversalExtentTree(byte* bytesOfExtentArea,vector<Ext4_extent> &extents)
{
	const int sizeOfItemExtentNode=12; //extent树节点大小
	byte itemBytes[sizeOfItemExtentNode];
	memcpy(itemBytes,bytesOfExtentArea,sizeOfItemExtentNode);
	Ext4_extent_header header(itemBytes); //构造头节点
	if(0==header.eh_depth) //判断该层是否为叶子节点
	{
		for(UInt16 i=1;i<=header.eh_entries;i++) //遍历有效节点
		{
			memcpy(itemBytes,bytesOfExtentArea+sizeOfItemExtentNode*i,sizeOfItemExtentNode);
			extents.push_back(Ext4_extent(itemBytes)); //保存extent树的叶子点，叶结点包含文件内容的块序号
		}
	}
	else
	{
		for(UInt16 i=1;i<=header.eh_entries;i++) //内部节点
		{
			memcpy(itemBytes,bytesOfExtentArea+sizeOfItemExtentNode*i,sizeOfItemExtentNode);
			Ext4_extent_idx idx(itemBytes);
			byte bytesOfChildNode[1024];
			UInt64 offset=idx.ei_leaf*superblock->block_size_in_byte; //该层节点在分区的起始位移
			_fseeki64(fp,offset+userdataPos,SEEK_SET);
			fread(bytesOfChildNode,1024,1,fp);
			traversalExtentTree(bytesOfChildNode,extents); //递归遍历extent树
		}
	}


}
void Parse_image::readAllBytesOfDirTab(byte *allBytesOfDirTab,vector<Ext4_extent> &extents)
{
	UInt32 totalBlocks=0;
	for(auto extent:extents)
	{
		UInt64 offset =extent.firstBlockNumberInThisExtent*superblock->block_size_in_byte;
		_fseeki64(fp,offset+userdataPos,SEEK_SET);
		fread(allBytesOfDirTab+totalBlocks*superblock->block_size_in_byte,extent.totalCoveredBlockByThisExtent*superblock->block_size_in_byte,1,fp);
		totalBlocks+=extent.totalCoveredBlockByThisExtent;
	}
}
void Parse_image::dealContentOfDir(unordered_map<string,UInt32> &hashFileAndInodeNumber,byte *allBytesOfDirTab,UInt32 totalSize)
{
	UInt32 totalBytesUsed=0;
	UInt32 inodeNumber=Bit2Int32(allBytesOfDirTab,0);
	while(inodeNumber!=0)
	{
		UInt16 dirLen=Bit2Int16(allBytesOfDirTab,totalBytesUsed+4);
		string fileName=UTF8Bit2str((const char *)allBytesOfDirTab,totalBytesUsed+8,(int)allBytesOfDirTab[totalBytesUsed+6]);
		if(fileName.compare(".")!=0&&fileName.compare("..")!=0)
		{
			hashFileAndInodeNumber[fileName]=inodeNumber;
		}
		if(dirLen+totalBytesUsed==totalSize)
			break;
		totalBytesUsed+=dirLen;
	    inodeNumber=Bit2Int32(allBytesOfDirTab,totalBytesUsed);

	}
}
void Parse_image::Parse_fileInode(const string& filePath,UInt32 inodeNumber)
{
	UInt32 posOfBGDInListOfBGDS = (inodeNumber - 1) / superblock->inodes_per_group;  //inode所在的块组
	//该块组的i-节点表起始块号 * 块的大小 + Inode在group table中的offset * inode大小 entry（256）
	Int64 offset = userdataPos + ((Int64)(vecOfBGDS[(int)posOfBGDInListOfBGDS].bg_inode_table) * superblock->block_size_in_byte) + (Int64)(((inodeNumber - 1) % superblock->inodes_per_group) *Inode::sizeOfInode);
	_fseeki64(fp,offset,SEEK_SET);
	byte bytesOfInode[Inode::sizeOfInode];
	fread(bytesOfInode,Inode::sizeOfInode,1,fp);
	Inode inode(bytesOfInode);
	if(!inode.isaDirectory&&!inode.isRegularFile)
		return ;
	if(inode.enableExtentFeature)
	{
		vector<Ext4_extent> extents;
		traversalExtentTree(inode.bytesOfExtentArea,extents);
		if(inode.isRegularFile)
		{
			if(!fileNamesOfContents.count(filePath))
				fileNamesOfContents[filePath]=make_pair(inode.lenOfFile,extents);
		}
		else
		{
			byte *allBytesOfDirTab=nullptr;
			UInt32 totalSize=0;
			for(auto extent:extents)
			{
				totalSize+=extent.totalCoveredBlockByThisExtent*superblock->block_size_in_byte;
			}
			allBytesOfDirTab=(byte*)malloc(totalSize);
			readAllBytesOfDirTab(allBytesOfDirTab,extents);
			unordered_map<string,UInt32> hashFileAndInodeNumber;
			dealContentOfDir(hashFileAndInodeNumber,allBytesOfDirTab,totalSize);
			free(allBytesOfDirTab);
			for(auto p:hashFileAndInodeNumber)
			{
				Parse_fileInode(filePath+"\/"+p.first,p.second);
			}
		}

	}

}
Parse_image::Parse_image(Int64 udPos)
{
	fp=nullptr;
	superblock=nullptr;
	userdataPos=udPos;
}
Parse_image::~Parse_image()
{
	if(fp)
		fclose(fp);
	if(superblock)
		delete superblock;
}
int string_replace(string &s1, const string &s2, const string &s3)
{
	string::size_type pos = 0;
	string::size_type a = s2.size();
	string::size_type b = s3.size();
	while ((pos = s1.find(s2, pos)) != string::npos)
	{
		s1.replace(pos, a, s3);
		pos += b;
	}
	return 0;
}

bool Parse_image::startParse(const string &filePath,const string &configPath)
{
	fp=fopen(filePath.c_str(),"rb+");
	if(fp==nullptr)
		return false;
	storeDir=filePath.substr(0,filePath.find_last_of('.'));
	CreateDir(storeDir.c_str());
	initSuperBlock();
	initVecOfBGDS();
	Parse_fileInode("",2);
	for(auto p:fileNamesOfContents)
	{
		if(p.first.find("/data/")!=string::npos)
		{
			string filePath=p.first;
			string_replace(filePath,"\/","\\");
			filePath=storeDir+"\\"+filePath;
			string dirPath=filePath.substr(0,filePath.find_last_of('\\'));
			CreateDir(dirPath.c_str());
			int fileSize=p.second.first;
			FILE *wfp=fopen(filePath.c_str(),"wb+");
			if(wfp==nullptr)
				continue;
			for(auto extent:p.second.second)
			{
				UInt64 offset=extent.firstBlockNumberInThisExtent*superblock->block_size_in_byte+userdataPos;
				UInt64 totalSize=extent.totalCoveredBlockByThisExtent*superblock->block_size_in_byte;
				_fseeki64(fp,offset,SEEK_SET);
				byte * Bytes=(byte*)malloc(totalSize);
				fread(Bytes,totalSize,1,fp);
				if(fileSize<totalSize)
				{
					fwrite(Bytes,fileSize,1,wfp);
					break;
				}
				fwrite(Bytes,totalSize,1,wfp);
				fileSize-=totalSize;
			}
			fclose(wfp);
		}
	}
	cout<<"inode parse end"<<endl;
}



SuperBlock::SuperBlock(byte *Bytes)
{
	total_inode_count=Bit2UInt32(Bytes,0x0);
	total_block_count=Bit2UInt32(Bytes,0x4);
	free_block_count=Bit2UInt32(Bytes,0xc);
	free_inode_count=Bit2Int32(Bytes,0x10);
	block_size_in_byte=pow(2,10+Bit2UInt32(Bytes,0x18));
	cluster_size_in_byte=pow(2,Bit2UInt32(Bytes,0x1C));
	blocks_per_group=Bit2UInt32(Bytes,0x20);
	clusters_per_group=Bit2UInt32(Bytes,0x24);
	inodes_per_group=Bit2UInt32(Bytes,0x28);
}
BlockGroupDescriptor::BlockGroupDescriptor(byte *Bytes)
{
	bg_block_bitmap=Bit2UInt32(Bytes,0x0);
	bg_inode_bitmap=Bit2UInt32(Bytes,0x4);
	bg_inode_table=Bit2UInt32(Bytes,0x8);
	bg_free_blocks_count=Bit2UInt16(Bytes,0xC);
	bg_free_inodes_count=Bit2UInt16(Bytes,0xE);
	bg_used_dirs_count=Bit2UInt16(Bytes,0x10);
}
Inode::Inode(byte *Bytes):posOfExtentTreeAreaInInode(0x28),lenOfExtentTreeAreaInInode(60)
{
	isRegularFile=(Bytes[1]&0xF0)==0x80;
	isaDirectory=(Bytes[1]&0xF0)==0x40;
	enableInlineDataFeature=(Bytes[0x23]&0x10)==0x10;
	enableExtentFeature=(Bytes[0x22]&0x08)==0x08;
	enableHashTreeDirectoriesFeature=(Bytes[0x21]&0x10)==0x10;

	lenOfFile=Bit2UInt32(Bytes,4);
	bytesOfExtentArea=(byte*)malloc(lenOfExtentTreeAreaInInode);
	memcpy(bytesOfExtentArea,Bytes+posOfExtentTreeAreaInInode,lenOfExtentTreeAreaInInode);

}
Inode::~Inode()
{
	if(bytesOfExtentArea!=nullptr)
		free(bytesOfExtentArea);
}

Ext4_extent_header::Ext4_extent_header(byte* Bytes):magicNumber(0xF30A)
{
	eh_entries = Bit2UInt16(Bytes, 0x2);
	eh_max = Bit2UInt16(Bytes, 0x4);
	eh_depth =Bit2UInt16(Bytes, 0x6);
}
Ext4_extent::Ext4_extent(byte* Bytes)
{
	filePointer =Bit2UInt32(Bytes, 0x0);
	totalCoveredBlockByThisExtent = Bit2UInt16(Bytes, 0x4);

	ee_start_hi =Bit2UInt16(Bytes, 0x6);
	ee_start_lo = Bit2UInt32(Bytes, 0x8);
	UInt64 tempLong = ee_start_hi;
	tempLong <<= 32;
	firstBlockNumberInThisExtent = tempLong + ee_start_lo;
}
Ext4_extent_idx::Ext4_extent_idx(byte * Bytes)
{
	ei_block =Bit2UInt32(Bytes, 0x0);
	ei_leaf_lo = Bit2UInt32(Bytes, 0x4);
	ei_leaf_hi = Bit2UInt16(Bytes, 0x8);
	UInt64 tempLong = ei_leaf_hi;
	tempLong <<= 32;
	ei_leaf = tempLong + ei_leaf_lo;
}
