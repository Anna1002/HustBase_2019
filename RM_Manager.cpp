//
// File:        RM_Manger.cpp
// Description: A implementation of RM_Manager.h to handle RM file
// Authors:     dim dew
//

#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"


RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//��ʼ��ɨ��
{

	return SUCCESS;
}

RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{
	return SUCCESS;
}

RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	return SUCCESS;
}

RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
	return SUCCESS;
}

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	return SUCCESS;
}

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	return SUCCESS;
}

// 
// Ŀ��: ��ȡ�ļ���� fd, ���� RM ����ҳ (�� 1 ҳ)
// 1. ���� recordSize �����ÿһҳ���Է��õļ�¼����
// 2. ���� PF_Manager::CreateFile() �� Paged File ����ؿ�����Ϣ���г�ʼ��
// 3. ���� PF_Manager::OpenFile() �򿪸��ļ� ��ȡ PF_FileHandle
// 4. ͨ���� PF_FileHandle �� AllocatePage ���������ڴ滺���� �õ��� 1 ҳ�� pData ָ��
// 5. ��ʼ��һ�� RM_FileHdr �ṹ��д�� pData ָ����ڴ���
// 6. ��� �� 1 ҳ Ϊ��
// 7. PF_Manager::CloseFile() ������ ForceAllPage
//
RC RM_CreateFile (char *fileName, int recordSize)
{
	int maxRecordsNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	RM_FileHdr* rmFileHdr;
	// ��� recordSize �Ƿ�Ϸ� 
	// (���ڲ�������ķ�ʽ������Ҫ�ڼ�¼֮ǰ���� nextFreeSlot �ֶ����Լ��� sizeof(int))
	if (recordSize + sizeof(int) > PF_PAGE_SIZE)
		return RM_RECSIZETOOBIG;

	// 1. ����ÿһҳ���Է��õļ�¼����
	maxRecordsNum = ( ( PF_PAGE_SIZE - sizeof(RM_PageHdr) ) << 3 ) / ( (recordSize << 3) + 33 );
	if ((((maxRecordsNum + 7) >> 3) +
		((recordSize + 4) << 3) +
		sizeof(RM_PageHdr)) > PF_PAGE_SIZE)
		maxRecordsNum--;
	maxRecordsNum = ((maxRecordsNum + 7) >> 3) << 3;

	// 2. ���� PF_Manager::CreateFile() �� Paged File ����ؿ�����Ϣ���г�ʼ��
	// 3. ���� PF_Manager::OpenFile() �򿪸��ļ� ��ȡ PF_FileHandle
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile(fileName, &pfFileHandle)))
		return rc;

	// 4. ͨ���� PF_FileHandle �� AllocatePage ���������ڴ滺���� �õ��� 1 ҳ�� pData ָ��
	if ((rc = AllocatePage(&pfFileHandle, &pfPageHandle)))
		return rc;

	rmFileHdr = (RM_FileHdr*)(pfPageHandle.pFrame->page.pData);

	// 5. ��ʼ��һ�� RM_FileHdr �ṹ��д�� pData ָ����ڴ���
	rmFileHdr->firstFreePage = RM_NO_MORE_FREE_PAGE;
	rmFileHdr->recordSize = recordSize;
	rmFileHdr->recordsPerPage = maxRecordsNum;
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + (maxRecordsNum >> 3);

	// 6. ��� �� 1 ҳ Ϊ��
	// 7. PF_Manager::CloseFile() ������ ForceAllPage
	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)) || 
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}

//Ŀ�ģ���ȡ�ļ�������RM_FileHandle
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	RM_FileHdr *rm_FileHdr;
    if((rc=openFile(fileName,&fileHandle->pfFileHandle)) ||
		rc=GetThisPage(&fileHandle->pfFileHandle,1,&pfPageHandle))
		return rc;
	fileHandle->bOpen=true;
	rm_FileHdr = (RM_FileHdr*)pfPageHandle.pFrame->page.pData;
	
	fileHandle->rmFileHdr = *rm_FileHdr;

	return SUCCESS;
}

RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	return SUCCESS;
}
