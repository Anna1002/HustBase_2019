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
#include <iostream>

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

int cmp(AttrType attrType, void *ldata, void *rdata, int llen, int rlen)
{
	switch (attrType) 
	{
	case chars:
	{
		char* l = (char*)ldata;
		char* r = (char*)rdata;
		int min_len = MIN(llen, rlen);
		int cmp_res = strncmp(l, r, min_len);
		if (cmp_res == 0)
			cmp_res = (llen > rlen) ? 1 : (llen == rlen) ? 0 : -1;
		return cmp_res;
	}
	case ints:
	{
		int l = *(int*)ldata;
		int r = *(int*)rdata;
		return (l > r) ? 1 : (l == r) ? 0 : -1;
	}
	case floats:
	{
		float l = *(float*)ldata;
		float r = *(float*)rdata;
		return (l > r) ? 1 : (l == r) ? 0 : -1;
	}
	}
}

bool matchComp (int cmp_res, CompOp compOp)
{
	switch (compOp)
	{
	case EQual:
		return cmp_res == 0;
	case LEqual:      
		return cmp_res <= 0;
	case NEqual:    
		return cmp_res != 0;
	case LessT:
		return cmp_res < 0;
	case GEqual:  
		return cmp_res >= 0;
	case GreatT:
		return cmp_res > 0;
	case NO_OP:
		return true;
	}
}

bool innerCmp(Con condition, char *pData)
{
	void* lval, * rval;

	lval = (condition.bLhsIsAttr == 1) ? (pData + condition.LattrOffset) : condition.Lvalue;
	rval = (condition.bRhsIsAttr == 1) ? (pData + condition.RattrOffset) : condition.Rvalue;

	return matchComp(cmp(condition.attrType, lval, rval, 
			condition.LattrLength, condition.RattrLength), condition.compOp);
}

//
// ��ʼ�� RM_FileScan �ṹ
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//��ʼ��ɨ��
{
	RC rc;

	if ((rmFileScan->bOpen))
		return RM_FSOPEN;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	rmFileScan->bOpen = true;
	rmFileScan->conditions = conditions;
	rmFileScan->conNum = conNum;
	rmFileScan->pRMFileHandle = fileHandle;
	rmFileScan->pn = 2;
	rmFileScan->sn = 0;

	//if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 2, &(rmFileScan->PageHandle)))) {
	//	if (rc == PF_INVALIDPAGENUM)
	//		return RM_NOMORERECINMEM;
	//	return rc;
	//}

	return SUCCESS;
}

//
// Ŀ��: 
//
RC CloseScan(RM_FileScan* rmFileScan)
{
	if (!rmFileScan->bOpen)
		return RM_FSCLOSED;

	rmFileScan->bOpen = false;
	return SUCCESS;
}

//
// Ŀ��: 
//
RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{
	RC rc;
	PageNum lastPageNum;
	PageNum nextPageNum;
	SlotNum nextSlotNum;
	PF_PageHandle pfPageHandle;
	RM_PageHdr* rmPageHdr;
	char* data;
	char* records;
	int recordSize = rmFileScan->pRMFileHandle->rmFileHdr.recordSize;

	if ((rc = GetLastPageNum(&(rmFileScan->pRMFileHandle->pfFileHandle), &lastPageNum)))
		return rc;

	nextPageNum = rmFileScan->pn;
	nextSlotNum = rmFileScan->sn;

	for (; nextPageNum <= lastPageNum; nextPageNum++) {
		std::cout << "============================== Page " << nextPageNum << "================================" << std::endl;
		if ((rc = GetThisPage(&(rmFileScan->pRMFileHandle->pfFileHandle), nextPageNum, &pfPageHandle)))
		{
			if (rc == PF_INVALIDPAGENUM)
				continue;
			return rc;
		}

		if ((rc = GetData(&pfPageHandle, &data)))
			return rc;

		rmPageHdr = (RM_PageHdr*)data;
		records = data + rmFileScan->pRMFileHandle->rmFileHdr.slotsOffset;

		for (; nextSlotNum < rmPageHdr->slotCount; nextSlotNum++) {
			if (!getBit(rmPageHdr->slotBitMap, nextSlotNum))
				continue;

			int i = 0;
			for (; i < rmFileScan->conNum; i++) {
				if (!innerCmp(rmFileScan->conditions[i],
					WHICH_REC(recordSize, records, nextSlotNum))) {
					break;
				}
			}

			if (i != rmFileScan->conNum)
				continue;

			rec->bValid = true;
			rec->rid.pageNum = nextPageNum;
			rec->rid.slotNum = nextSlotNum;
			memcpy(rec->pData, WHICH_REC(recordSize, records, nextSlotNum), recordSize);

			std::cout << "PageNum: " << nextPageNum << " SlotNum: " << nextSlotNum << std::endl;
			std::cout << "Content: " << rec->pData << std::endl;

			if (nextSlotNum == rmPageHdr->slotCount - 1) {
				rmFileScan->pn = nextPageNum + 1;
				rmFileScan->sn = 0;
			} else {
				rmFileScan->pn = nextPageNum;
				rmFileScan->sn = nextSlotNum + 1;
			}

			if ((rc = UnpinPage(&pfPageHandle)))
				return rc;

			return SUCCESS;
		}

		if ((rc = UnpinPage(&pfPageHandle)))
			return rc;

		nextSlotNum = 0;
	}

	return RM_EOF;
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
	PageNum firstFreePage, lastPageNum;
	SlotNum firstFreeSlot, lastSlotNum;
	PF_PageHandle pfPageHandle, lastPageHandle, newPageHandle;
	RM_PageHdr* rmPageHdr;
	char* data;
	char* records;
	int* nextFreeSlot;

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
		memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, firstFreeSlot),
			pData, fileHandle->rmFileHdr.recordSize);
		rmPageHdr->firstFreeSlot = *((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, firstFreeSlot));
		*((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, firstFreeSlot)) = RM_NO_MORE_FREE_SLOT;
		rid->pageNum = firstFreePage;
		rid->slotNum = firstFreeSlot;

		setBit(rmPageHdr->slotBitMap, firstFreeSlot, true);

		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;

		//    1.4 �����ʱ ֮ǰ�� slot λ�õ� nextSlot �� RM_NO_MORE_FREE_SLOT
		if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
			fileHandle->rmFileHdr.firstFreePage = rmPageHdr->nextFreePage;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			fileHandle->isRMHdrDirty = true;
		}
	} else {
		// ��� pageNum == RM_NO_MORE_FREE_PAGE
		if ((rc = GetLastPageNum(&(fileHandle->pfFileHandle), &lastPageNum)) ||
			(rc = GetThisPage(&(fileHandle->pfFileHandle), lastPageNum, &lastPageHandle)) ||
			(rc = GetData(&lastPageHandle, &data))) {
			return rc;
		}

		// 2.2 ��ø�ҳ PageHdr �е� slotNum, ��� slotNum ��δ�ﵽ�������ƣ������ļ�����д���µļ�¼
		rmPageHdr = (RM_PageHdr*)data;
		lastSlotNum = rmPageHdr->slotCount;
		records = data + fileHandle->rmFileHdr.slotsOffset;

		if (lastPageNum >= 2 && lastSlotNum < fileHandle->rmFileHdr.recordsPerPage) {
			// slotNum ��δ�ﵽ��������, �����ļ�����д���µļ�¼
			memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, lastSlotNum),
				pData, fileHandle->rmFileHdr.recordSize);
			setBit(rmPageHdr->slotBitMap, lastSlotNum, true);
			*((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, lastSlotNum)) = RM_NO_MORE_FREE_SLOT;
			rid->pageNum = lastPageNum;
			rid->slotNum = lastSlotNum;

			rmPageHdr->slotCount++;

			if ((rc = MarkDirty(&lastPageHandle)))
				return rc;
		} else {
			//  2.3.1 ͨ�� AllocatePage �򻺳��������µ�ҳ��
			if ((rc = AllocatePage(&(fileHandle->pfFileHandle), &newPageHandle)) ||
				(rc = GetData(&newPageHandle, &data)))
				return rc;

			rmPageHdr = (RM_PageHdr*)data;
			records = data + fileHandle->rmFileHdr.slotsOffset;

			//  2.3.2 ��ʼ�� PageHdr
			rmPageHdr->firstFreeSlot = RM_NO_MORE_FREE_SLOT;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			rmPageHdr->slotCount = 0;

			//  2.3.3 д�룬���� bitmap, ����slotCount�������� ��ҳ Ϊ��
			memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, 0),
				pData, fileHandle->rmFileHdr.recordSize);
			*((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, 0)) = RM_NO_MORE_FREE_SLOT;
			rid->pageNum = newPageHandle.pFrame->page.pageNum;
			rid->slotNum = 0;

			setBit(rmPageHdr->slotBitMap, 0, true);
			rmPageHdr->slotCount++;

			if ((rc = MarkDirty(&newPageHandle)) ||
				(rc = UnpinPage(&newPageHandle)))
				return rc;
		}

		if ((rc = UnpinPage(&lastPageHandle)))
			return rc;
	}

	return SUCCESS;
}

//
// Ŀ��: ɾ��ָ��RIDλ�õļ�¼
// 1. ͨ�� GetThisPage ��� rid ָʾ��ҳ
// 2. ͨ����ҳ�� bitMap ��� ��Ӧ�� slotNum ��¼�Ƿ���Ч
// 3. ά�� free slot list
// 4. ���� bitMap
// 5. ���ԭ������ free Page list �ϣ� ����ҳ���� free ��
// 
//
RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;
	char* data;
	char* records;
	int* nextFreeSlot;
	RM_PageHdr* rmPageHdr;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	if (pageNum < 2)
		return RM_INVALIDRID;

	// 1. ͨ�� GetThisPage ��� rid ָʾ��ҳ
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

	// 2. ͨ����ҳ�� bitMap ��� ��Ӧ�� slotNum ��¼�Ƿ���Ч
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 3. ά�� free slot list
	nextFreeSlot = (int*)(REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, slotNum));
	*nextFreeSlot = rmPageHdr->firstFreeSlot;

	// 4. ���� bitMap
	setBit(rmPageHdr->slotBitMap, slotNum, false);

	if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
		// 5. ���ԭ������ free Page list �ϣ� ����ҳ���� free ��
		rmPageHdr->nextFreePage = fileHandle->rmFileHdr.firstFreePage;
		fileHandle->rmFileHdr.firstFreePage = pageNum;
		
		fileHandle->isRMHdrDirty = true;
	}

	rmPageHdr->firstFreeSlot = slotNum;

	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

//
// Ŀ��: ����ָ�� RID ������
//
RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	RC rc;
	RM_Record search;
	PF_PageHandle pfPageHandle;
	PageNum pageNum = rec->rid.pageNum;
	SlotNum slotNum = rec->rid.slotNum;
	RID tmp;

	tmp.pageNum = pageNum;
	tmp.slotNum = slotNum;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	if (pageNum < 2)
		return RM_INVALIDRID;

	if ((rc = GetRec(fileHandle, &tmp, &search)))
		return rc;

	if (!search.bValid)
		return RM_INVALIDRID;

	memcpy(search.pData, rec->pData, fileHandle->rmFileHdr.recordSize);

	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

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
	maxRecordsNum = ( ( PF_PAGE_SIZE - sizeof(RM_PageHdr) ) << 3 ) / ( (recordSize << 3) + (sizeof(int) << 3) + 1 );
	// std::cout << "PageSize: " << PF_PAGE_SIZE << " sizeof(RM_PageHdr) " << sizeof(RM_PageHdr) << std::endl;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	if ((((maxRecordsNum + 7) >> 3) +
		((recordSize + sizeof(int)) * maxRecordsNum) +
		sizeof(RM_PageHdr)) > PF_PAGE_SIZE)
		maxRecordsNum--;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	// std::cout << "maxBitMapNum: " << ((maxRecordsNum + 7) >> 3) << std::endl;
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
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + ((maxRecordsNum + 7) >> 3);

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
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle))) {
		std::cout << "A" << std::endl;
		return rc;
	}

	// 3. ��ʼ�� RM_FileHandle ��Ա
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	if ((rc = GetData(&pfPageHandle, &data))) {
		std::cout << "B" << std::endl;
		return rc;
	}
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
