//
// File:       IX_Manager.h
// Desciption: An interface defination of Index File Manager
// Authors:    dim dew
//

#ifndef IX_MANAGER_H_H
#define IX_MANAGER_H_H

#include "RM_Manager.h"
#include "PF_Manager.h"

const int IX_NO_MORE_BUCKET_SLOT = -1;  
const int IX_USELESS_SLOTNUM = -2;      // �ڲ��ڵ���ֻ��Ҫ�� pagenum
const int IX_NULL_CHILD = -3;
const int IX_NO_MORE_NEXT_LEAF = -4;

typedef struct{
	int attrLength;
	// int keyLength;
	AttrType attrType;
	PageNum rootPage;
	PageNum first_leaf;
	int order;                    // һ��Node���ܹ�������key��������ΧΪ [ (order - 1) / 2, order - 1 ]

	// һЩ offset ֵ�ı��棬�������ļ��н��ж�λ
	int nodeEntryListOffset;
	int bucketEntryListOffset;

	int nodeKeyListOffset;
	int entrysPerBucket;
} IX_FileHeader;

typedef struct{
	bool bOpen;
	bool isHdrDirty;
	PF_FileHandle fileHandle;
	IX_FileHeader fileHeader;
} IX_IndexHandle;

typedef struct {
	int is_leaf;                   // �ڵ�����
	int keynum;                    // Ŀǰ���е� keys ������
	// PageNum parent;             // �����е���û�б�Ҫ                
	PageNum sibling;               // Ҷ�ӽڵ���ֵܽڵ�
	PageNum firstChild;            // �ڲ��ڵ�ĵ�һ���ӽڵ�
} IX_NodePageHeader; 

typedef struct {
	IX_NodePageHeader* nodeHdr;    // ָ���ڴ滺�����ڵ� NodePageHeader, ���ڸ�������

	char* keys;                    // ָ���ڴ滺�����ڵ� NodePage �� �ؼ��� ������
	RID* rids;                     // ָ���ڴ滺�����ڵ� NodePage �� ָ����
} IX_Node;

typedef struct {
	SlotNum slotNum;
	SlotNum firstValidSlot;
	SlotNum firstFreeSlot;
	PageNum nextBucket;
} IX_BucketPageHeader;

typedef struct{
	bool bOpen;		                               /*ɨ���Ƿ�� */
	IX_IndexHandle *pIXIndexHandle;	               //ָ�������ļ�������ָ��
	CompOp compOp;                                 /* ���ڱȽϵĲ�����*/
	char *value;		                           /* �������бȽϵ�ֵ */
    PF_PageHandle pfPageHandles[PF_BUFFER_SIZE];   // �̶��ڻ�����ҳ������Ӧ��ҳ������б�
	PageNum pnNext; 	                           //��һ����Ҫ�������ҳ���
} IX_IndexScan;

typedef struct Tree_Node{
	int  keyNum;		//�ڵ��а����Ĺؼ��֣�����ֵ������
	char  **keys;		//�ڵ��а����Ĺؼ��֣�����ֵ������
	Tree_Node  *parent;	//���ڵ�
	Tree_Node  *sibling;	//�ұߵ��ֵܽڵ�
	Tree_Node  *firstChild;	//����ߵĺ��ӽڵ�
} Tree_Node; //�ڵ����ݽṹ

typedef struct{
	AttrType  attrType;	//B+����Ӧ���Ե���������
	int  attrLength;	//B+����Ӧ����ֵ�ĳ���
	int  order;			//B+��������
	Tree_Node  *root;	//B+���ĸ��ڵ�
} Tree;

RC CreateIndex(const char * fileName,AttrType attrType,int attrLength);
RC OpenIndex(const char *fileName,IX_IndexHandle *indexHandle);
RC CloseIndex(IX_IndexHandle *indexHandle);

RC InsertEntry(IX_IndexHandle *indexHandle,void *pData,const RID * rid);
RC DeleteEntry(IX_IndexHandle *indexHandle,void *pData,const RID * rid);
RC OpenIndexScan(IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value);
RC IX_GetNextEntry(IX_IndexScan *indexScan,RID * rid);
RC CloseIndexScan(IX_IndexScan *indexScan);
// RC GetIndexTree(char *fileName, Tree *index);

#endif