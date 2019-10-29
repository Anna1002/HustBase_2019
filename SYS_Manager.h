#ifndef SYS_MANAGER_H_H
#define SYS_MANAGER_H_H

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "str.h"
#include <string>
#include <map>
#include <cassert>
#include <vector>

#define TABLE_META_NAME "SYSTABLES"
#define COLUMN_META_NAME "SYSCOLUMNS"
#define RM_FILE_SUFFIX ".rm"
#define IX_FILE_SUFFIX ".ix"
//SYSTABLE��SYSCOLUMNS�м�¼ÿһ��ĳ���
#define SIZE_TABLE_NAME 21
#define SIZE_ATTR_COUNT 4
#define SIZE_ATTR_NAME 21
#define SIZE_ATTR_TYPE 4
#define SIZE_ATTR_LENGTH 4
#define SIZE_ATTR_OFFSET 4
#define SIZE_IX_FLAG 1
#define SIZE_INDEX_NAME 21
#define SIZE_SYS_TABLE 25
#define SIZE_SYS_COLUMNS 76
#define MAX_CON_LEN 100 //���������ֲ�����

typedef struct db_info {
	RM_FileHandle* sysTables;
	RM_FileHandle* sysColumns;
	int MAXATTRS=20;		 //�����������
	std::string curDbName; //��ŵ�ǰDB����
	std::string path;	 //����CreateDB�����·��

}DB_INFO;
DB_INFO dbInfo;

void ExecuteAndMessage(char * ,CEditArea*);
bool CanButtonClick();

RC CreateDB(char *dbpath,char *dbname);
RC DropDB(char *dbname);
RC OpenDB(char *dbname);
RC CloseDB();

RC execute(char * sql);

RC CreateTable(char *relName,int attrCount,AttrInfo *attributes);
RC DropTable(char *relName);
RC CreateIndex(char *indexName,char *relName,char *attrName);
RC DropIndex(char *indexName);
RC Insert(char *relName,int nValues,Value * values);
RC Delete(char *relName,int nConditions,Condition *conditions);
RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions);

// һЩ�ǽӿڷ���
// SYSTABLES Ԫ���ݱ����
RC TableMetaInsert(char* relName, int attrCount); 
RC TableMetaDelete(char* relName);
RC TableMetaSearch(char* relName, RM_Record& rmRecord);
RC TableMetaShow();

// SYSCOLUMNS Ԫ���ݱ����
RC ColumnSearchAttr(char* relName, char* attrName);      
RC ToData(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName, char* pData);
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName);     
RC ColumnMetaDelete(char* relName);                                  
RC ColumnMetaUpdate(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName);      
RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute);
RC ColumnMetaShow();                                                 // ��ӡ�� column Ԫ���ݱ�
// ��� relName ��Ч��ͬ��
// ��ȡ����Ϣ������ attribute ��
RC MetaGet(char* relName, int attrCount, AttrInfo* attributes);      // ͨ�� table Ԫ���� �� column Ԫ����
                                                                     // ���һ�����ȫ��������Ϣ, ���ڽ������ͼ��
                                                                     // CreateTable �� �����

#endif