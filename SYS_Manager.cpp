#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>
#include <set>

WorkSpace workSpace;

//����SYS_Manager.h�лᷢ����ͻ
typedef struct db_info {
	std::vector< RM_FileHandle* > sysFileHandle_Vec; //����ϵͳ�ļ��ľ��
	std::map<std::string, RM_FileHandle*> rmFileHandle_Map; //���ļ���ӳ�䵽��Ӧ�ļ�¼�ļ����
	std::map<std::string, IX_IndexHandle*> ixIndexHandle_Map; //���ļ���ӳ�䵽��Ӧ�������ļ����

	int MAXATTRS = 20;		 //�����������
	char curDbName[300] = ""; //��ŵ�ǰDB����
	char path[300] = "C:\C++\HBASE\DB";		 //�������DB�������ϼ�Ŀ¼
}DB_INFO;
DB_INFO dbInfo;

void ExecuteAndMessage(char * sql,CEditArea* editArea){//����ִ�е���������ڽ�������ʾִ�н�����˺������޸�
	std::string s_sql = sql;
	if(s_sql.find("select") == 0){
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//����ѯ�������һ�£����������������ʽ
		//����editArea->ShowSelResult(col_num,row_num,fields,rows);
		int col_num = 5;
		int row_num = 3;
		char ** fields = new char *[5];
		for(int i = 0;i<col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],0,20);
			fields[i][0] = 'f';
			fields[i][1] = i+'0';
		}
		char *** rows = new char**[row_num];
		for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];
			for(int j = 0;j<col_num;j++){
				rows[i][j] = new char[20];
				memset(rows[i][j],0,20);
				rows[i][j][0] = 'r';
				rows[i][j][1] = i + '0';
				rows[i][j][2] = '+';
				rows[i][j][3] = j + '0';
			}
		}
		editArea->ShowSelResult(col_num,row_num,fields,rows);
		for(int i = 0;i<5;i++){
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	RC rc = execute(sql);
	int row_num = 0;
	char**messages;
	switch(rc){
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "�����ɹ�";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "���﷨����";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "����δʵ��";
		editArea->ShowMessage(row_num,messages);
	delete[] messages;
		break;
	}
}

RC execute(char * sql){
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
  	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX
	
	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////�ж�SQL���Ϊselect���

			//break;

			case 2:
			//�ж�SQL���Ϊinsert���

			case 3:	
			//�ж�SQL���Ϊupdate���
			break;

			case 4:					
			//�ж�SQL���Ϊdelete���
			break;

			case 5:
			//�ж�SQL���ΪcreateTable���
			break;

			case 6:	
			//�ж�SQL���ΪdropTable���
			break;

			case 7:
			//�ж�SQL���ΪcreateIndex���
			break;
	
			case 8:	
			//�ж�SQL���ΪdropIndex���
			break;
			
			case 9:
			//�ж�Ϊhelp��䣬���Ը���������ʾ
			break;
		
			case 10: 
			//�ж�Ϊexit��䣬�����ɴ˽����˳�����
			break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
		return rc;
	}
}


//
// Ŀ�ģ���·�� dbPath �´���һ����Ϊ dbName �Ŀտ⣬������Ӧ��ϵͳ�ļ���
// 1. ��OS������dbpath·�������ļ��С�
// 2. ����SYSTABLES�ļ���SYSCOLUMNS�ļ�
// 
RC CreateDB(char *dbpath,char *dbname)
{
	RC rc;
	//1. ���dbpath��dbname�ĺϷ��ԡ�
	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	if (workSpace.curWorkPath.size() && workSpace.curDBName.size())
		if (workSpace.curWorkPath.compare(dbpath) != 0 || 
			workSpace.curDBName.compare(dbname) != 0)
			if (rc = CloseDB())
				return rc;

	//3. ��OS������dbpath·�������ļ��С�
	std::string dbPath = dbpath;
	dbPath += "\\";
	dbPath += dbname;
	if (CreateDirectory(dbPath.c_str(), NULL)) {
		if (SetCurrentDirectory(dbpath)) {
			//4. ����SYSTABLES�ļ���SYSCOLUMNS�ļ�
			std::string sysTablePath = dbPath + TABLE_META_NAME;
			std::string sysColumnsPath = dbPath + COLUMN_META_NAME;
			if ((rc = RM_CreateFile((char*)sysTablePath.c_str(), TABLE_ENTRY_SIZE)) ||
				(rc = RM_CreateFile((char*)sysColumnsPath.c_str(), COL_ENTRY_SIZE)))
				return rc;

			//5. ����dbPath��curDbName��
			workSpace.curWorkPath = dbpath;
			workSpace.curDBName = dbname;
			return SUCCESS;
		}
		return OSFAIL;
	}

	return OSFAIL;
}

//
// Ŀ�ģ�ɾ�����ݿ��Ӧ��Ŀ¼�Լ�Ŀ¼�µ������ļ�
//       ���ڲ��������ļ��У����Բ���Ҫ���еݹ鴦��
// 1. �ж�ɾ�����Ƿ��ǵ�ǰDB�������Ҫ�رյ�ǰĿ¼.
// 2. ���������õ�dbnameĿ¼�µ������ļ�����ɾ��,ע��Ҫ����.��..Ŀ¼
// 3. ɾ��dbnameĿ¼
// 
RC DropDB(char *dbname)
{
	//1. �ж�dbInfo.path·�����Ƿ����dbname������������򱨴�
	std::string dbPath = workSpace.curWorkPath + "\\" + dbname;
	if (!workSpace.curWorkPath.size() || access(dbPath.c_str(), 0) == -1)
		return DB_NOT_EXIST;

	//2. �ж�ɾ�����Ƿ��ǵ�ǰDB
	if (workSpace.curDBName.compare(dbname) == 0) {
		//�رյ�ǰĿ¼
		RC rc;
		if (rc = CloseDB())
			return rc;
	}

	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	dbPath += "\\*.*";
	hFile = FindFirstFile(dbPath.c_str(), &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return OSFAIL;

	//3. ����ɾ���ļ����µ������ļ�
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//����.��..Ŀ¼
			continue;
		if (!DeleteFile(pNextInfo.cFileName))
			return OSFAIL;
	}

	//4. ɾ��dbnameĿ¼
	if (!RemoveDirectory(dbname))
		return OSFAIL;

	return SUCCESS;
}

//
// Ŀ�ģ��ı�ϵͳ�ĵ�ǰ���ݿ�Ϊ dbName ��Ӧ���ļ����е����ݿ�
// 1. ���ý��̵�ǰ����Ŀ¼Ϊdbnmae
// 2. �رյ�ǰ�򿪵�Ŀ¼��
// 3. ��SYSTABLES��SYSCOLUMNS������sysFileHandle_Vec
// 4. ����curDbName
// 
RC OpenDB(char *dbname)
{
	RC rc;

	//1.  �رյ�ǰ�򿪵�Ŀ¼����SYSTABLES��SYSCOLUMNS������sysFileHandle_Vec
	if (workSpace.curDBName.size() && (rc = CloseDB()))
		return rc;

	std::string sysTablePath = dbname;
	sysTablePath += "\\";
	sysTablePath += TABLE_META_NAME;
	std::string sysColumnsPath = dbname;
	sysColumnsPath += "\\";
	sysColumnsPath += COLUMN_META_NAME;
	if ((rc = RM_OpenFile((char*)sysTablePath.c_str(), &(workSpace.sysTable))) ||
		(rc = RM_OpenFile((char*)sysColumnsPath.c_str(), &(workSpace.sysColumn))))
		return rc;

	//3. ����curDbName
	workSpace.curDBName = dbname;

	return SUCCESS;
}

//
// Ŀ��:�رյ�ǰ���ݿ⡣�رյ�ǰ���ݿ��д򿪵������ļ�
// 1. ��鵱ǰ�Ƿ����DB������鱣���curDbName�Ƿ�Ϊ�ա�
// 2. ���ñ������ж�Ӧ��Close������
//	 2.1 �ر�RM�ļ�
//	 2.2 �ر�IX�ļ�
//	 2.3 �ر�SYS�ļ�
// 3. �л����ϼ�Ŀ¼.
//
RC CloseDB()
{
	//1. ��鵱ǰ�Ƿ����DB�����curDbNameΪ���򱨴���������curDbNameΪ�ա�
	if (!workSpace.curDBName.size())
		return NO_DB_OPENED;
	workSpace.curDBName.clear();

	//2. ���ñ����SYS�ļ����close�������ر�SYS�ļ���
	RM_CloseFile(&(workSpace.sysTable));
	RM_CloseFile(&(workSpace.sysColumn));

	return SUCCESS;
}

bool CanButtonClick()
{
	// �����ǰ�����ݿ��Ѿ���
	// return true;
	// �����ǰû�����ݿ��
	// return false;
	return (workSpace.curDBName.size() != 0);
}


bool attrVaild(int attrCount, AttrInfo* attributes)
{
	std::set<std::string> dupJudger;
	std::set<std::string>::iterator iter;

	for (int i = 0; i < attrCount; i++) {
		if (strlen(attributes[i].attrName) >= ATTRNAME)
			return false;

		std::string attrName(attributes[i].attrName);
		iter = dupJudger.find(attrName);

		if (iter != dupJudger.end())
			return false;

		dupJudger.insert(attrName);
	}

	return true;
}

//
// Ŀ��:����һ����Ϊ relName �ı�
// 1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
// 2. ����Ƿ��Ѿ�����relName������Ѿ����ڣ��򷵻ش���
// 3. ���atrrCount�Ƿ���� MAXATTRS��
// 4. ��SYSTABLE��SYSCOLUMNS���д���Ԫ��Ϣ
// 5. �����¼�Ĵ�С��������Ӧ��RM�ļ�
// 
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) 
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[TABLE_ENTRY_SIZE];
	memset(rmRecord.pData, 0, TABLE_ENTRY_SIZE);

	int tableRecLength = 0;
	// 1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (!workSpace.curDBName.size()) {
		delete[] rmRecord.pData;
		return NO_DB_OPENED;
	}

	// ��� relName �Ƿ�Ϸ�
	if (strlen(relName) >= TABLENAME ||
		!strcmp(relName, TABLE_META_NAME) ||
		!strcmp(relName, COLUMN_META_NAME)) {
		delete[] rmRecord.pData;
		return SQL_SYNTAX;
	}

	if ((rc = TableMetaSearch(relName, &rmRecord)) != TABLE_NOT_EXIST) {
		delete[] rmRecord.pData;
		return SQL_SYNTAX;
	}

	// ��� attr �Ƿ�Ϸ�
	if (!attrVaild(attrCount, attributes)) {
		delete[] rmRecord.pData;
		return SQL_SYNTAX;
	}

	// �� column Ԫ���� ���룬������ �ñ�ļ�¼����
	for (int i = 0; i < attrCount; i++) {
		if (rc = ColumnMetaInsert(relName, attributes[i].attrName, attributes[i].attrType,
			attributes[i].attrLength, tableRecLength, false, NULL)) {
			delete[] rmRecord.pData;
			return rc;
		}
		tableRecLength += attributes[i].attrLength;
	}

	// �� table Ԫ���� ����
	if (rc = TableMetaInsert(relName, attrCount)) {
		delete[] rmRecord.pData;
		return rc;
	}

	// ���� RM �ļ�
	std::string rmPath = workSpace.curDBName + "\\";
	rmPath += relName;
	rmPath += REC_FILE_SUFFIX;

	if (rc = RM_CreateFile((char*)(rmPath.c_str()), tableRecLength)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//Ŀ�ģ�������Ϊ relName �ı��Լ��ڸñ��Ͻ���������������
//1. ��鵱ǰ�Ƿ��Ѿ�����һ�����ݿ⣬���û���򱨴�
//2. �ж��Ƿ���ڱ�relName.
//	 2.1. ����Ѿ������򱨴�
//	 3.2. �����SYSTABLES��ɾ��relName��Ӧ��һ���¼
//3. ɾ��rm�ļ���
//	 3.1.��dbInfo��rmFileHandle_Map�в���relName��Ӧ��rm�ļ��Ƿ񱻴򿪣�����Ѿ��������close����������Map��ɾ����
//	 3.2.ɾ��relName��Ӧ��rm�ļ�
//4. ɾ��ix�ļ�
//	 4.1.��ȡSYSCOLUMNS���õ�relName��atrrname��
//	 4.2.�ж��Ƿ񴴽������������������������
//		 4.2.1 ��dbInfo��ixIndexHandle_Map�в���relName_atrrname��Ӧ��ix�ļ��Ƿ񱻴򿪣�����Ѿ��������close����������Map��ɾ����
//		 4.2.2 ɾ��relName_atrrname��Ӧ��ix�ļ�
//	 4.3.ɾ��SYSCOLUMNS��atrrname��Ӧ�ļ�¼��
RC DropTable(char* relName) 
{
	// 1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (!workSpace.curDBName.size()) {
		return NO_DB_OPENED;
	}



	return SUCCESS;
}

//�ú����ڹ�ϵ relName ������ attrName �ϴ�����Ϊ indexName ��������
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
//2. ����Ƿ��Ѿ����ڶ�Ӧ������������Ѿ������򱨴�
//3. ������Ӧ��������
//	 3.1. ��ȡrelName,atrrName��SYSCOLUMNS�еļ�¼��õ�attrType��attrLength
//	 3.2. ����IX�ļ�
//	 3.3. ����SYS_COLUMNS
RC CreateIndex(char* indexName, char* relName, char* attrName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

	char ixFileName[45];//relname+"_"+attrName+".ix"-1
	memset(ixFileName, 0, 42);
	strcat(ixFileName,relName);
	strcat(ixFileName, "_");
	strcat(ixFileName, attrName);
	strcat(ixFileName, ".ix");
	
	if (strcmp(indexName, ixFileName)) {//���indexName��Լ�����Ƿ�һ�¡�
		return SQL_SYNTAX;
	}

	//2. ����Ƿ��Ѿ����ڶ�Ӧ������������Ѿ������򱨴�
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[3];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 21; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;

	conditions[1].attrType = chars; conditions[1].bLhsIsAttr = 1; conditions[1].bRhsIsAttr = 0;
	conditions[1].compOp = EQual; conditions[1].LattrLength = 21; conditions[1].LattrOffset = 21;
	conditions[1].Lvalue = NULL; conditions[1].RattrLength = 21; conditions[1].RattrOffset = 0;
	conditions[1].Rvalue = attrName;

	char str_one[1];//��Ϊix_flag��һ���ֽڵı�ʶλ��������chars�ķ�ʽ���Ƚϡ�
	str_one[0]=1;
	conditions[2].attrType = chars; conditions[2].bLhsIsAttr = 1; conditions[2].bRhsIsAttr = 0;
	conditions[2].compOp = EQual; conditions[2].LattrLength = 1; conditions[2].LattrOffset = + 21 + 21 + 4 + 4 + 4+1;
	conditions[2].Lvalue = NULL; conditions[2].RattrLength = 1; conditions[2].RattrOffset = 0;
	conditions[2].Rvalue = str_one;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 3, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	if (rc == SUCCESS || rmRecord.bValid) {//�Ѿ������˶�Ӧ������
		return SQL_SYNTAX;
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. ������Ӧ��������
	//	 3.1. ��ȡrelName,atrrName��SYSCOLUMNS�еļ�¼��õ�atrrType
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 2, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	if (rc != SUCCESS || !rmRecord.bValid)
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 3.2. ����IX�ļ�
	int attrType = *((int*)rmRecord.pData + 21 + 21);
	int attrLength = *((int*)rmRecord.pData + 21 + 21 + 4);
	char* ix_Flag = (char*)rmRecord.pData + 21 + 21 + 4 + 4 + 4;
	if (ix_Flag != 0)//���ix_flag����0
		return SQL_SYNTAX;
	//RC CreateIndex(const char * fileName,AttrType attrType,int attrLength);
	if(CreateIndex(ixFileName,(AttrType)attrType,attrLength))
		return SQL_SYNTAX;

	//	 3.3. ����SYS_COLUMNS
	*ix_Flag = 1;
	RM_FileHandle* rmFileHandle;
	char rmFileName[24];//relName+".rm"
	memset(rmFileName, 0, 24);
	strcat(rmFileName, relName);
	strcat(rmFileName, ".rm");
	if(	RM_OpenFile(rmFileName,rmFileHandle)||
		UpdateRec(rmFileHandle,&rmRecord)||
		RM_CloseFile(rmFileHandle))
		return SQL_SYNTAX;
	
}

//
//Ŀ�ģ��ú�������ɾ����Ϊ indexName �������� 
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
//2. ��SYSCOLUMNS�м�������Ƿ���ڡ�����������������Լ����indexname����Ψһ�������в��ң�
//	 2.1 ����������򱨴�
//	 2.2.�����޸�SYSCOLUMNS�ж�Ӧ��¼��ix_flagΪ0.
//3. ɾ��ix�ļ�
RC DropIndex(char* indexName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	return SUCCESS;
}

// 
// �� table meta �����¼
// 
RC TableMetaInsert(char* relName, int attrCount)
{
	RC rc;
	RID rid;

	char* factory = new char[TABLE_ENTRY_SIZE];
	memset(factory, 0, sizeof(TABLE_ENTRY_SIZE));

	// װ�� table meta ��Ҫ������
	memcpy(factory, relName, strlen(relName));
	memcpy(factory + ATTRCOUNT_OFF, &attrCount, sizeof(int));

	if (rc = InsertRec(&(workSpace.sysTable), factory, &rid)) {
		delete[] factory;
		return rc;
	}

	delete[] factory;
	return SUCCESS;
}

//
// ���� table meta ���Ƿ����ָ�� relName �ļ�¼
// 
RC TableMetaSearch(char* relName, RM_Record* rmRecord)
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[TABLE_ENTRY_SIZE];
	memset(rec.pData, 0, TABLE_ENTRY_SIZE);
	Con cond;

	if (strlen(relName) >= TABLENAME) {
		delete[] rec.pData;
		return TABLE_NAME_ILLEGAL;
	}

	cond.attrType = chars; cond.bLhsIsAttr = 1; cond.bRhsIsAttr = 0;
	cond.compOp = EQual; cond.LattrLength = TABLENAME; cond.LattrOffset = TABLENAME_OFF;
	cond.Lvalue = NULL; cond.RattrLength = TABLENAME; cond.RattrOffset = 0;
	cond.Rvalue = relName;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysTable), 1, &cond)) {
		delete[] rec.pData;
		return rc;
	}

	if ((rc = GetNextRec(&rmFileScan, &rec)) == RM_EOF) {
		delete[] rec.pData;
		return TABLE_NOT_EXIST;
	}
	if (rc != SUCCESS) {
		delete[] rec.pData;
		return rc;
	}

	memcpy(rmRecord->pData, rec.pData, TABLE_ENTRY_SIZE);
	rmRecord->rid = rec.rid;
	rmRecord->bValid = rec.bValid;

	if ((rc = GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		delete[] rec.pData;
		return FAIL;             // ���������֧���ܴ��������߼�����
	}

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rec.pData;
		return rc;
	}

	delete[] rec.pData;
	return SUCCESS;
}

// 
// �� table meta ����ɾ��ָ�� relName �� ��¼
// 
RC TableMetaDelete(char* relName)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[TABLE_ENTRY_SIZE];
	memset(rmRecord.pData, 0, TABLE_ENTRY_SIZE);

	if (rc = TableMetaSearch(relName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	// ɾ����� rm �ļ�


	if (rc = DeleteRec(&(workSpace.sysTable), &(rmRecord.rid))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC TableMetaShow()
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[TABLE_ENTRY_SIZE];
	memset(rec.pData, 0, TABLE_ENTRY_SIZE);
	int recnt = 0;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysTable), 0, NULL)) {
		delete[] rec.pData;
		return rc;
	}

	printf("Table Meta Table: \n");
	printf("+--------------------+----------+\n");
	printf("|     Table Name     |  AttrCnt |\n");
	printf("+--------------------+----------+\n");
	while ((rc == GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] rec.pData;
			return rc;
		}
		printf("|%-20s|%-10d|\n", rec.pData, *(int*)(rec.pData + ATTRCOUNT_OFF));
		recnt++;
	}
	printf("+--------------------+----------+\n");
	printf("Table Count: %d\n", recnt);

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rec.pData;
		return rc;
	}

	delete[] rec.pData;
	return SUCCESS;
}

// ����һ�� column ��¼
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool idx, char* indexName)
{
	RC rc;
	RID rid;
	char idx_c = (idx) ? 1 : 0;
	
	char* factory = new char[COL_ENTRY_SIZE];
	memset(factory, 0, sizeof(COL_ENTRY_SIZE));

	// װ�� column meta ��Ҫ������
	memcpy(factory, relName, strlen(relName));
	memcpy(factory + ATTRNAME_OFF, attrName, strlen(attrName));
	memcpy(factory + ATTRTYPE_OFF, &attrType, sizeof(int));
	memcpy(factory + ATTRLENGTH_OFF, &attrLength, sizeof(int));
	memcpy(factory + ATTROFFSET_OFF, &attrOffset, sizeof(int));
	memcpy(factory + IXFLAG_OFF, &idx_c, sizeof(char));
	if (indexName)
		memcpy(factory + INDEXNAME_OFF, indexName, strlen(indexName));

	if (rc = InsertRec(&(workSpace.sysColumn), factory, &rid)) {
		delete[] factory;
		return rc;
	}

	delete[] factory;
	return SUCCESS;
}

RC ColumnSearchAttr(char* relName, char* attrName, RM_Record* rmRecord)
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[COL_ENTRY_SIZE];
	memset(rec.pData, 0, COL_ENTRY_SIZE);
	Con cond[2];

	if (strlen(relName) >= TABLENAME ||
		strlen(attrName) >= ATTRNAME) {
		delete[] rec.pData;
		return TABLE_NAME_ILLEGAL;
	}

	cond[0].attrType = chars; cond[0].bLhsIsAttr = 1; cond[0].bRhsIsAttr = 0;
	cond[0].compOp = EQual; cond[0].LattrLength = TABLENAME; cond[0].LattrOffset = TABLENAME_OFF;
	cond[0].Lvalue = NULL; cond[0].RattrLength = TABLENAME; cond[0].RattrOffset = 0;
	cond[0].Rvalue = relName;

	cond[1].attrType = chars; cond[1].bLhsIsAttr = 1; cond[1].bRhsIsAttr = 0;
	cond[1].compOp = EQual; cond[1].LattrLength = ATTRNAME; cond[1].LattrOffset = ATTRNAME_OFF;
	cond[1].Lvalue = NULL; cond[1].RattrLength = ATTRNAME; cond[1].RattrOffset = 0;
	cond[1].Rvalue = attrName;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysColumn), 2, cond)) {
		delete[] rec.pData;
		return rc;
	}

	if ((rc = GetNextRec(&rmFileScan, &rec)) == RM_EOF) {
		delete[] rec.pData;
		return TABLE_NOT_EXIST;
	}
	if (rc != SUCCESS) {
		delete[] rec.pData;
		return rc;
	}

	memcpy(rmRecord->pData, rec.pData, COL_ENTRY_SIZE);
	rmRecord->bValid = rec.bValid;
	rmRecord->rid = rec.rid;

	if ((rc = GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		delete[] rec.pData;
		return FAIL;             // ���������֧���ܴ��������߼�����
	}

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rec.pData;
		return rc;
	}

	delete[] rec.pData;
	return SUCCESS;
}

RC ColumnMetaDelete(char* relName)
{
	// ��� relName ���Ƿ����
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);
	RM_FileScan rmFileScan;
	Con cond;
	char idxName[21];
	char ix_c;

	if (rc = TableMetaSearch(relName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	cond.attrType = chars; cond.bLhsIsAttr = 1; cond.bRhsIsAttr = 0;
	cond.compOp = EQual; cond.LattrLength = TABLENAME; cond.LattrOffset = TABLENAME_OFF;
	cond.Lvalue = NULL; cond.RattrLength = TABLENAME; cond.RattrOffset = 0;
	cond.Rvalue = relName;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysColumn), 1, &cond)) {
		delete[] rmRecord.pData;
		return rc;
	}

	while ((rc = GetNextRec(&rmFileScan, &rmRecord)) != RM_EOF) {
		if (rc != SUCCESS) { // ��������
			delete[] rmRecord.pData;
			return rc;
		}

		// TODO: ɾ�� ix �ļ�
		ix_c = *(rmRecord.pData + IXFLAG_OFF);
		if (ix_c == 1) {
			memcpy(idxName, rmRecord.pData + INDEXNAME_OFF, INDEXNAME);
			std::string indexFilePath = workSpace.curDBName;
			indexFilePath += "\\";
			indexFilePath += idxName;
			indexFilePath += INDEX_FILE_SUFFIX;

			std::cout << "delete index file: " <<  indexFilePath << std::endl;

			if (!DeleteFile(indexFilePath.c_str()))
				return FAIL;
		}

		if (rc = DeleteRec(&(workSpace.sysColumn), &(rmRecord.rid))) {
			delete[] rmRecord.pData;
			return rc;
		}
	}

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColumnMetaUpdate(char* relName, char* attrName, bool ixFlag, char* indexName)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);
	char* pData;
	char ix_c = (ixFlag) ? 1 : 0;

	if (rc = ColumnSearchAttr(relName, attrName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	pData = rmRecord.pData;
	*(pData + IXFLAG_OFF) = ix_c;
	if (indexName)
		memcpy(pData + INDEXNAME_OFF, indexName, strlen(indexName));

	if (rc = UpdateRec(&(workSpace.sysColumn), &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);
	char* pData;
	
	assert(attrName);

	if (rc = ColumnSearchAttr(relName, attrName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	pData = rmRecord.pData;
	attribute->attrName = attrName;
	attribute->attrLength = *(int*)(pData + ATTRLENGTH_OFF);
	attribute->attrType = (AttrType)(*(int*)(pData + ATTRTYPE));

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColumnMetaShow()
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[COL_ENTRY_SIZE];
	memset(rec.pData, 0, COL_ENTRY_SIZE);
	int recnt = 0;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysTable), 0, NULL)) {
		delete[] rec.pData;
		return rc;
	}

	printf("Column Meta Table: \n");
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	printf("|     Table Name     ");
	printf("|   Attribute Name   ");
	printf("| Attr Type ");
	printf("|  AttrLen  ");
	printf("|Attr Offset");
	printf("|  idxFlag  ");
	printf("|     index Name     |\n");
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	while ((rc == GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] rec.pData;
			return rc;
		}
		printf("|%-20s|%-20s|%-10d|%-10d|%-10d|%-10d|%-20s|\n", rec.pData, rec.pData + ATTRNAME_OFF,
			                      *(int*)(rec.pData + ATTRTYPE_OFF), *(int*)(rec.pData + ATTRLENGTH_OFF), 
								  *(int*)(rec.pData + ATTROFFSET_OFF), (int)(*(rec.pData + IXFLAG_OFF)),
			                      rec.pData + INDEXNAME_OFF);
		recnt++;
	}
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	printf("Column Count: %d\n", recnt);

	delete[] rec.pData;
	return SUCCESS;
}

