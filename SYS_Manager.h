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

// table Ԫ���ݱ��ֶγ���
const int TABLENAME = 21;
const int ATTRCOUNT = 4;
// table Ԫ���ݸ����ֶ�ƫ��
const int TABLENAME_OFF = 0;
const int ATTRCOUNT_OFF = TABLENAME_OFF + TABLENAME;

// column Ԫ���ݱ��ֶγ���
// const int TABLENAME = 21;
const int ATTRNAME = 21;
const int ATTRTYPE = 4;
const int ATTRLENGTH = 4;
const int ATTROFFSET = 4;
const int IXFLAG = 1;
const int INDEXNAME = 21;
// column Ԫ���ݱ�����ֶ�ƫ��
// const int TABLENAME_OFF = 0;
const int ATTRNAME_OFF = TABLENAME_OFF + TABLENAME;
const int ATTRTYPE_OFF = ATTRNAME_OFF + ATTRNAME;
const int ATTRLENGTH_OFF = ATTRTYPE_OFF + ATTRTYPE;
const int ATTROFFSET_OFF = ATTRLENGTH_OFF + ATTRLENGTH;
const int IXFLAG_OFF = ATTROFFSET_OFF + ATTROFFSET;
const int INDEXNAME_OFF = IXFLAG_OFF + IXFLAG;

typedef struct {
	RM_FileHandle sysTable;
	RM_FileHandle sysColumn;

	std::string curWorkPath;        // DB �ļ����ϲ�Ŀ¼·��
	std::string curDBPath;
} WorkSpace;

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
// table Ԫ���ݱ����
//
// relName: ������
// attrCount: ��������
// RC �ɵײ� RM insert �������з���
// 
RC TableMetaInsert(char* relName, int attrCount);                    // ����һ�� table name ��¼
//
// relName: ������
// �ж� relName �Ƿ����, �����ڷ��� TABLE_NOT_EXIST
// ��� RC �ɵײ� RM Scan/delete �������з���
// �ײ�ʵ�ֱ�֤ �߲��ɾ Ӧ���ǰ�ȫ��
// 
RC TableMetaDelete(char* relName);                                   // ɾ��һ�� table name ��¼
//
// relName: ������
// �ж� relName �Ƿ����, �����ڷ��� TABLE_NOT_EXIST
// ���� ���� SUCCESS
// 
RC TableMetaSearch(char* relName);                                   // �ж�һ�� table name �Ƿ����
// RC ���� RM Scan �����з����Ĵ��� (��ȥ RM_EOF)
// ��� Scan ���� Success
// �����ʽ�����ÿ�Щ������ mysql ������
// 
RC TableMetaShow();                                                  // ��ӡ�� table Ԫ���ݱ�

// column Ԫ���ݱ����
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool idx, char* indexName);      // ����һ�� column ��¼
// ��� relName ��Ч��ͬ��
RC ColumnMetaDelete(char* relName);                                  // ɾ��ͬһ�����е�������������Ϣ
                                                                     // RM ��ʵ������߲��ɾ
// ��� relName ��Ч��ͬ��
RC ColumnMetaUpdate(char* relName, char* attrName, int metaOffset, 
	                AttrType metaType, void *value);                 // ���� column Ԫ���ݱ��е�ĳһ���¼��������� 
// ��� relName ��Ч��ͬ��
// ��ȡ����Ϣ������ attribute ��
RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute);// ��ȡĳ�� �� �е� ĳһ�����Ե� ���͡�����
// ͬ��
RC ColumnMetaShow();                                                 // ��ӡ�� column Ԫ���ݱ�

// ��� relName ��Ч��ͬ��
// ��ȡ����Ϣ������ attribute ��
RC MetaGet(char* relName, int attrCount, AttrInfo* attributes);      // ͨ�� table Ԫ���� �� column Ԫ����
                                                                     // ���һ�����ȫ��������Ϣ, ���ڽ������ͼ��
                                                                     // CreateTable �� �����

#endif