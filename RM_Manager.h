//
// File:        RM_Manager.h
// Description: A interface for RM File
// Authors:     dim dew
//

#ifndef RM_MANAGER_H_H
#define RM_MANAGER_H_H

#include "PF_Manager.h"
#include "str.h"

#define WHICH_REC(recordSize, records, x) ((records) + ((recordSize) + sizeof(int)) * x + sizeof(int))
#define REC_NEXT_SLOT(recordSize, records, x) ((records) + ((recordSize) + sizeof(int)) * x)

const int RM_NO_MORE_FREE_PAGE = -1;
const int RM_NO_MORE_FREE_SLOT = -2;

typedef int SlotNum;

typedef struct {	
	PageNum pageNum;	       // ��¼����ҳ��ҳ��
	SlotNum slotNum;		   // ��¼�Ĳ�ۺ�
} RID;

typedef struct{
	bool bValid;		       // False��ʾ��δ�������¼
	RID  rid; 		           // ��¼�ı�ʶ�� 
	char *pData; 		       // ��¼���洢������ 
} RM_Record;


typedef struct
{
	int bLhsIsAttr,bRhsIsAttr;//���������ԣ�1������ֵ��0��
	AttrType attrType;
	int LattrLength,RattrLength;
	int LattrOffset,RattrOffset;
	CompOp compOp;
	void *Lvalue,*Rvalue;
} Con;

typedef struct {
	int nextFreePage;          // �� RM_FileHdr ����һ�� freePage ��
	int firstFreeSlot;         // �� 1 �� free slot
	int slotCount;             // �Ѿ������ slot ���� ����ɾ��֮��Ŀ� slot
	char slotBitMap[0];        // ��� slot �Ƿ�ʹ�õ� bitMap���˴�����Ϊ��ַ�� (������ʵ�ʷ���ռ�)
} RM_PageHdr;

typedef struct {
	int recordSize;             // һ����¼�Ĵ�С
	int recordsPerPage;			// ÿһҳ�п��Դ�ŵ�record����
	int firstFreePage;          // ��һ�����Է��¼�¼��page��� (>= 2)
	int slotsOffset;            // ÿ��ҳ���м�¼������� PF_Page.pData ��ƫ��
	// Ϊ�˼��� FileHdr ��ˢд���ﲻά���ļ��м�¼�ĸ���
} RM_FileHdr;

typedef struct{                 // �ļ����
	bool bOpen;
	PF_FileHandle pfFileHandle; // ���� PF_FileHandle
	RM_FileHdr rmFileHdr;       
	bool isRMHdrDirty;          // �� 1 ҳ�Ƿ���
} RM_FileHandle;

typedef struct{
	bool  bOpen;		//ɨ���Ƿ�� 
	RM_FileHandle  *pRMFileHandle;		//ɨ��ļ�¼�ļ����
	int  conNum;		//ɨ���漰���������� 
	Con  *conditions;	//ɨ���漰����������ָ��
    PF_PageHandle  PageHandle; //�����е�ҳ����
	PageNum  pn; 	//ɨ�輴�������ҳ���
	SlotNum  sn;		//ɨ�輴������Ĳ�ۺ�
}RM_FileScan;



RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec);

RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions);

RC CloseScan(RM_FileScan *rmFileScan);

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec);

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid);

RC InsertRec (RM_FileHandle *fileHandle, char *pData, RID *rid); 

RC GetRec (RM_FileHandle *fileHandle, RID *rid, RM_Record *rec); 

RC RM_CloseFile (RM_FileHandle *fileHandle);

RC RM_OpenFile (char *fileName, RM_FileHandle *fileHandle);

RC RM_CreateFile (char *fileName, int recordSize);



#endif