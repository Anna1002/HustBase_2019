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
// 初始化 RM_FileScan 结构
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//初始化扫描
=======
// 鍒濆鍖� RM_FileScan 缁撴瀯
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//鍒濆鍖栨壂鎻�
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
// 目的: 
=======
// 鐩殑: 
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
// 目的: RM_FileScan 迭代器
=======
// 鐩殑: RM_FileScan 杩唬鍣�
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
// 目的: 根据给定的 rid, 获取相应的文件记录保存到 rec 指向的地址
// 1. 通过 rid 获得指定的 pageNum 和 slotNum
// 2. 通过 PF_FileHandle::GetThisPage() 获得 pageNum 页
// 3. 获得该页上的 PageHdr 信息, 定位记录数组起始地址
// 4. 检查该 slotNum 项是否是可用的
// 5. 设置返回的 rec
=======
// 鐩殑: 鏍规嵁缁欏畾鐨� rid, 鑾峰彇鐩稿簲鐨勬枃浠惰褰曚繚瀛樺埌 rec 鎸囧悜鐨勫湴鍧�
// 1. 閫氳繃 rid 鑾峰緱鎸囧畾鐨� pageNum 鍜� slotNum
// 2. 閫氳繃 PF_FileHandle::GetThisPage() 鑾峰緱 pageNum 椤�
// 3. 鑾峰緱璇ラ〉涓婄殑 PageHdr 淇℃伅, 瀹氫綅璁板綍鏁扮粍璧峰鍦板潃
// 4. 妫�鏌ヨ slotNum 椤规槸鍚︽槸鍙敤鐨�
// 5. 璁剧疆杩斿洖鐨� rec
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
	// 1. 通过 rid 获得指定的 pageNum 和 slotNum
=======
	// 1. 閫氳繃 rid 鑾峰緱鎸囧畾鐨� pageNum 鍜� slotNum
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;

	if (pageNum < 2)
		return RM_INVALIDRID;

<<<<<<< HEAD
	// 2. 通过 PF_FileHandle::GetThisPage() 获得 pageNum 页
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. 获得该页上的 PageHdr 信息, 定位记录数组起始地址
=======
	// 2. 閫氳繃 PF_FileHandle::GetThisPage() 鑾峰緱 pageNum 椤�
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. 鑾峰緱璇ラ〉涓婄殑 PageHdr 淇℃伅, 瀹氫綅璁板綍鏁扮粍璧峰鍦板潃
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
	// 4. 检查该 slotNum 项是否是可用的
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. 设置返回的 rec
=======
	// 4. 妫�鏌ヨ slotNum 椤规槸鍚︽槸鍙敤鐨�
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. 璁剧疆杩斿洖鐨� rec
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
=======
// 鐩殑: 鍚戞枃浠朵腑鍐欏叆璁板綍椤�
// 1. 閫氳繃 RM_FileHandle 鐨� RM_FileHdr 涓殑 firstFreePage 鑾峰緱绗竴涓彲鐢ㄧ殑 pageNum
// 濡傛灉 pageNum != RM_NO_MORE_FREE_PAGE
//    1.1 閫氳繃 GetThisPage() 鑾峰緱璇ラ〉 pageHdr 鐨勫湴鍧�
//    1.2 閫氳繃 pageHdr 涓殑 firstFreeSlot  
//    1.3 鍐欏叆锛屾爣璁拌剰锛屾洿鏂� freeSlot 閾�, 淇敼bitmap (鍗虫洿鏂� pageHdr 鐨� firstFreeSlot)
//    1.4 濡傛灉姝ゆ椂 涔嬪墠璇� slot 浣嶇疆鐨� nextSlot 鏄� RM_NO_MORE_FREE_SLOT
//          1.4.1 閭ｄ箞 FileHandle 鐨� RM_FileHdr 鐨� firstFreePage 鍚屾牱杩涜淇敼 缁存姢 FreePage 閾�
//          1.4.2 缁存姢涔嬪悗 璁剧疆 FileHdr 涓鸿剰
// 濡傛灉 pageNum == RM_NO_MORE_FREE_PAGE
//    2.1 鑾峰緱 鏈�鍚庝竴椤� 鐨� pageNum锛� GetThisPage鍒板唴瀛樼紦鍐插尯
//    2.2 鑾峰緱璇ラ〉 PageHdr 涓殑 slotNum, 濡傛灉 slotNum 灏氭湭杈惧埌鏁伴噺闄愬埗锛屽氨鍦ㄦ枃浠跺悗闈㈠啓鍏ユ柊鐨勮褰�
//    2.3 濡傛灉 slotNum 鏁伴噺杈惧埌浜� recordsPerPage
//          2.3.1 閫氳繃 AllocatePage 鍚戠紦鍐插尯鐢宠鏂扮殑椤甸潰
//          2.3.2 鍒濆鍖� PageHdr
//          2.3.3 鍐欏叆锛屾洿鏂� bitmap, 鏇存柊slotCount锛屽苟璁剧疆 璇ラ〉 涓鸿剰
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
		//    1.1 通过 GetThisPage() 获得该页 pageHdr 的地址
=======
		//    1.1 閫氳繃 GetThisPage() 鑾峰緱璇ラ〉 pageHdr 鐨勫湴鍧�
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), firstFreePage, &pfPageHandle)) || 
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

<<<<<<< HEAD
		//    1.2 通过 pageHdr 中的 firstFreeSlot  
=======
		//    1.2 閫氳繃 pageHdr 涓殑 firstFreeSlot  
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		rmPageHdr = (RM_PageHdr*)data;
		firstFreeSlot = rmPageHdr->firstFreeSlot;

		records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
		//    1.3 写入，标记脏，更新 freeSlot 链 (即更新 pageHdr 的 firstFreeSlot)
=======
		//    1.3 鍐欏叆锛屾爣璁拌剰锛屾洿鏂� freeSlot 閾� (鍗虫洿鏂� pageHdr 鐨� firstFreeSlot)
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
		//    1.4 如果此时 之前该 slot 位置的 nextSlot 是 RM_NO_MORE_FREE_SLOT
=======
		//    1.4 濡傛灉姝ゆ椂 涔嬪墠璇� slot 浣嶇疆鐨� nextSlot 鏄� RM_NO_MORE_FREE_SLOT
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
			fileHandle->rmFileHdr.firstFreePage = rmPageHdr->nextFreePage;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			fileHandle->isRMHdrDirty = true;
		}
	} else {
<<<<<<< HEAD
		// 如果 pageNum == RM_NO_MORE_FREE_PAGE
=======
		// 濡傛灉 pageNum == RM_NO_MORE_FREE_PAGE
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if ((rc = GetLastPageNum(&(fileHandle->pfFileHandle), &lastPageNum)) ||
			(rc = GetThisPage(&(fileHandle->pfFileHandle), lastPageNum, &lastPageHandle)) ||
			(rc = GetData(&lastPageHandle, &data))) {
			return rc;
		}

<<<<<<< HEAD
		// 2.2 获得该页 PageHdr 中的 slotNum, 如果 slotNum 尚未达到数量限制，就在文件后面写入新的记录
=======
		// 2.2 鑾峰緱璇ラ〉 PageHdr 涓殑 slotNum, 濡傛灉 slotNum 灏氭湭杈惧埌鏁伴噺闄愬埗锛屽氨鍦ㄦ枃浠跺悗闈㈠啓鍏ユ柊鐨勮褰�
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		rmPageHdr = (RM_PageHdr*)data;
		lastSlotNum = rmPageHdr->slotCount;
		records = data + fileHandle->rmFileHdr.slotsOffset;

		if (lastPageNum >= 2 && lastSlotNum < fileHandle->rmFileHdr.recordsPerPage) {
<<<<<<< HEAD
			// slotNum 尚未达到数量限制, 就在文件后面写入新的记录
=======
			// slotNum 灏氭湭杈惧埌鏁伴噺闄愬埗, 灏卞湪鏂囦欢鍚庨潰鍐欏叆鏂扮殑璁板綍
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
			//  2.3.1 通过 AllocatePage 向缓冲区申请新的页面
=======
			//  2.3.1 閫氳繃 AllocatePage 鍚戠紦鍐插尯鐢宠鏂扮殑椤甸潰
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
			if ((rc = AllocatePage(&(fileHandle->pfFileHandle), &newPageHandle)) ||
				(rc = GetData(&newPageHandle, &data)))
				return rc;

			rmPageHdr = (RM_PageHdr*)data;
			records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
			//  2.3.2 初始化 PageHdr
=======
			//  2.3.2 鍒濆鍖� PageHdr
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
			rmPageHdr->firstFreeSlot = RM_NO_MORE_FREE_SLOT;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			rmPageHdr->slotCount = 0;

<<<<<<< HEAD
			//  2.3.3 写入，更新 bitmap, 更新slotCount，并设置 该页 为脏
=======
			//  2.3.3 鍐欏叆锛屾洿鏂� bitmap, 鏇存柊slotCount锛屽苟璁剧疆 璇ラ〉 涓鸿剰
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
// 目的: 删除指定RID位置的记录
// 1. 通过 GetThisPage 获得 rid 指示的页
// 2. 通过该页的 bitMap 检查 相应的 slotNum 记录是否有效
// 3. 维护 free slot list
// 4. 更新 bitMap
// 5. 如果原来不在 free Page list 上， 将该页加入 free 链
=======
// 鐩殑: 鍒犻櫎鎸囧畾RID浣嶇疆鐨勮褰�
// 1. 閫氳繃 GetThisPage 鑾峰緱 rid 鎸囩ず鐨勯〉
// 2. 閫氳繃璇ラ〉鐨� bitMap 妫�鏌� 鐩稿簲鐨� slotNum 璁板綍鏄惁鏈夋晥
// 3. 缁存姢 free slot list
// 4. 鏇存柊 bitMap
// 5. 濡傛灉鍘熸潵涓嶅湪 free Page list 涓婏紝 灏嗚椤靛姞鍏� free 閾�
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
	// 1. 通过 GetThisPage 获得 rid 指示的页
=======
	// 1. 閫氳繃 GetThisPage 鑾峰緱 rid 鎸囩ず鐨勯〉
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
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
=======
	// 2. 閫氳繃璇ラ〉鐨� bitMap 妫�鏌� 鐩稿簲鐨� slotNum 璁板綍鏄惁鏈夋晥
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 3. 缁存姢 free slot list
	nextFreeSlot = (int*)(REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, slotNum));
	*nextFreeSlot = rmPageHdr->firstFreeSlot;

	// 4. 鏇存柊 bitMap
	setBit(rmPageHdr->slotBitMap, slotNum, false);

	if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
		// 5. 濡傛灉鍘熸潵涓嶅湪 free Page list 涓婏紝 灏嗚椤靛姞鍏� free 閾�
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
// 目的: 更新指定 RID 的内容
=======
// 鐩殑: 鏇存柊鎸囧畾 RID 鐨勫唴瀹�
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
// 鐩殑: 鑾峰彇鏂囦欢鍙ユ焺 fd, 璁剧疆 RM 鎺у埗椤� (绗� 1 椤�)
// 1. 鏍规嵁 recordSize 璁＄畻鍑烘瘡涓�椤靛彲浠ユ斁缃殑璁板綍涓暟
// 2. 璋冪敤 PF_Manager::CreateFile() 灏� Paged File 鐨勭浉鍏虫帶鍒朵俊鎭繘琛屽垵濮嬪寲
// 3. 璋冪敤 PF_Manager::OpenFile() 鎵撳紑璇ユ枃浠� 鑾峰彇 PF_FileHandle
// 4. 閫氳繃璇� PF_FileHandle 鐨� AllocatePage 鏂规硶鐢宠鍐呭瓨缂撳啿鍖� 鎷垮埌绗� 1 椤电殑 pData 鎸囬拡
// 5. 鍒濆鍖栦竴涓� RM_FileHdr 缁撴瀯锛屽啓鍒� pData 鎸囧悜鐨勫唴瀛樺尯
// 6. 鏍囪 绗� 1 椤� 涓鸿剰
// 7. PF_Manager::CloseFile() 灏嗚皟鐢� ForceAllPage
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
	// 检查 recordSize 是否合法 
	// (由于采用链表的方式所以需要在记录之前保存 nextFreeSlot 字段所以加上 sizeof(int))
=======
	// 妫�鏌� recordSize 鏄惁鍚堟硶 
	// (鐢变簬閲囩敤閾捐〃鐨勬柟寮忔墍浠ラ渶瑕佸湪璁板綍涔嬪墠淇濆瓨 nextFreeSlot 瀛楁鎵�浠ュ姞涓� sizeof(int))
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if (recordSize + sizeof(int) > PF_PAGE_SIZE || 
		recordSize < 0)
		return RM_INVALIDRECSIZE;

<<<<<<< HEAD
	// 1. 计算每一页可以放置的记录个数
=======
	// 1. 璁＄畻姣忎竴椤靛彲浠ユ斁缃殑璁板綍涓暟
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
	// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
	// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
=======
	// 2. 璋冪敤 PF_Manager::CreateFile() 灏� Paged File 鐨勭浉鍏虫帶鍒朵俊鎭繘琛屽垵濮嬪寲
	// 3. 璋冪敤 PF_Manager::OpenFile() 鎵撳紑璇ユ枃浠� 鑾峰彇 PF_FileHandle
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile(fileName, &pfFileHandle)))
		return rc;

	// 4. 閫氳繃璇� PF_FileHandle 鐨� AllocatePage 鏂规硶鐢宠鍐呭瓨缂撳啿鍖� 鎷垮埌绗� 1 椤电殑 pData 鎸囬拡
	if ((rc = AllocatePage(&pfFileHandle, &pfPageHandle)))
		return rc;

	assert(pfPageHandle.pFrame->page.pageNum == 1);
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmFileHdr = (RM_FileHdr*)data;

	// 5. 鍒濆鍖栦竴涓� RM_FileHdr 缁撴瀯锛屽啓鍒� pData 鎸囧悜鐨勫唴瀛樺尯
	rmFileHdr->firstFreePage = RM_NO_MORE_FREE_PAGE;
	rmFileHdr->recordSize = recordSize;
	rmFileHdr->recordsPerPage = maxRecordsNum;
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + ((maxRecordsNum + 7) >> 3);

	// 6. 鏍囪 绗� 1 椤� 涓鸿剰
	// 7. PF_Manager::CloseFile() 灏嗚皟鐢� ForceAllPage
	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)) || 
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}

<<<<<<< HEAD
// 
<<<<<<< HEAD
// 目的: 打开文件，建立 RM_fileHandle
// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
// 3. 初始化 RM_FileHandle 成员
=======
// 鐩殑: 鎵撳紑鏂囦欢锛屽缓绔� RM_fileHandle
// 1. 璋冪敤 PF_Manager::OpenFile锛岃幏寰椾竴涓� PF_FileHandle
// 2. 閫氳繃 PF_FileHandle 鏉ヨ幏鍙�1鍙烽〉闈笂鐨� RM_FileHdr 淇℃伅
// 3. 鍒濆鍖� RM_FileHandle 鎴愬憳
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
	// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
	// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
=======
	// 1. 璋冪敤 PF_Manager::OpenFile锛岃幏寰椾竴涓� PF_FileHandle
	// 2. 閫氳繃 PF_FileHandle 鏉ヨ幏鍙�1鍙烽〉闈笂鐨� RM_FileHdr 淇℃伅
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = openFile(fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle))) {
		return rc;
	}

<<<<<<< HEAD
	// 3. 初始化 RM_FileHandle 成员
=======
	// 3. 鍒濆鍖� RM_FileHandle 鎴愬憳
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	if ((rc = GetData(&pfPageHandle, &data))) {
		return rc;
	}
	fileHandle->rmFileHdr = *((RM_FileHdr*)data);

<<<<<<< HEAD
	// 4. 释放内存缓冲区
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;
=======
//目的；读取文件，设置RM_FileHandle
=======
	// 4. 閲婃斁鍐呭瓨缂撳啿鍖�
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;
=======
//鐩殑锛涜鍙栨枃浠讹紝璁剧疆RM_FileHandle
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
// 目的: 关闭文件，判断 RM_FileHdr 是否被修改
// 如果发生了修改， GetThisPage到内存缓冲区，写入该页, 标记为脏之后
// 调用 PF_Manager::CloseFile() 关闭 RM 文件
=======
// 鐩殑: 鍏抽棴鏂囦欢锛屽垽鏂� RM_FileHdr 鏄惁琚慨鏀�
// 濡傛灉鍙戠敓浜嗕慨鏀癸紝 GetThisPage鍒板唴瀛樼紦鍐插尯锛屽啓鍏ヨ椤�, 鏍囪涓鸿剰涔嬪悗
// 璋冪敤 PF_Manager::CloseFile() 鍏抽棴 RM 鏂囦欢
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
	// 判断是否发生了修改
=======
	// 鍒ゆ柇鏄惁鍙戠敓浜嗕慨鏀�
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if (fileHandle->isRMHdrDirty) {
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

<<<<<<< HEAD
		// 需要拷贝
=======
		// 闇�瑕佹嫹璐�
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		memcpy(data, (char*) & (fileHandle->rmFileHdr), sizeof(RM_FileHdr));
		
		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

<<<<<<< HEAD
	// 调用 PF_Manager::CloseFile() 关闭 RM 文件
=======
	// 璋冪敤 PF_Manager::CloseFile() 鍏抽棴 RM 鏂囦欢
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = CloseFile(&(fileHandle->pfFileHandle))))
		return rc;

	fileHandle->bOpen = false;
	fileHandle->isRMHdrDirty = false;

	return SUCCESS;
}
