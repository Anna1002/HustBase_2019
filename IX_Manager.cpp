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
// Ŀ��: ����һ�� Index File��
//       �� Index File ���� ����Ϊ attrlength ����Ϊ attrType �� key
// ʵ�ַǳ����� RM_Manager �е� CreateFile
// 1. �����һ�� Page ���ܹ���ŵ� key entry �ṹ������������˵��ʼ�� B+�� �� order
// 2. ���� PF_Manager::CreateFile() �� Paged File ����ؿ�����Ϣ���г�ʼ��
// 3. ���� PF_Manager::OpenFile() �򿪸��ļ� ��ȡ PF_FileHandle
// 4. ͨ���� PF_FileHandle �� AllocatePage ���������ڴ滺���� �õ��� 1 ҳ�� pData ָ��
// 5. ��ʼ��һ�� IX_FileHdr �ṹ��д�� pData ָ����ڴ���
// 6. ͨ���� PF_FileHandle �� AllocatePage ���������ڴ滺���� �õ��� 2 ҳ�� pData ָ��
// 7. ��ʼ��һ�� IX_NodeHdr �ṹ��д�� pData ָ����ڴ���, ��ʼ��һ���յ� Root �ڵ�
// 6. ��� �� 1��2 ҳ Ϊ��
// 7. PF_Manager::CloseFile() ������ ForceAllPage
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

	// B+�� ��С order ��ҪΪ 3�� split���������Ҫ��һ�� node �д��� 3�� key��¼
	maxKeysNum = (PF_PAGE_SIZE - sizeof(IX_NodePageHeader)) 
					/ (sizeof(IX_NodeEntry) + attrLength);
	maxBucketEntryNum = (PF_PAGE_SIZE - sizeof(IX_BucketPageHeader)) / sizeof(IX_BucketEntry);

	if (maxKeysNum < 3)
		return IX_INVALIDKEYSIZE;

	// 2. ���� PF_Manager::CreateFile() �� Paged File ����ؿ�����Ϣ���г�ʼ��
	// 3. ���� PF_Manager::OpenFile() �򿪸��ļ� ��ȡ PF_FileHandle
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
// Ŀ��: ���Ѿ������� Index File������ʼ��һ�� IX_IndexHandle
// 1. ���Ϸ���
// 2. ���� PF_Manager::OpenFile�����һ�� PF_FileHandle
// 3. ͨ�� PF_FileHandle ����ȡ1��ҳ���ϵ� RM_FileHdr ��Ϣ
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
// Ŀ��: �ر��ļ������޸ĵ�����ˢд������
// ��� indexHandle->isHdrDirty �Ƿ�Ϊ��
// ���Ϊ�棬��ҪGetThisPage����ļ���1ҳ����indexHandle��ά����IX_FileHeader
// ���浽��Ӧ�Ļ�����
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
// Ŀ��: ��ʼ��һ�� Bucket Page������ʼ���õ� Bucket Page �� pageNum ����
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
// Ŀ��: ������� bucket Page(s) д�� RID
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
			// ���� free slot
			// д�� RID
			int free_idx = bucketPageHdr->firstFreeSlot;
			int next_free_idx = entry[free_idx].nextFreeSlot;
			entry[free_idx].nextFreeSlot = bucketPageHdr->firstValidSlot;
			entry[free_idx].rid = rid;
			bucketPageHdr->firstValidSlot = free_idx;
			bucketPageHdr->firstFreeSlot = next_free_idx;

			bucketPageHdr->slotNum++;

			// �����
			if ((rc = MarkDirty(&bucketPage)) ||
				(rc = UnpinPage(&bucketPage)))
				return rc;

			return SUCCESS;
		}

		bucketPageNum = bucketPageHdr->nextBucket;
	}

	// ��������ȫ���� Bucket Page ��û�и�ԣ�ռ���
	// ����һ���µ�
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
// Ŀ��: �� Bucket Page(s) �в���ƥ��� rid����ɾ����Ӧ�� rid
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

	// ��������Ҳû���ҵ�
	return IX_DELETE_NO_RID;
}

//
// Ŀ��: �� Node �ļ�ҳ�� ����ָ�� pData
//       �ҵ�����С�ڵ��� pData ������λ�ñ����� idx ��
//       ������ڵ��� ���� true ���򷵻� false
// pData: �û�ָ��������ָ��
// key: Node �ļ��Ĺؼ������׵�ַָ��
// s: ������Χ��ʼ����
// e: ������Χ��ֹ����
// ������ΧΪ����
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
// shift ��������ҳ�棬������ҳ����
// entry: ҳ��ָ�����׵�ַ
// key:   ҳ��ؼ������׵�ַ
// idx:   ��idx��ʼ������λ�� len - 1 �����ƶ�
// dir:   true ��ʾ ����ƶ�; false ��֮
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
// splitChild ������ parent ҳ����ͷţ����Ǹ����ӽڵ�ҳ�����ҳ����ͷ�
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
		// �ӽڵ���һ��Ҷ�ӽڵ�
		// ��Ҷ�ӽڵ�� childKeyNum / 2 ��ʼ��������Ϣ�������½ڵ�λ��
		memcpy(new_entry, child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2),
			sizeof(IX_NodeEntry) * (childKeyNum - (childKeyNum / 2)));
		memcpy(new_key, child_key + attrLen * (childKeyNum / 2),
			attrLen * (childKeyNum - (childKeyNum / 2)));

		newPageHdr->firstChild = IX_NULL_CHILD;
		newPageHdr->is_leaf = true;
		newPageHdr->keynum = childKeyNum - (childKeyNum / 2);
		newPageHdr->sibling = childHdr->sibling;
	} else {
		// �ӽڵ����ڲ��ڵ�
		// ��Ҷ�ӽڵ�� childKeyNum / 2 + 1 ��ʼ��������Ϣ�������½ڵ�λ��
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

	// �ƶ����ڵ�����
	shift(indexHandle, par_entry, par_key, idx + 1, parentHdr->keynum, true);
	// �Ѻ��ӽڵ���м�����д�뵽���ڵ�
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
// InsertEntry �İ�������
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
		// ��ǰ�ڵ���Ҷ�ӽڵ�
		// 1. ����Ҷ�ӽڵ����Ƿ������ͬkey��slot
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		if (findRes) {
			// ���ڸ�key
			// ��ȡ��Ӧ����λ�� entry ������Ϣ
			IX_NodeEntry* pIdxEntry = (IX_NodeEntry*)(entry + sizeof(IX_NodeEntry) * idx);
			// ��ȡ����ı�־λ char tag
			char tag = pIdxEntry->tag;
			RID* thisRid = &(pIdxEntry->rid);
			PageNum bucketPageNum;
			PF_PageHandle bucketPageHandle;

			assert(tag != 0);

			if (tag == OCCUPIED) {
				// ��δ���Ϊ�ظ���ֵ, ��Ҫ����һ�� Bucket File
				// ��ȡ�·���� Bucket Page �� pageHandle,�������¼
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
				// �Ѿ����Ϊ�ظ���ֵ����ȡ��Ӧ�� Bucket File�������¼
				bucketPageNum = thisRid->pageNum;
				if (rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *rid))
					return rc;

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return SUCCESS;
			}
		} else {
			// ������
			// idx �б����� <= pData ������λ��
			// ��Ҫ�� idx + 1 ֮��� ָ���� �� �ؼ����� ����������ƶ�
			// �ٰ����� RID �� �ؼ��� д�� idx + 1 λ��
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
		// ��ǰ�ڵ����ڲ��ڵ�
		// �ҵ� С�ڵ��� ����ؼ��ֵ��������λ�ã��ݹ�ִ�� InsertEntryIntoTree
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		IX_NodeEntry* nodeEntry = (IX_NodeEntry*)entry;

		PageNum child;
		if (idx == -1)
			child = nodePageHdr->firstChild;
		else
			child = nodeEntry[idx].rid.pageNum;

		// ������ B+�� ���еݹ�֮ǰ���ͷŵ���ǰ page
		if (rc = UnpinPage(&nodePage))
			return rc;

		rc = InsertEntryIntoTree(indexHandle, child, pData, rid);

		if (rc == IX_CHILD_NODE_OVERFLOW) {
			// �ӽڵ㷢���� overflow
			// Get ��ǰ�ڵ�� Page
			if (rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage))
				return rc;
			// split �ӽڵ�
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
// Ŀ��: �� index file �в����µ� key - rid ��
//
RC InsertEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle newRootHandle;
	char* data;
	IX_NodePageHeader* newRootHdr;

	rc = InsertEntryIntoTree(indexHandle, indexHandle->fileHeader.rootPage, pData, rid);

	if (rc == IX_CHILD_NODE_OVERFLOW) {
		// ���ڵ�����
		// ����һ���µ� page ��Ϊ�µĸ��ڵ�
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