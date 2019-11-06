#ifndef SYS_MANAGER_H_H
#define SYS_MANAGER_H_H

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "str.h"
#include "EditArea.h"
#include <string>
#include <map>
#include <cassert>
#include <vector>
#include <set>
#include <algorithm>

#define TABLE_META_NAME "SYSTABLES.xx"
#define COLUMN_META_NAME "SYSCOLUMNS.xx"
#define RM_FILE_SUFFIX ""
#define IX_FILE_SUFFIX ""
//SYSTABLE�м�¼ÿһ��ĳ���
#define SIZE_TABLE_NAME 21
#define TABLE_NAME_OFF 0
#define SIZE_ATTR_COUNT 4
#define ATTR_COUNT_OFF (TABLE_NAME_OFF+SIZE_TABLE_NAME)
#define SIZE_SYS_TABLE 25
//SYSCOLUMNS�м�¼ÿһ��ĳ���
#define SIZE_ATTR_NAME 21
#define ATTR_NAME_OFF (TABLE_NAME_OFF+SIZE_TABLE_NAME)
#define SIZE_ATTR_TYPE 4
#define ATTR_TYPE_OFF (ATTR_NAME_OFF+SIZE_ATTR_NAME)
#define SIZE_ATTR_LENGTH 4
#define ATTR_LENGTH_OFF (ATTR_TYPE_OFF+SIZE_ATTR_TYPE)
#define SIZE_ATTR_OFFSET 4
#define ATTR_OFFSET_OFF (ATTR_LENGTH_OFF+SIZE_ATTR_LENGTH)
#define SIZE_IX_FLAG 1
#define ATTR_IXFLAG_OFF (ATTR_OFFSET_OFF+SIZE_ATTR_OFFSET)
#define SIZE_INDEX_NAME 21
#define ATTR_INDEX_NAME_OFF (ATTR_IXFLAG_OFF+SIZE_IX_FLAG)
#define SIZE_SYS_COLUMNS 76


#define MAX_CON_LEN 100 

typedef struct db_info {
	RM_FileHandle sysTables;
	RM_FileHandle sysColumns;
	int MAXATTRS = 20;		 //�����������
	std::string curDbName; //��ŵ�ǰDB����
}DB_INFO;


typedef struct {
	AttrType attrType;
	int attrLength;
	int attrOffset;
	bool ix_flag;
	std::string indexName;
	std::string attrName;
} AttrEntry;

typedef struct {
	int attrOffset;
	IX_IndexHandle ixIndexHandle;
} IxEntry;

void ExecuteAndMessage(char*, CEditArea*);
bool CanButtonClick();

RC CreateDB(char* dbpath, char* dbname);
RC DropDB(char* dbname);
RC OpenDB(char* dbname);
RC CloseDB();

RC execute(char* sql);

RC CreateTable(char* relName, int attrCount, AttrInfo* attributes);
RC DropTable(char* relName);
RC CreateIndex(char* indexName, char* relName, char* attrName);
RC DropIndex(char* indexName);
RC Insert(char* relName, int nValues, Value* values);
RC Delete(char* relName, int nConditions, Condition* conditions);
RC Update(char* relName, char* attrName, Value* value, int nConditions, Condition* conditions);

// һЩ�ǽӿڷ���
// SYSTABLES Ԫ���ݱ����
RC TableMetaInsert(char* relName, int attrCount);
RC TableMetaDelete(char* relName);
RC TableMetaSearch(char* relName, RM_Record* rmRecord);
RC TableMetaShow();

// SYSCOLUMNS Ԫ���ݱ����
RC ColumnSearchAttr(char* relName, char* attrName, RM_Record* rmRecord);
bool attrVaild(int attrCount, AttrInfo* attributes);
RC ToData(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName, char* pData);
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName);
RC ColumnMetaDelete(char* relName);
RC ColumnMetaUpdate(char* relName, char* attrName, bool ixFlag, char* indexName);
RC ColumnMetaGet(char* relName, char* attrName, AttrEntry* attribute);
RC ColumnMetaShow();                                                 // ��ӡ�� column Ԫ���ݱ�
// ��� relName ��Ч��ͬ��
// ��ȡ����Ϣ������ attribute ��
RC ColumnEntryGet(char* relName, int* attrCount,
	std::vector<AttrEntry>& attributes);                      // ͨ�� table Ԫ���� �� column Ԫ����
															  // ���һ�����ȫ��������Ϣ, ���ڽ������ͼ��
															  // CreateTable �� �����

//��װ��RM����
RC CreateIxFromTable(char* relName, char* indexName, int attrOffset);
RC CreateConFromCondition(char* relName, int nConditons, Condition* conditions, Con* cons);
bool CheckCondition(char* relName, Condition& condition);
bool checkAttr(char* relName, int hsIsAttr, RelAttr& hsAttr, AttrType* attrType);
RC InsertRmAndIx(RM_FileHandle* rmFileHandle, std::vector<IxEntry>& ixEntrys, char* pData);
RC DeleteRmAndIx(RM_FileHandle* rmFileHandle, std::vector<IxEntry>& ixEntrys, RM_Record* delRecord);
RC GetRecordSize(char* relName, int* recordSize);
RC ShowTable(char* relName);
RC ShowIndex(char* relName, char* attrName, bool def, int cutLen);

#endif