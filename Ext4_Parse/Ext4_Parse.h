#pragma once
#include<string>
#include<unordered_map>
#include<iostream>
#include<vector>
using namespace std;
#define DLL_EXPORT __declspec(dllexport)
typedef unsigned char byte;
typedef __int64 Int64;
typedef __int32 Int32;
typedef __int16 Int16;
typedef unsigned __int64 UInt64;
typedef unsigned __int32 UInt32;
typedef unsigned __int16 UInt16;
int reverse_bytes(byte *p, char c);
void CreateDir( const char *dir );
class DLL_EXPORT PartitionTableInfo
{
	class PartitionInfo
	{
	public:
		Int64 partitionBegin;
		Int64 partitionEnd;
		Int64 partitionLen;
		string partitionName;

	};
public:
	unordered_map<string,PartitionInfo> hashPartition;
	PartitionTableInfo(string imgPath);

};
class BlockGroupDescriptor
{
public:
	UInt32 bg_block_bitmap;//location of block bitmap.  该块组的块位图的起始块号
	UInt32 bg_inode_bitmap;//location of inode bitmap. 该块组的i-位图起始块号
	UInt32 bg_inode_table;//location of inode table 该块组的i-节点表起始块号
	UInt16 bg_free_blocks_count;//free block count. 该块组的空闲块数
	UInt16 bg_free_inodes_count;//free inode count. 该块组的空闲i-节点数
	UInt16 bg_used_dirs_count;//directory count.    该块组的目录总数
	BlockGroupDescriptor(byte* Bytes);
};
class SuperBlock
{
public:
	UInt32 total_inode_count;//i-节点总数
	UInt32 total_block_count;//总块数
	UInt32 free_block_count ;//空闲块数
	UInt32 free_inode_count;//空闲i-节点数
	UInt32 block_size_in_byte ;//块大小
	UInt32 cluster_size_in_byte;//段大小
	UInt32 blocks_per_group ;//每块包含的块数
	UInt32 clusters_per_group ;//每块包含的段数
	UInt32 inodes_per_group ;//每块包含的i-节点数
	SuperBlock(byte *Bytes);

};
class Ext4_extent
{
public:
	UInt32 filePointer;//该段起始block在文件内容视图中的指针（也用Block Number表示）
	UInt16 totalCoveredBlockByThisExtent;//该Extent总共包含的blocK数量
	UInt64 firstBlockNumberInThisExtent;//该段起始block在文件系统视图中的指针（即正常的block number）

	UInt16 ee_start_hi;//物理数据块的高16位
	UInt32 ee_start_lo;//物理数据块的低32位
	Ext4_extent(byte* Bytes);
};
class Ext4_extent_header
{
public:
	Ext4_extent_header(byte* Bytes);
public:
	const UInt16 magicNumber;//作为判断是不是inline data的标识
    UInt16 eh_entries;//Number of valid entries following the header 有效项的个数
    UInt16 eh_max;//Maximum number of entries that could follow the header. 项中的存储容量
    UInt16 eh_depth;//Depth of this extent node in the extent tree. 0 = this extent node points to data blocks; otherwise, this extent node points to other extent nodes

};
class Ext4_extent_idx
{
public:
	Ext4_extent_idx(byte * Bytes);
public:
	UInt32 ei_block;// 	This index node covers file blocks from 'block' onward.经常是0，代表什么？block数量？
    UInt64 ei_leaf;//the block number of the extent node that is the next level lower in the tree. The tree node pointed to can be either another internal node or a leaf node, described below.
private :
	UInt32 ei_leaf_lo;//Lower 32-bits of the block number of the extent node that is the next level lower in the tree. The tree node pointed to can be either another internal node or a leaf node, described below.
    UInt16 ei_leaf_hi;//Upper 16-bits of the previous field. 
        //上面两项应该整合为同一项
       
};
class Inode
{
public :
	static const UInt16 sizeOfInode=256;
	bool isRegularFile;//正常文件
	bool isaDirectory;//目录
	bool enableInlineDataFeature;//文件数据量过少，直接存储在inode的extent区域，这种文件可以直接忽略
	bool enableExtentFeature;
	bool enableHashTreeDirectoriesFeature;//目录文件的内容存储方式两种：Linear (Classic) Directories 和 Hash Tree Directories 
	UInt32 lenOfFile;
	byte *bytesOfExtentArea;
	Inode(byte * Bytes);
	~Inode();
private:
	const UInt16 posOfExtentTreeAreaInInode;//第一个直接块指针
	const UInt16 lenOfExtentTreeAreaInInode;

};
typedef pair<int,vector<Ext4_extent>> pairExtents;


class DLL_EXPORT Parse_image
{
private:
	FILE *fp;
	Int64 userdataPos;
	vector<BlockGroupDescriptor> vecOfBGDS;
	SuperBlock *superblock;
	unordered_map<string,pairExtents> fileNamesOfContents;
	unordered_map<string,vector<string>> pkgNameAndPath;
	string storeDir;
public:
	bool startParse(const string &filePath,const string &configPath);
	Parse_image(Int64 udPos=0);
	~Parse_image();
private:
	void initSuperBlock();
	void initVecOfBGDS();
	void Parse_fileInode(const string& filePath,UInt32 inodeNumber);
	void traversalExtentTree(byte *bytesOfExtentArea,vector<Ext4_extent> &extents);
	void readAllBytesOfDirTab(byte *allBytesOfDirTab,vector<Ext4_extent> &extents);
	void dealContentOfDir(unordered_map<string,UInt32> &hashFileAndInodeNumber,byte *allBytesOfDirTab,UInt32 totalSize);
};