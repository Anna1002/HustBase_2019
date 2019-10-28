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
<<<<<<< HEAD
// ��ʼ�� RM_FileScan �ṹ
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//��ʼ��ɨ��
=======
// 初始化 RM_FileScan 结构
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//初始化扫描
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
{
	RC rc;

	memset(rmFileScan, 0, sizeof(RM_FileScan));

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
<<<<<<< HEAD
// Ŀ��: 
=======
// 目的: 
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC CloseScan(RM_FileScan* rmFileScan)
{
	if (!rmFileScan->bOpen)
		return RM_FSCLOSED;

	rmFileScan->bOpen = false;
	return SUCCESS;
}

//
<<<<<<< HEAD
// Ŀ��: RM_FileScan ������
=======
// 目的: RM_FileScan 迭代器
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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

	assert(rec->pData);

	if ((rc = GetLastPageNum(&(rmFileScan->pRMFileHandle->pfFileHandle), &lastPageNum)))
		return rc;

	nextPageNum = rmFileScan->pn;
	nextSlotNum = rmFileScan->sn;

	for (; nextPageNum <= lastPageNum; nextPageNum++) {
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
				std::cout << "============================== Page " << nextPageNum << " end ================================" << std::endl;
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

		std::cout << "============================== Page " << nextPageNum << " end ================================" << std::endl;
		nextSlotNum = 0;
	}

	return RM_EOF;
}

//
<<<<<<< HEAD
// Ŀ��: ���ݸ����� rid, ��ȡ��Ӧ���ļ���¼���浽 rec ָ��ĵ�ַ
// 1. ͨ�� rid ���ָ���� pageNum �� slotNum
// 2. ͨ�� PF_FileHandle::GetThisPage() ��� pageNum ҳ
// 3. ��ø�ҳ�ϵ� PageHdr ��Ϣ, ��λ��¼������ʼ��ַ
// 4. ���� slotNum ���Ƿ��ǿ��õ�
// 5. ���÷��ص� rec
=======
// 目的: 根据给定的 rid, 获取相应的文件记录保存到 rec 指向的地址
// 1. 通过 rid 获得指定的 pageNum 和 slotNum
// 2. 通过 PF_FileHandle::GetThisPage() 获得 pageNum 页
// 3. 获得该页上的 PageHdr 信息, 定位记录数组起始地址
// 4. 检查该 slotNum 项是否是可用的
// 5. 设置返回的 rec
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	RC rc;
	PF_PageHandle pfPageHandle;
	RM_PageHdr *rmPageHdr;
	char *data;
	char *records;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	rec->bValid = false;
<<<<<<< HEAD
	// 1. ͨ�� rid ���ָ���� pageNum �� slotNum
=======
	// 1. 通过 rid 获得指定的 pageNum 和 slotNum
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;

	if (pageNum < 2)
		return RM_INVALIDRID;

<<<<<<< HEAD
	// 2. ͨ�� PF_FileHandle::GetThisPage() ��� pageNum ҳ
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. ��ø�ҳ�ϵ� PageHdr ��Ϣ, ��λ��¼������ʼ��ַ
=======
	// 2. 通过 PF_FileHandle::GetThisPage() 获得 pageNum 页
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. 获得该页上的 PageHdr 信息, 定位记录数组起始地址
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
	// 4. ���� slotNum ���Ƿ��ǿ��õ�
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. ���÷��ص� rec
=======
	// 4. 检查该 slotNum 项是否是可用的
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. 设置返回的 rec
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	rec->bValid = true;
	memcpy(rec->pData, WHICH_REC(fileHandle->rmFileHdr.recordSize, records, slotNum), 
		fileHandle->rmFileHdr.recordSize);
	rec->rid = *rid;

	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

//
<<<<<<< HEAD
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
=======
// 目的: 向文件中写入记录项
// 1. 通过 RM_FileHandle 的 RM_FileHdr 中的 firstFreePage 获得第一个可用的 pageNum
// 如果 pageNum != RM_NO_MORE_FREE_PAGE
//    1.1 通过 GetThisPage() 获得该页 pageHdr 的地址
//    1.2 通过 pageHdr 中的 firstFreeSlot  
//    1.3 写入，标记脏，更新 freeSlot 链, 修改bitmap (即更新 pageHdr 的 firstFreeSlot)
//    1.4 如果此时 之前该 slot 位置的 nextSlot 是 RM_NO_MORE_FREE_SLOT
//          1.4.1 那么 FileHandle 的 RM_FileHdr 的 firstFreePage 同样进行修改 维护 FreePage 链
//          1.4.2 维护之后 设置 FileHdr 为脏
// 如果 pageNum == RM_NO_MORE_FREE_PAGE
//    2.1 获得 最后一页 的 pageNum， GetThisPage到内存缓冲区
//    2.2 获得该页 PageHdr 中的 slotNum, 如果 slotNum 尚未达到数量限制，就在文件后面写入新的记录
//    2.3 如果 slotNum 数量达到了 recordsPerPage
//          2.3.1 通过 AllocatePage 向缓冲区申请新的页面
//          2.3.2 初始化 PageHdr
//          2.3.3 写入，更新 bitmap, 更新slotCount，并设置 该页 为脏
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
		//    1.1 ͨ�� GetThisPage() ��ø�ҳ pageHdr �ĵ�ַ
=======
		//    1.1 通过 GetThisPage() 获得该页 pageHdr 的地址
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), firstFreePage, &pfPageHandle)) || 
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

<<<<<<< HEAD
		//    1.2 ͨ�� pageHdr �е� firstFreeSlot  
=======
		//    1.2 通过 pageHdr 中的 firstFreeSlot  
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		rmPageHdr = (RM_PageHdr*)data;
		firstFreeSlot = rmPageHdr->firstFreeSlot;

		records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
		//    1.3 д�룬����࣬���� freeSlot �� (������ pageHdr �� firstFreeSlot)
=======
		//    1.3 写入，标记脏，更新 freeSlot 链 (即更新 pageHdr 的 firstFreeSlot)
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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

<<<<<<< HEAD
		//    1.4 �����ʱ ֮ǰ�� slot λ�õ� nextSlot �� RM_NO_MORE_FREE_SLOT
=======
		//    1.4 如果此时 之前该 slot 位置的 nextSlot 是 RM_NO_MORE_FREE_SLOT
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
			fileHandle->rmFileHdr.firstFreePage = rmPageHdr->nextFreePage;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			fileHandle->isRMHdrDirty = true;
		}
	} else {
<<<<<<< HEAD
		// ��� pageNum == RM_NO_MORE_FREE_PAGE
=======
		// 如果 pageNum == RM_NO_MORE_FREE_PAGE
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if ((rc = GetLastPageNum(&(fileHandle->pfFileHandle), &lastPageNum)) ||
			(rc = GetThisPage(&(fileHandle->pfFileHandle), lastPageNum, &lastPageHandle)) ||
			(rc = GetData(&lastPageHandle, &data))) {
			return rc;
		}

<<<<<<< HEAD
		// 2.2 ��ø�ҳ PageHdr �е� slotNum, ��� slotNum ��δ�ﵽ�������ƣ������ļ�����д���µļ�¼
=======
		// 2.2 获得该页 PageHdr 中的 slotNum, 如果 slotNum 尚未达到数量限制，就在文件后面写入新的记录
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		rmPageHdr = (RM_PageHdr*)data;
		lastSlotNum = rmPageHdr->slotCount;
		records = data + fileHandle->rmFileHdr.slotsOffset;

		if (lastPageNum >= 2 && lastSlotNum < fileHandle->rmFileHdr.recordsPerPage) {
<<<<<<< HEAD
			// slotNum ��δ�ﵽ��������, �����ļ�����д���µļ�¼
=======
			// slotNum 尚未达到数量限制, 就在文件后面写入新的记录
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
			//  2.3.1 ͨ�� AllocatePage �򻺳��������µ�ҳ��
=======
			//  2.3.1 通过 AllocatePage 向缓冲区申请新的页面
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
			if ((rc = AllocatePage(&(fileHandle->pfFileHandle), &newPageHandle)) ||
				(rc = GetData(&newPageHandle, &data)))
				return rc;

			rmPageHdr = (RM_PageHdr*)data;
			records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
			//  2.3.2 ��ʼ�� PageHdr
=======
			//  2.3.2 初始化 PageHdr
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
			rmPageHdr->firstFreeSlot = RM_NO_MORE_FREE_SLOT;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			rmPageHdr->slotCount = 0;

<<<<<<< HEAD
			//  2.3.3 д�룬���� bitmap, ����slotCount�������� ��ҳ Ϊ��
=======
			//  2.3.3 写入，更新 bitmap, 更新slotCount，并设置 该页 为脏
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
// Ŀ��: ɾ��ָ��RIDλ�õļ�¼
// 1. ͨ�� GetThisPage ��� rid ָʾ��ҳ
// 2. ͨ����ҳ�� bitMap ��� ��Ӧ�� slotNum ��¼�Ƿ���Ч
// 3. ά�� free slot list
// 4. ���� bitMap
// 5. ���ԭ������ free Page list �ϣ� ����ҳ���� free ��
=======
// 目的: 删除指定RID位置的记录
// 1. 通过 GetThisPage 获得 rid 指示的页
// 2. 通过该页的 bitMap 检查 相应的 slotNum 记录是否有效
// 3. 维护 free slot list
// 4. 更新 bitMap
// 5. 如果原来不在 free Page list 上， 将该页加入 free 链
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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

<<<<<<< HEAD
	// 1. ͨ�� GetThisPage ��� rid ָʾ��ҳ
=======
	// 1. 通过 GetThisPage 获得 rid 指示的页
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
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
=======
	// 2. 通过该页的 bitMap 检查 相应的 slotNum 记录是否有效
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 3. 维护 free slot list
	nextFreeSlot = (int*)(REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, slotNum));
	*nextFreeSlot = rmPageHdr->firstFreeSlot;

	// 4. 更新 bitMap
	setBit(rmPageHdr->slotBitMap, slotNum, false);

	if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
		// 5. 如果原来不在 free Page list 上， 将该页加入 free 链
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
// Ŀ��: ����ָ�� RID ������
=======
// 目的: 更新指定 RID 的内容
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	PageNum pageNum = rec->rid.pageNum;
	SlotNum slotNum = rec->rid.slotNum;
	RID tmp;
	char* data;
	char* records;
	RM_PageHdr* rmPageHdr;

	tmp.pageNum = pageNum;
	tmp.slotNum = slotNum;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	if (pageNum < 2)
		return RM_INVALIDRID;

	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, slotNum), rec->pData,
		fileHandle->rmFileHdr.recordSize);

	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

// 
// 目的: 获取文件句柄 fd, 设置 RM 控制页 (第 1 页)
// 1. 根据 recordSize 计算出每一页可以放置的记录个数
// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
// 5. 初始化一个 RM_FileHdr 结构，写到 pData 指向的内存区
// 6. 标记 第 1 页 为脏
// 7. PF_Manager::CloseFile() 将调用 ForceAllPage
//
RC RM_CreateFile (char *fileName, int recordSize)
{
	int maxRecordsNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	RM_FileHdr* rmFileHdr;
	char* data;
<<<<<<< HEAD
	// ��� recordSize �Ƿ�Ϸ� 
	// (���ڲ�������ķ�ʽ������Ҫ�ڼ�¼֮ǰ���� nextFreeSlot �ֶ����Լ��� sizeof(int))
=======
	// 检查 recordSize 是否合法 
	// (由于采用链表的方式所以需要在记录之前保存 nextFreeSlot 字段所以加上 sizeof(int))
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if (recordSize + sizeof(int) > PF_PAGE_SIZE || 
		recordSize < 0)
		return RM_INVALIDRECSIZE;

<<<<<<< HEAD
	// 1. ����ÿһҳ���Է��õļ�¼����
=======
	// 1. 计算每一页可以放置的记录个数
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	maxRecordsNum = ( ( PF_PAGE_SIZE - sizeof(RM_PageHdr) ) << 3 ) / ( (recordSize << 3) + (sizeof(int) << 3) + 1 );
	// std::cout << "PageSize: " << PF_PAGE_SIZE << " sizeof(RM_PageHdr) " << sizeof(RM_PageHdr) << std::endl;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	if ((((maxRecordsNum + 7) >> 3) +
		((recordSize + sizeof(int)) * maxRecordsNum) +
		sizeof(RM_PageHdr)) > PF_PAGE_SIZE)
		maxRecordsNum--;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	// std::cout << "maxBitMapNum: " << ((maxRecordsNum + 7) >> 3) << std::endl;
<<<<<<< HEAD
	// 2. ���� PF_Manager::CreateFile() �� Paged File ����ؿ�����Ϣ���г�ʼ��
	// 3. ���� PF_Manager::OpenFile() �򿪸��ļ� ��ȡ PF_FileHandle
=======
	// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
	// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile(fileName, &pfFileHandle)))
		return rc;

	// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
	if ((rc = AllocatePage(&pfFileHandle, &pfPageHandle)))
		return rc;

	assert(pfPageHandle.pFrame->page.pageNum == 1);
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmFileHdr = (RM_FileHdr*)data;

	// 5. 初始化一个 RM_FileHdr 结构，写到 pData 指向的内存区
	rmFileHdr->firstFreePage = RM_NO_MORE_FREE_PAGE;
	rmFileHdr->recordSize = recordSize;
	rmFileHdr->recordsPerPage = maxRecordsNum;
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + ((maxRecordsNum + 7) >> 3);

	// 6. 标记 第 1 页 为脏
	// 7. PF_Manager::CloseFile() 将调用 ForceAllPage
	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)) || 
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}

<<<<<<< HEAD
// 
<<<<<<< HEAD
// Ŀ��: ���ļ������� RM_fileHandle
// 1. ���� PF_Manager::OpenFile�����һ�� PF_FileHandle
// 2. ͨ�� PF_FileHandle ����ȡ1��ҳ���ϵ� RM_FileHdr ��Ϣ
// 3. ��ʼ�� RM_FileHandle ��Ա
=======
// 目的: 打开文件，建立 RM_fileHandle
// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
// 3. 初始化 RM_FileHandle 成员
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	char* data;

	memset(fileHandle, 0, sizeof(RM_FileHandle));

	if (fileHandle->bOpen)
		return RM_FHOPENNED;

<<<<<<< HEAD
	// 1. ���� PF_Manager::OpenFile�����һ�� PF_FileHandle
	// 2. ͨ�� PF_FileHandle ����ȡ1��ҳ���ϵ� RM_FileHdr ��Ϣ
=======
	// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
	// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = openFile(fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle))) {
		return rc;
	}

<<<<<<< HEAD
	// 3. ��ʼ�� RM_FileHandle ��Ա
=======
	// 3. 初始化 RM_FileHandle 成员
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	if ((rc = GetData(&pfPageHandle, &data))) {
		return rc;
	}
	fileHandle->rmFileHdr = *((RM_FileHdr*)data);

<<<<<<< HEAD
	// 4. �ͷ��ڴ滺����
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;
=======
//Ŀ�ģ���ȡ�ļ�������RM_FileHandle
=======
	// 4. 释放内存缓冲区
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;
=======
//目的；读取文件，设置RM_FileHandle
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
>>>>>>> f0594f8d9057450446ce7c495932a73b2b65bf97

	return SUCCESS;
}


// 
<<<<<<< HEAD
// Ŀ��: �ر��ļ����ж� RM_FileHdr �Ƿ��޸�
// ����������޸ģ� GetThisPage���ڴ滺������д���ҳ, ���Ϊ��֮��
// ���� PF_Manager::CloseFile() �ر� RM �ļ�
=======
// 目的: 关闭文件，判断 RM_FileHdr 是否被修改
// 如果发生了修改， GetThisPage到内存缓冲区，写入该页, 标记为脏之后
// 调用 PF_Manager::CloseFile() 关闭 RM 文件
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

<<<<<<< HEAD
	// �ж��Ƿ������޸�
=======
	// 判断是否发生了修改
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if (fileHandle->isRMHdrDirty) {
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

<<<<<<< HEAD
		// ��Ҫ����
=======
		// 需要拷贝
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		memcpy(data, (char*) & (fileHandle->rmFileHdr), sizeof(RM_FileHdr));
		
		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

<<<<<<< HEAD
	// ���� PF_Manager::CloseFile() �ر� RM �ļ�
=======
	// 调用 PF_Manager::CloseFile() 关闭 RM 文件
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = CloseFile(&(fileHandle->pfFileHandle))))
		return rc;

	fileHandle->bOpen = false;
	fileHandle->isRMHdrDirty = false;

	return SUCCESS;
}
