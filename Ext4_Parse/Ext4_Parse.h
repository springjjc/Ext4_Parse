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
	UInt32 bg_block_bitmap;//location of block bitmap.  �ÿ���Ŀ�λͼ����ʼ���
	UInt32 bg_inode_bitmap;//location of inode bitmap. �ÿ����i-λͼ��ʼ���
	UInt32 bg_inode_table;//location of inode table �ÿ����i-�ڵ����ʼ���
	UInt16 bg_free_blocks_count;//free block count. �ÿ���Ŀ��п���
	UInt16 bg_free_inodes_count;//free inode count. �ÿ���Ŀ���i-�ڵ���
	UInt16 bg_used_dirs_count;//directory count.    �ÿ����Ŀ¼����
	BlockGroupDescriptor(byte* Bytes);
};
class SuperBlock
{
public:
	UInt32 total_inode_count;//i-�ڵ�����
	UInt32 total_block_count;//�ܿ���
	UInt32 free_block_count ;//���п���
	UInt32 free_inode_count;//����i-�ڵ���
	UInt32 block_size_in_byte ;//���С
	UInt32 cluster_size_in_byte;//�δ�С
	UInt32 blocks_per_group ;//ÿ������Ŀ���
	UInt32 clusters_per_group ;//ÿ������Ķ���
	UInt32 inodes_per_group ;//ÿ�������i-�ڵ���
	SuperBlock(byte *Bytes);

};
class Ext4_extent
{
public:
	UInt32 filePointer;//�ö���ʼblock���ļ�������ͼ�е�ָ�루Ҳ��Block Number��ʾ��
	UInt16 totalCoveredBlockByThisExtent;//��Extent�ܹ�������blocK����
	UInt64 firstBlockNumberInThisExtent;//�ö���ʼblock���ļ�ϵͳ��ͼ�е�ָ�루��������block number��

	UInt16 ee_start_hi;//�������ݿ�ĸ�16λ
	UInt32 ee_start_lo;//�������ݿ�ĵ�32λ
	Ext4_extent(byte* Bytes);
};
class Ext4_extent_header
{
public:
	Ext4_extent_header(byte* Bytes);
public:
	const UInt16 magicNumber;//��Ϊ�ж��ǲ���inline data�ı�ʶ
    UInt16 eh_entries;//Number of valid entries following the header ��Ч��ĸ���
    UInt16 eh_max;//Maximum number of entries that could follow the header. ���еĴ洢����
    UInt16 eh_depth;//Depth of this extent node in the extent tree. 0 = this extent node points to data blocks; otherwise, this extent node points to other extent nodes

};
class Ext4_extent_idx
{
public:
	Ext4_extent_idx(byte * Bytes);
public:
	UInt32 ei_block;// 	This index node covers file blocks from 'block' onward.������0������ʲô��block������
    UInt64 ei_leaf;//the block number of the extent node that is the next level lower in the tree. The tree node pointed to can be either another internal node or a leaf node, described below.
private :
	UInt32 ei_leaf_lo;//Lower 32-bits of the block number of the extent node that is the next level lower in the tree. The tree node pointed to can be either another internal node or a leaf node, described below.
    UInt16 ei_leaf_hi;//Upper 16-bits of the previous field. 
        //��������Ӧ������Ϊͬһ��
       
};
class Inode
{
public :
	static const UInt16 sizeOfInode=256;
	bool isRegularFile;//�����ļ�
	bool isaDirectory;//Ŀ¼
	bool enableInlineDataFeature;//�ļ����������٣�ֱ�Ӵ洢��inode��extent���������ļ�����ֱ�Ӻ���
	bool enableExtentFeature;
	bool enableHashTreeDirectoriesFeature;//Ŀ¼�ļ������ݴ洢��ʽ���֣�Linear (Classic) Directories �� Hash Tree Directories 
	UInt32 lenOfFile;
	byte *bytesOfExtentArea;
	Inode(byte * Bytes);
	~Inode();
private:
	const UInt16 posOfExtentTreeAreaInInode;//��һ��ֱ�ӿ�ָ��
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