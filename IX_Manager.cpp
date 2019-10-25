#include "stdafx.h"
#include "IX_Manager.h"
#include <cassert>
#include <iostream>

int IXComp(IX_IndexHandle *indexHandle, void *ldata, void *rdata, int llen, int rlen)
{
	switch (indexHandle->fileHeader.attrType) {
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

bool ridEqual(const RID *lrid, const RID *rrid)
{
	return (lrid->pageNum == rrid->pageNum && lrid->slotNum == rrid->slotNum);
}

RC OpenIndexScan (IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value)
{
	return SUCCESS;
}

RC IX_GetNextEntry (IX_IndexScan *indexScan,RID * rid)
{
	return SUCCESS;
}

RC CloseIndexScan (IX_IndexScan *indexScan)
{
	return SUCCESS;
}

//RC GetIndexTree(char *fileName, Tree *index){
//		return SUCCESS;
//}


// 
// 目的: 创建一个 Index File，
//       该 Index File 基于 长度为 attrlength 类型为 attrType 的 key
// 实现非常类似 RM_Manager 中的 CreateFile
// 1. 计算出一个 Page 中能够存放的 key entry 结构的数量，或者说初始化 B+树 的 order
// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
// 5. 初始化一个 IX_FileHdr 结构，写到 pData 指向的内存区
// 6. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 2 页的 pData 指针
// 7. 初始化一个 IX_NodeHdr 结构，写到 pData 指向的内存区, 初始化一个空的 Root 节点
// 6. 标记 第 1，2 页 为脏
// 7. PF_Manager::CloseFile() 将调用 ForceAllPage
//
RC CreateIndex (const char* fileName, AttrType attrType, int attrLength)
{
	int maxKeysNum, maxBucketEntryNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfHdrPage, pfFirstRootPage;
	char* fileHdrData, *nodeHdrData;
	IX_FileHeader* ixFileHdr;
	IX_NodePageHeader* ixNodeHdr;

	// B+树 最小 order 需要为 3， split情况下至少要在一个 node 中存下 3条 key记录
	maxKeysNum = (PF_PAGE_SIZE - sizeof(IX_NodePageHeader)) 
					/ (sizeof(IX_NodeEntry) + attrLength);
	maxBucketEntryNum = (PF_PAGE_SIZE - sizeof(IX_BucketPageHeader)) / sizeof(IX_BucketEntry);

	if (maxKeysNum < 3)
		return IX_INVALIDKEYSIZE;

	// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
	// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile((char*)fileName, &pfFileHandle)))
		return rc;

	if ((rc = AllocatePage(&pfFileHandle, &pfHdrPage)) ||
		(rc = GetData(&pfHdrPage, &fileHdrData)) || 
		(rc = AllocatePage(&pfFileHandle, &pfFirstRootPage)) ||
		(rc = GetData(&pfFirstRootPage, &nodeHdrData)))
		return rc;

	ixFileHdr = (IX_FileHeader*)fileHdrData;
	ixNodeHdr = (IX_NodePageHeader*)nodeHdrData;

	ixFileHdr->attrLength = attrLength;
	ixFileHdr->attrType = attrType;
	ixFileHdr->bucketEntryListOffset = sizeof(IX_BucketPageHeader);
	ixFileHdr->entrysPerBucket = maxBucketEntryNum;
	ixFileHdr->first_leaf = 2;
	ixFileHdr->nodeEntryListOffset = sizeof(IX_NodePageHeader);
	ixFileHdr->nodeKeyListOffset = ixFileHdr->nodeEntryListOffset + sizeof(IX_NodeEntry) * maxKeysNum;
	ixFileHdr->order = maxKeysNum;
	ixFileHdr->rootPage = 2;

	ixNodeHdr->firstChild = IX_NULL_CHILD;
	ixNodeHdr->is_leaf = true;
	ixNodeHdr->keynum = 0;
	ixNodeHdr->sibling = IX_NO_MORE_NEXT_LEAF;

	if ((rc = MarkDirty(&pfHdrPage)) ||
		(rc = MarkDirty(&pfFirstRootPage)) ||
		(rc = UnpinPage(&pfHdrPage)) ||
		(rc = UnpinPage(&pfFirstRootPage)) ||
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}


//
// 目的: 打开已经创建的 Index File，并初始化一个 IX_IndexHandle
// 1. 检查合法性
// 2. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
// 3. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
//
RC OpenIndex (const char* fileName, IX_IndexHandle* indexHandle)
{
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	char* data;

	memset(indexHandle, 0, sizeof(IX_IndexHandle));

	if (indexHandle->bOpen)
		return IX_IHOPENNED;

	if ((rc = openFile((char*)fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle)))
		return rc;

	indexHandle->bOpen = true;
	if (rc = GetData(&pfPageHandle, &data))
		return rc;
	indexHandle->fileHeader = *(IX_FileHeader*)data;
	indexHandle->fileHandle = pfFileHandle;
	indexHandle->isHdrDirty = false;

	if (rc = UnpinPage(&pfPageHandle))
		return rc;

	return SUCCESS;
}

// 
// 目的: 关闭文件，将修改的数据刷写到磁盘
// 检查 indexHandle->isHdrDirty 是否为真
// 如果为真，需要GetThisPage获得文件第1页，将indexHandle中维护的IX_FileHeader
// 保存到相应的缓冲区
// 
RC CloseIndex (IX_IndexHandle* indexHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;

	if (!(indexHandle->bOpen))
		return IX_IHCLOSED;

	if (indexHandle->isHdrDirty) {
		if ((rc = GetThisPage(&(indexHandle->fileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

		memcpy(data, (char*) & (indexHandle->fileHeader), sizeof(IX_FileHeader));

		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

	if ((rc = CloseFile(&(indexHandle->fileHandle))))
		return rc;

	indexHandle->bOpen = false;
	indexHandle->isHdrDirty = false;

	return SUCCESS;
}

//
// 目的: 初始化一个 Bucket Page，将初始化好的 Bucket Page 的 pageNum 返回
//
RC CreateBucket(IX_IndexHandle* indexHandle, PageNum* pageNum)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;
	IX_BucketEntry *slotRid;
	IX_BucketPageHeader* bucketPageHdr;

	if ((rc = AllocatePage(&(indexHandle->fileHandle), &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	bucketPageHdr = (IX_BucketPageHeader*)data;
	slotRid = (IX_BucketEntry*)(data + indexHandle->fileHeader.bucketEntryListOffset);

	for (int i = 0; i < indexHandle->fileHeader.entrysPerBucket - 1; i++)
		slotRid[i].nextFreeSlot = i + 1;
	slotRid[indexHandle->fileHeader.entrysPerBucket - 1].nextFreeSlot = IX_NO_MORE_BUCKET_SLOT;

	bucketPageHdr->slotNum = 0;
	bucketPageHdr->nextBucket = IX_NO_MORE_BUCKET_PAGE;
	bucketPageHdr->firstFreeSlot = 0;
	bucketPageHdr->firstValidSlot = IX_NO_MORE_BUCKET_SLOT;

	*pageNum = pfPageHandle.pFrame->page.pageNum;

	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

// 
// 目的: 向给定的 bucket Page(s) 写入 RID
// 
RC InsertRIDIntoBucket(IX_IndexHandle* indexHandle, PageNum bucketPageNum, RID rid)
{
	RC rc;
	IX_BucketPageHeader* bucketPageHdr;
	PF_PageHandle bucketPage;
	IX_BucketEntry* entry;
	PageNum lastPageNum = bucketPageNum;
	char* data;
	
	while (bucketPageNum != IX_NO_MORE_BUCKET_PAGE)
	{
		lastPageNum = bucketPageNum;

		if ((rc = GetThisPage(&(indexHandle->fileHandle), bucketPageNum, &bucketPage)) ||
			(rc = GetData(&bucketPage, &data)))
			return rc;

		bucketPageHdr = (IX_BucketPageHeader*)data;
		entry = (IX_BucketEntry*)(data + indexHandle->fileHeader.bucketEntryListOffset);

		if (bucketPageHdr->firstFreeSlot != IX_NO_MORE_BUCKET_SLOT) {
			// 存在 free slot
			// 写入 RID
			int free_idx = bucketPageHdr->firstFreeSlot;
			int next_free_idx = entry[free_idx].nextFreeSlot;
			entry[free_idx].nextFreeSlot = bucketPageHdr->firstValidSlot;
			entry[free_idx].rid = rid;
			bucketPageHdr->firstValidSlot = free_idx;
			bucketPageHdr->firstFreeSlot = next_free_idx;

			bucketPageHdr->slotNum++;

			// 标记脏
			if ((rc = MarkDirty(&bucketPage)) ||
				(rc = UnpinPage(&bucketPage)))
				return rc;

			return SUCCESS;
		}

		bucketPageNum = bucketPageHdr->nextBucket;
	}

	// 遍历完了全部的 Bucket Page 都没有富裕空间了
	// 创建一个新的
	PageNum newPage;
	PF_PageHandle newPageHandle, lastPageHandle;
	char* newData, *lastData;
	IX_BucketPageHeader* newHdr, *lastPageHdr;
	IX_BucketEntry* newEntry;

	if ((rc = CreateBucket(indexHandle, &newPage)) ||
		(rc = GetThisPage(&(indexHandle->fileHandle), newPage, &newPageHandle)) ||
		(rc = GetData(&newPageHandle, &newData)))
		return rc;

	newHdr = (IX_BucketPageHeader*)newData;
	newEntry = (IX_BucketEntry*)(newData + indexHandle->fileHeader.bucketEntryListOffset);

	int free_idx = newHdr->firstFreeSlot;
	int next_free_idx = newEntry[free_idx].nextFreeSlot;
	newEntry[free_idx].nextFreeSlot = newHdr->firstValidSlot;
	newEntry[free_idx].rid = rid;
	newHdr->firstValidSlot = free_idx;
	newHdr->firstFreeSlot = next_free_idx;

	newHdr->slotNum++;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), lastPageNum, &lastPageHandle)) ||
		(rc = GetData(&lastPageHandle, &lastData)))
		return rc;

	lastPageHdr = (IX_BucketPageHeader*)lastData;

	lastPageHdr->nextBucket = newPage;

	if ((rc = MarkDirty(&newPageHandle)) ||
		(rc = MarkDirty(&lastPageHandle)) ||
		(rc = UnpinPage(&newPageHandle)) ||
		(rc = UnpinPage(&lastPageHandle)))
		return rc;

	return SUCCESS;
}

//
// 目的: 在 Bucket Page(s) 中查找匹配的 rid，并删除相应的 rid
//
RC DeleteRIDFromBucket(IX_IndexHandle* indexHandle, PageNum bucketPageNum, const RID* rid)
{
	RC rc;
	PF_PageHandle bucketPageHandle;
	char* data;
	IX_BucketPageHeader* bucketPageHdr;
	IX_BucketEntry* entry;

	while (bucketPageNum != IX_NO_MORE_BUCKET_PAGE) 
	{
		if ((rc = GetThisPage(&(indexHandle->fileHandle), bucketPageNum, &bucketPageHandle)) ||
			(rc = GetData(&bucketPageHandle, &data)))
			return rc;

		bucketPageHdr = (IX_BucketPageHeader*)data;
		entry = (IX_BucketEntry*)(data + indexHandle->fileHeader.bucketEntryListOffset);

		SlotNum validSlot = bucketPageHdr->firstValidSlot;
		SlotNum preValidSlot = IX_NO_MORE_BUCKET_SLOT;

		while (validSlot != IX_NO_MORE_BUCKET_SLOT) 
		{
			SlotNum next_valid = entry[validSlot].nextFreeSlot;

			if (ridEqual(&(entry[validSlot].rid), rid)) {
				if (preValidSlot == IX_NO_MORE_BUCKET_SLOT) {
					bucketPageHdr->firstValidSlot = next_valid;
				} else {
					entry[preValidSlot].nextFreeSlot = next_valid;
				}

				entry[validSlot].nextFreeSlot = bucketPageHdr->firstFreeSlot;
				bucketPageHdr->firstFreeSlot = validSlot;
				bucketPageHdr->slotNum--;

				if ((rc = MarkDirty(&bucketPageHandle)) ||
					(rc = UnpinPage(&bucketPageHandle)))
					return rc;

				return SUCCESS;
			}

			preValidSlot = validSlot;
			validSlot = entry[validSlot].nextFreeSlot;
		}

		if (rc = UnpinPage(&bucketPageHandle))
			return rc;

		bucketPageNum = bucketPageHdr->nextBucket;
	}

	// 遍历完了也没有找到
	return IX_DELETE_NO_RID;
}

//
// 目的: 在 Node 文件页中 查找指定 pData
//       找到最大的小于等于 pData 的索引位置保存在 idx 中
//       如果存在等于 返回 true 否则返回 false
// pData: 用户指定的数据指针
// key: Node 文件的关键字区首地址指针
// s: 搜索范围起始索引
// e: 搜索范围终止索引
// 上述范围为包含
// 
bool findKeyInNode(IX_IndexHandle* indexHandle, void* pData, void* key, int s, int e, int* idx)
{
	int mid, cmpres;
	int attrLen = indexHandle->fileHeader.attrLength;

	while (s < e) {
		mid = ((s + e) >> 1) + 1;
		cmpres = IXComp(indexHandle, (char*)key + attrLen * mid, pData, attrLen, attrLen);

		if (cmpres < 0) {
			s = mid;
		} else if (cmpres > 0) {
			e = mid - 1;
		} else {
			*idx = mid;
			return true;
		}
	}

	if (IXComp(indexHandle, (char*)key + attrLen * s, pData, attrLen, attrLen) > 0) {
		*idx = s - 1;
	} else {
		*idx = s;
	}
	return false;
}

// 
// shift 不负责获得页面，并设置页面脏
// entry: 页面指针区首地址
// key:   页面关键字区首地址
// idx:   从idx开始到结束位置 len - 1 进行移动
// dir:   true 表示 向后移动; false 反之
//
void shift (IX_IndexHandle* indexHandle, char *entry, char *key, int idx, int len, bool dir) 
{
	int attrLen = indexHandle->fileHeader.attrLength;

	if (dir) {
		for (int i = len - 1; i >= idx; i--) {
			memcpy(key + attrLen * (i + 1), key + attrLen * i, attrLen);
			memcpy(entry + sizeof(IX_NodeEntry) * (i + 1), 
				entry + sizeof(IX_NodeEntry) * i, sizeof(IX_NodeEntry));
		}
	} else {
		for (int i = idx; i < len; i++) {
			memcpy(key + attrLen * (i - 1), key + attrLen * i, attrLen);
			memcpy(entry + sizeof(IX_NodeEntry) * (i - 1),
				entry + sizeof(IX_NodeEntry) * i, sizeof(IX_NodeEntry));
		}
	}
}

//
// splitChild 不负责 parent 页面的释放，但是负责子节点页面和新页面的释放
// 
RC splitChild(IX_IndexHandle* indexHandle, PF_PageHandle* parent, int idx, PageNum child)
{
	RC rc;
	char* data, * par_entry, * par_key;
	char* child_entry, * child_key;
	char* new_entry, * new_key;
	PF_PageHandle childPageHandle, newPageHandle;
	PageNum newPageNum;
	IX_NodePageHeader* parentHdr, * childHdr, * newPageHdr;
	int childKeyNum;
	int attrLen = indexHandle->fileHeader.attrLength;

	if (rc = GetData(parent, &data))
		return rc;

	parentHdr = (IX_NodePageHeader*)data;
	par_entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	par_key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), child, &childPageHandle)) ||
		(rc = GetData(&childPageHandle, &data)))
		return rc;

	childHdr = (IX_NodePageHeader*)data;
	childKeyNum = childHdr->keynum;
	child_entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	child_key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if ((rc = AllocatePage(&(indexHandle->fileHandle), &newPageHandle)) ||
		(rc = GetData(&newPageHandle, &data)))
		return rc;

	newPageHdr = (IX_NodePageHeader*)data;
	newPageNum = newPageHandle.pFrame->page.pageNum;
	new_entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	new_key = data + indexHandle->fileHeader.nodeKeyListOffset;

	IX_NodeEntry* midChildEntry = (IX_NodeEntry*)(child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2));

	if (childHdr->is_leaf) {
		// 子节点是一个叶子节点
		// 将叶子节点从 childKeyNum / 2 开始的所有信息拷贝到新节点位置
		memcpy(new_entry, child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2),
			sizeof(IX_NodeEntry) * (childKeyNum - (childKeyNum / 2)));
		memcpy(new_key, child_key + attrLen * (childKeyNum / 2),
			attrLen * (childKeyNum - (childKeyNum / 2)));

		newPageHdr->firstChild = IX_NULL_CHILD;
		newPageHdr->is_leaf = true;
		newPageHdr->keynum = childKeyNum - (childKeyNum / 2);
		newPageHdr->sibling = childHdr->sibling;
	} else {
		// 子节点是内部节点
		// 将叶子节点从 childKeyNum / 2 + 1 开始的所有信息拷贝到新节点位置
		memcpy(new_entry, child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2 + 1), 
			sizeof(IX_NodeEntry) * (childKeyNum - (childKeyNum / 2) - 1));
		memcpy(new_key, child_key + attrLen * (childKeyNum / 2 + 1),
			attrLen * (childKeyNum - (childKeyNum / 2) - 1));

		newPageHdr->firstChild = midChildEntry->rid.pageNum;
		newPageHdr->is_leaf = false;
		newPageHdr->keynum = childKeyNum - (childKeyNum / 2) - 1;
		newPageHdr->sibling = childHdr->sibling;
	}

	childHdr->keynum = childKeyNum / 2;
	childHdr->sibling = newPageNum;

	// 移动父节点数据
	shift(indexHandle, par_entry, par_key, idx + 1, parentHdr->keynum, true);
	// 把孩子节点的中间数据写入到父节点
	IX_NodeEntry* parEntry = (IX_NodeEntry*)(par_entry + sizeof(IX_NodeEntry) * (idx + 1));
	parEntry->tag = OCCUPIED;
	parEntry->rid.pageNum = newPageNum;
	parEntry->rid.slotNum = IX_USELESS_SLOTNUM;
	
	memcpy(par_key + attrLen * (idx + 1), child_key + attrLen * (childKeyNum / 2), attrLen);

	parentHdr->keynum++;

	if ((rc = MarkDirty(&childPageHandle)) ||
		(rc = MarkDirty(&newPageHandle)) ||
		(rc = MarkDirty(parent)) ||
		(rc = UnpinPage(&childPageHandle)) ||
		(rc = UnpinPage(&newPageHandle)))
		return rc;

	return SUCCESS;
}

//
// InsertEntry 的帮助函数
//
RC InsertEntryIntoTree(IX_IndexHandle* indexHandle, PageNum node, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle nodePage;
	char* data, *entry, *key;
	IX_NodePageHeader *nodePageHdr;
	int keyNum;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage)) ||
		(rc = GetData(&nodePage, &data)))
		return rc;

	nodePageHdr = (IX_NodePageHeader*)data;
	keyNum = nodePageHdr->keynum;
	entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if (nodePageHdr->is_leaf) {
		// 当前节点是叶子节点
		// 1. 查找叶子节点中是否存在相同key的slot
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		if (findRes) {
			// 存在该key
			// 获取对应索引位置 entry 区的信息
			IX_NodeEntry* pIdxEntry = (IX_NodeEntry*)(entry + sizeof(IX_NodeEntry) * idx);
			// 读取该项的标志位 char tag
			char tag = pIdxEntry->tag;
			RID* thisRid = &(pIdxEntry->rid);
			PageNum bucketPageNum;
			PF_PageHandle bucketPageHandle;

			assert(tag != 0);

			if (tag == OCCUPIED) {
				// 尚未标记为重复键值, 需要创建一个 Bucket File
				// 获取新分配的 Bucket Page 的 pageHandle,并插入记录
				if ((rc = CreateBucket(indexHandle, &bucketPageNum)) || 
					(rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *thisRid)) || 
					(rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *rid)))
					return rc;

				pIdxEntry->tag = DUPLICATE;
				thisRid->pageNum = bucketPageNum;
				thisRid->slotNum = IX_USELESS_SLOTNUM;

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return SUCCESS;

			} else if (tag == DUPLICATE) {
				// 已经标记为重复键值，获取对应的 Bucket File，插入记录
				bucketPageNum = thisRid->pageNum;
				if (rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *rid))
					return rc;

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return SUCCESS;
			}
		} else {
			// 不存在
			// idx 中保存了 <= pData 的索引位置
			// 需要把 idx + 1 之后的 指针区 和 关键字区 的内容向后移动
			// 再把数据 RID 和 关键字 写到 idx + 1 位置
			shift(indexHandle, entry, key, idx + 1, keyNum, true);

			IX_NodeEntry* pIdxEntry = (IX_NodeEntry*)(entry + sizeof(IX_NodeEntry) * (idx + 1));

			pIdxEntry->tag = OCCUPIED;
			pIdxEntry->rid = *rid;

			int parentKeyNum = ++(nodePageHdr->keynum);

			if ((rc = MarkDirty(&nodePage)) ||
				(rc = UnpinPage(&nodePage)))
				return rc;

			return (parentKeyNum >= indexHandle->fileHeader.order) ? IX_CHILD_NODE_OVERFLOW : SUCCESS;
		}
	} else {
		// 当前节点是内部节点
		// 找到 小于等于 输入关键字的最大索引位置，递归执行 InsertEntryIntoTree
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		IX_NodeEntry* nodeEntry = (IX_NodeEntry*)entry;

		PageNum child;
		if (idx == -1)
			child = nodePageHdr->firstChild;
		else
			child = nodeEntry[idx].rid.pageNum;

		// 在深入 B+树 进行递归之前，释放掉当前 page
		if (rc = UnpinPage(&nodePage))
			return rc;

		rc = InsertEntryIntoTree(indexHandle, child, pData, rid);

		if (rc == IX_CHILD_NODE_OVERFLOW) {
			// 子节点发生了 overflow
			// Get 当前节点的 Page
			if (rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage))
				return rc;
			// split 子节点
			if (rc = splitChild(indexHandle, &nodePage, idx, child))
				return rc;
			
			if (rc = GetData(&nodePage, &data))
				return rc;

			int parentKeyNum = ((IX_NodePageHeader*)data)->keynum;

			if (rc = UnpinPage(&nodePage))
				return rc;

			return (parentKeyNum >= indexHandle->fileHeader.order) ? IX_CHILD_NODE_OVERFLOW : SUCCESS;
		}

		return rc;
	}

	return SUCCESS;
}

// 
// 目的: 向 index file 中插入新的 key - rid 对
//
RC InsertEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle newRootHandle;
	char* data;
	IX_NodePageHeader* newRootHdr;

	rc = InsertEntryIntoTree(indexHandle, indexHandle->fileHeader.rootPage, pData, rid);

	if (rc == IX_CHILD_NODE_OVERFLOW) {
		// 根节点满了
		// 创建一个新的 page 作为新的根节点
		if ((rc = AllocatePage(&(indexHandle->fileHandle), &newRootHandle)) || 
			(rc = GetData(&newRootHandle, &data)))
			return rc;

		newRootHdr = (IX_NodePageHeader*)data;
		newRootHdr->firstChild = indexHandle->fileHeader.rootPage;
		newRootHdr->is_leaf = false;
		newRootHdr->keynum = 0;
		newRootHdr->sibling = IX_NO_MORE_NEXT_LEAF;

		if (rc = splitChild(indexHandle, &newRootHandle, -1, newRootHdr->firstChild))
			return rc;

		indexHandle->fileHeader.rootPage = newRootHandle.pFrame->page.pageNum;
		indexHandle->isHdrDirty = true;

		if (rc = UnpinPage(&newRootHandle))
			return rc;

		return SUCCESS;
	}

	return rc;
}

RC DeleteEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{
	return SUCCESS;
}