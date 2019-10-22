//
// File:        RM_Manger.cpp
// Description: A implementation of RM_Manager.h to handle RM file
// Authors:     dim dew
//

#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"
#include <cassert>
#include <string.h>

bool getBit (char *bitMap, int bitIdx)
{
	return ((bitMap[bitIdx >> 3] >> (bitIdx & 0x7)) & 1) == 1;
}

void setBit(char* bitMap, int bitIdx, bool val)
{
	if (val)
		bitMap[bitIdx >> 3] |= (1 << (bitIdx & 0x7));
	else
		bitMap[bitIdx >> 3] &= (~(1 << (bitIdx & 0x7)));
}

RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//��ʼ��ɨ��
{

	return SUCCESS;
}

RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{

	return SUCCESS;
}

//
// Ŀ��: ���ݸ����� rid, ��ȡ��Ӧ���ļ���¼���浽 rec ָ��ĵ�ַ
// 1. ͨ�� rid ���ָ���� pageNum �� slotNum
// 2. ͨ�� PF_FileHandle::GetThisPage() ��� pageNum ҳ
// 3. ��ø�ҳ�ϵ� PageHdr ��Ϣ, ��λ��¼������ʼ��ַ
// 4. ���� slotNum ���Ƿ��ǿ��õ�
// 5. ���÷��ص� rec
//
RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	RC rc;
	PF_PageHandle pfPageHandle;
	RM_PageHdr rmPageHdr;
	char *data;
	char *records;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	rec->bValid = false;
	rec->pData = NULL;
	// 1. ͨ�� rid ���ָ���� pageNum �� slotNum
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;

	if (pageNum < 2)
		return RM_INVALIDRID;

	// 2. ͨ�� PF_FileHandle::GetThisPage() ��� pageNum ҳ
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. ��ø�ҳ�ϵ� PageHdr ��Ϣ, ��λ��¼������ʼ��ַ
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmPageHdr = *((RM_PageHdr*)data);
	records = data + fileHandle->rmFileHdr.slotsOffset;

	// 4. ���� slotNum ���Ƿ��ǿ��õ�
	if (!getBit(rmPageHdr.slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. ���÷��ص� rec
	rec->bValid = true;
	rec->pData = records + (fileHandle->rmFileHdr.recordSize + sizeof(int)) * slotNum + sizeof(int);
	rec->rid = *rid;

	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

//
// Ŀ��: ���ļ���д���¼��
// 1. ͨ�� RM_FileHandle �� RM_FileHdr �е� firstFreePage ��õ�һ�����õ� pageNum
// ��� pageNum != RM_NO_MORE_FREE_PAGE
//    1.1 ͨ�� GetThisPage() ��ø�ҳ pageHdr �ĵ�ַ
//    1.2 ͨ�� pageHdr �е� firstFreeSlot  
//    1.3 д�룬����࣬���� freeSlot ��, �޸�bitmap (������ pageHdr �� firstFreeSlot)
//    1.4 �����ʱ ֮ǰ�� slot λ�õ� nextSlot �� RM_NO_MORE_FREE_SLOT
//          1.4.1 ��ô FileHandle �� RM_FileHdr �� firstFreePage ͬ�������޸� ά�� FreePage ��
//          1.4.2 ά��֮�� ���� FileHdr Ϊ��
// ��� pageNum == RM_NO_MORE_FREE_PAGE
//    2.1 ��� ���һҳ �� pageNum�� GetThisPage���ڴ滺����
//    2.2 ��ø�ҳ PageHdr �е� slotNum, ��� slotNum ��δ�ﵽ�������ƣ������ļ�����д���µļ�¼
//    2.3 ��� slotNum �����ﵽ�� recordsPerPage
//          2.3.1 ͨ�� AllocatePage �򻺳��������µ�ҳ��
//          2.3.2 ��ʼ�� PageHdr
//          2.3.3 д�룬���� bitmap, ����slotCount�������� ��ҳ Ϊ��
//
RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
	RC rc;
	PageNum firstFreePage;
	SlotNum firstFreeSlot;
	PF_PageHandle pfPageHandle;
	RM_PageHdr* rmPageHdr;
	char* data;
	char* records;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	firstFreePage = fileHandle->rmFileHdr.firstFreePage;

	if (firstFreePage != RM_NO_MORE_FREE_PAGE) {
		//    1.1 ͨ�� GetThisPage() ��ø�ҳ pageHdr �ĵ�ַ
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), firstFreePage, &pfPageHandle)) || 
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

		//    1.2 ͨ�� pageHdr �е� firstFreeSlot  
		rmPageHdr = (RM_PageHdr*)data;
		firstFreeSlot = rmPageHdr->firstFreeSlot;

		records = data + fileHandle->rmFileHdr.slotsOffset;

		//    1.3 д�룬����࣬���� freeSlot �� (������ pageHdr �� firstFreeSlot)
		memcpy(records + (fileHandle->rmFileHdr.recordSize + sizeof(int)) * firstFreeSlot + sizeof(int),
			pData, fileHandle->rmFileHdr.recordSize);
		rmPageHdr->firstFreeSlot = *((int*)(records + (fileHandle->rmFileHdr.recordSize + sizeof(int)) * firstFreeSlot));
		
		if ((rc))
	} else {

	}

	return SUCCESS;
}

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	return SUCCESS;
}

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

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
	char* data;
	// ��� recordSize �Ƿ�Ϸ� 
	// (���ڲ�������ķ�ʽ������Ҫ�ڼ�¼֮ǰ���� nextFreeSlot �ֶ����Լ��� sizeof(int))
	if (recordSize + sizeof(int) > PF_PAGE_SIZE || 
		recordSize < 0)
		return RM_INVALIDRECSIZE;

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

	assert(pfPageHandle.pFrame->page.pageNum == 1);
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmFileHdr = (RM_FileHdr*)data;

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

// 
// Ŀ��: ���ļ������� RM_fileHandle
// 1. ���� PF_Manager::OpenFile�����һ�� PF_FileHandle
// 2. ͨ�� PF_FileHandle ����ȡ1��ҳ���ϵ� RM_FileHdr ��Ϣ
// 3. ��ʼ�� RM_FileHandle ��Ա
//
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	char* data;

	if (fileHandle->bOpen)
		return RM_FHOPENNED;

	// 1. ���� PF_Manager::OpenFile�����һ�� PF_FileHandle
	// 2. ͨ�� PF_FileHandle ����ȡ1��ҳ���ϵ� RM_FileHdr ��Ϣ
	if ((rc = openFile(fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle)))
		return rc;

	// 3. ��ʼ�� RM_FileHandle ��Ա
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	fileHandle->rmFileHdr = *((RM_FileHdr*)data);

	// 4. �ͷ��ڴ滺����
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}


// 
// Ŀ��: �ر��ļ����ж� RM_FileHdr �Ƿ��޸�
// ����������޸ģ� GetThisPage���ڴ滺������д���ҳ, ���Ϊ��֮��
// ���� PF_Manager::CloseFile() �ر� RM �ļ�
//
RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	// �ж��Ƿ������޸�
	if (fileHandle->isRMHdrDirty) {
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

		// ��Ҫ����
		memcpy(data, (char*) & (fileHandle->rmFileHdr), sizeof(RM_FileHdr));
		
		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

	// ���� PF_Manager::CloseFile() �ر� RM �ļ�
	if ((rc = CloseFile(&(fileHandle->pfFileHandle))))
		return rc;

	fileHandle->bOpen = false;
	fileHandle->isRMHdrDirty = false;

	return SUCCESS;
}
