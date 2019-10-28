#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>

//����SYS_Manager.h�лᷢ����ͻ
typedef struct db_info {
	std::vector< RM_FileHandle* > sysFileHandle_Vec; //����ϵͳ�ļ��ľ��
	std::map<std::string,RM_FileHandle*> rmFileHandle_Map; //���ļ���ӳ�䵽��Ӧ�ļ�¼�ļ����
	std::map<std::string,IX_IndexHandle*> ixIndexHandle_Map; //���ļ���ӳ�䵽��Ӧ�������ļ����

	int MAXATTRS=20;		 //�����������
	char curDbName[300]=""; //��ŵ�ǰDB����
	char path[300]= "C:\C++\HBASE\DB";		 //�������DB�������ϼ�Ŀ¼
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
//Ŀ�ģ���·�� dbPath �´���һ����Ϊ dbName �Ŀտ⣬������Ӧ��ϵͳ�ļ���
//1. ��OS������dbpath·�������ļ��С�
//2. ����SYSTABLES�ļ���SYSCOLUMNS�ļ�
RC CreateDB(char *dbpath,char *dbname){

	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	if (strcmp(dbpath,dbInfo.path))//�������Ĳ�����Ĭ�ϵĲ�����һ���򱨴�
		return SQL_SYNTAX;

	char createPath[40];
	RC rc;
	memset(createPath, 0, 40);
	strcat(createPath, dbpath);
	strcat(createPath, "\\");
	strcat(createPath, dbname);
	
	//1. ��OS������dbpath·�������ļ��С�
	if (CreateDirectory((LPCWSTR)createPath, NULL)) {
		if (SetCurrentDirectory((LPCWSTR)createPath)){
			//2. ����SYSTABLES�ļ���SYSCOLUMNS�ļ�
			//SYSTABLES��ż�¼:tablename atrrcount ���25���ֽ�
			//SYSCOLUMNS��ż�¼��tablename attrname attrtype attrlength attroffset ix_flag indexname�����76���ֽ�
			if ((rc= RM_CreateFile("SYSTABLES", 25)) ||  (rc= RM_CreateFile("SYSCOLUMNS", 76)))
				return rc;
			return SUCCESS;
		}
		return SQL_SYNTAX;
	}
	//����������������ġ�����ֻ�����������֣�
	return SQL_SYNTAX;
}


//
//Ŀ�ģ�ɾ�����ݿ��Ӧ��Ŀ¼�Լ�Ŀ¼�µ������ļ�
//      ���ڲ��������ļ��У����Բ���Ҫ���еݹ鴦��
//1. �ж�ɾ�����Ƿ��ǵ�ǰDB�������Ҫ�رյ�ǰĿ¼.
//2. ���������õ�dbnameĿ¼�µ������ļ�����ɾ��,ע��Ҫ����.��..Ŀ¼
//3. ɾ��dbnameĿ¼

RC DropDB(char *dbname){

	//1. �ж�ɾ�����Ƿ��ǵ�ǰDB
	if (!strcmp(dbInfo.curDbName, dbname)) {
		//�رյ�ǰĿ¼
		CloseDB();
	}

	char dbPath[300], fileName[300];
	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	//����·��
	memset(dbPath, 0, 300);
	strcat(dbPath, dbInfo.path);
	strcat(dbPath, dbname);
	strcat(dbPath, "\\*.*");

	hFile = FindFirstFile((LPCTSTR)dbPath, &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return SQL_SYNTAX;

	//2. ����ɾ���ļ����µ������ļ�
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//����.��..Ŀ¼
			continue;
		memset(fileName, 0, 300);
		strcat(fileName, dbInfo.path);
		strcat(fileName, dbname);
		strcat(fileName, "\\");
		strcat(fileName, pNextInfo.cFileName);
		DeleteFile(fileName);
	}

	//3. ɾ��dbnameĿ¼
	if (!RemoveDirectory(dbname))
		return SQL_SYNTAX;


	return SUCCESS;
}

//
//Ŀ�ģ��ı�ϵͳ�ĵ�ǰ���ݿ�Ϊ dbName ��Ӧ���ļ����е����ݿ�
//1. ���ý��̵�ǰ����Ŀ¼Ϊdbnmae
//2. �رյ�ǰ�򿪵�Ŀ¼��
//3. ��SYSTABLES��SYSCOLUMNS������sysFileHandle_Vec
//4. ����curDbName
RC OpenDB(char *dbname){

	RC rc;
	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;
	char dbPath[300];

	//1. ���ý��̵�ǰ����Ŀ¼Ϊdbnmae
	if (!SetCurrentDirectory(dbInfo.path) || !SetCurrentDirectory(dbname))
		return SQL_SYNTAX;

	//2.  �رյ�ǰ�򿪵�Ŀ¼����SYSTABLES��SYSCOLUMNS������sysFileHandle_Vec
	CloseDB();

	//3. ��SYSTABLES��SYSCOLUMNS������sysFileHandle_Vec
	RM_FileHandle* sysTables, * sysColumns;
	sysTables = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	sysColumns = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));

	if ((rc = RM_OpenFile("SYSTABLES", sysTables)) || (rc = RM_OpenFile("SYSCOLUMNS", sysColumns)))
		return SQL_SYNTAX;
	
	dbInfo.sysFileHandle_Vec.push_back(sysTables);
	dbInfo.sysFileHandle_Vec.push_back(sysColumns);

	//4. ����curDbName
	memset(dbInfo.curDbName, 0, 300);
	strcpy(dbInfo.curDbName, dbname);
	
	return SUCCESS;
}



//
//Ŀ��:�رյ�ǰ���ݿ⡣�رյ�ǰ���ݿ��д򿪵������ļ�
//1. ��鵱ǰ�Ƿ����DB������鱣���curDbName�Ƿ�Ϊ�ա�
//2. ���ñ������ж�Ӧ��Close������
//	2.1 �ر�RM�ļ�
//	2.2 �ر�IX�ļ�
//	2.3 �ر�SYS�ļ�
//3. �л����ϼ�Ŀ¼.
RC CloseDB(){
	//1. ��鵱ǰ�Ƿ����DB������鱣���curDbName�Ƿ�Ϊ�ա�
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. ���ñ������ж�Ӧ��Close������

	//	2.1 �ر�RM�ļ�
	std::map<std::string, RM_FileHandle*>::iterator rmIter;
	RM_FileHandle* rmFileHandle;
	for (rmIter = dbInfo.rmFileHandle_Map.begin(); rmIter != dbInfo.rmFileHandle_Map.end(); rmIter++) {
		rmFileHandle = rmIter->second;
		RM_CloseFile(rmFileHandle);
		free(rmFileHandle);
	}
	dbInfo.rmFileHandle_Map.clear();


	//	2.2 �ر�IX�ļ�
	std::map<std::string, IX_IndexHandle*>::iterator ixIter;
	IX_IndexHandle* ixIndexHandle;
	for (ixIter = dbInfo.ixIndexHandle_Map.begin(); ixIter != dbInfo.ixIndexHandle_Map.end(); ixIter++) {
		ixIndexHandle = ixIter->second;
		CloseIndex(ixIndexHandle);
		free(ixIndexHandle);
	}
	dbInfo.ixIndexHandle_Map.clear();
	
	//	2.3 �ر�SYS�ļ�
	RM_CloseFile(dbInfo.sysFileHandle_Vec[0]);
	RM_CloseFile(dbInfo.sysFileHandle_Vec[1]);
	free(dbInfo.sysFileHandle_Vec[0]);
	free(dbInfo.sysFileHandle_Vec[1]);
	dbInfo.sysFileHandle_Vec.clear();


	//3. �л����ϼ�Ŀ¼.
	if (!SetCurrentDirectory((LPCWSTR)dbInfo.path))
		return SQL_SYNTAX;

	return SUCCESS;
}

bool CanButtonClick(){//��Ҫ����ʵ��
	//�����ǰ�����ݿ��Ѿ���
	return true;
	//�����ǰû�����ݿ��
	//return false;
}


//
//Ŀ��:����һ����Ϊ relName �ı�
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
//2. ����Ƿ��Ѿ�����relName������Ѿ����ڣ��򷵻ش���
//3. ���atrrCount�Ƿ���� MAXATTRS��
//4. ��SYSTABLE��SYSCOLUMNS���д���Ԫ��Ϣ
//5. �����¼�Ĵ�С��������Ӧ��RM�ļ�
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) {
	//���� attrCount ��ʾ��ϵ�����Ե�������ȡֵΪ1 �� MAXATTRS ֮�䣩 �� 
	//���� attributes ��һ������Ϊ attrCount �����顣 �����¹�ϵ�е� i �����ԣ�
	//attributes �����еĵ� i ��Ԫ�ذ������ơ� ���ͺ����Եĳ��ȣ��� AttrInfo �ṹ���壩

	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

	//2. ����Ƿ��Ѿ�����relName��
	//��Ϊû��ֱ�ӵ�ɨ�躯��������ֻ��ͨ��RM��Scan���������ɨ��
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[1];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 21; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[0], 1, conditions); 

	rmRecord.bValid = false;
	rc = GetNextRec(&rmFileScan,&rmRecord); //����GetNextRecӦ�÷���RM_EOF����Ϊ�޷�ʶ��RM_EOF������û�б���.��������SUCCESS�Ķ�����һ�µ�

	if (rc==SUCCESS || rmRecord.bValid ) {//�Ѿ�����relName���ж�bValidֻ��Ϊ��ȷ����ȷ
		return SQL_SYNTAX;
	}
	CloseScan(&rmFileScan);

	//3. ���atrrCount�Ƿ���� MAXATTRS��
	if (attrCount > dbInfo.MAXATTRS)
		return SQL_SYNTAX;

	//4. ��SYSTABLE��SYSCOLUMNS���д���Ԫ��Ϣ
	int rmRecordSize=0;//�����¼��С
	char sysData[76];
	int* sysData_attrcount;
	RID rid;
	//������tablename�� ռ 21 ���ֽڣ� ������Ϊ��󳤶�Ϊ 20 ���ַ�����
	memset(sysData, 0, 76);
	strncpy(sysData, relName, 20);
	//���Ե�������attrcount�� Ϊ int ���ͣ� ռ 4 ���ֽڡ�
	sysData_attrcount = (int*)(sysData + 21);
	*sysData_attrcount = attrCount;
	
	if (rc = InsertRec(dbInfo.sysFileHandle_Vec[0], sysData, &rid))//����ʧ��
		//����ֱ�ӷ���û������������ʵ��������ģ�����Ϊ�˽��͸��Ӷ���ʱ����ʵ����
		return SQL_SYNTAX;

	//��SYSCOLUMNS��������ݡ�
	//SYSCOLUMNS:tablename attrname attrtype attrlength attroffset ix_flag indexname
	for (int i = 0; i < attrCount; i++) {
		strncpy(sysData + 21, attributes[i].attrName, 20);//������tablename�� ����������attrname�� ��ռ 21 ���ֽ�
		//attrtype int����		
		sysData_attrcount = (int*)(sysData + 21+21);// ���Ե����ͣ�attrtype�� Ϊ int ���ͣ� ռ 4 ���ֽ�
		*sysData_attrcount = attributes[i].attrType;
		//atrrlength int����
		sysData_attrcount = (int*)(sysData + 21 + 21+4);// ���Եĳ��ȣ�attrlength�� Ϊ int ���ͣ���ռ 4 ���ֽ�
		*sysData_attrcount = attributes[i].attrLength;
		rmRecordSize+= attributes[i].attrLength;
		//atrroffset int����
		sysData_attrcount = (int*)(sysData + 21 + 21 + 4 + 4);// ��¼�е�ƫ������atrroffset�� Ϊ int ���ͣ���ռ 4 ���ֽ�
		*sysData_attrcount = i;
		//ix_flag bool����
		sysData_attrcount = (int*)(sysData + 21 + 21 + 4 + 4 + 4);//�����������Ƿ���������ı�ʶ(ix_flag)ռ 1 ���ֽ�
		*sysData_attrcount = 0;									//û���������ʣ���ʼ��û������
		//indexname string����
		sysData_attrcount += 1;
		memset(sysData_attrcount, 0, 21);					//����������(indexname)ռ 21 ���ֽ�

		if (rc = InsertRec(dbInfo.sysFileHandle_Vec[1], sysData, &rid))//����ʧ��
			return SQL_SYNTAX;
	}

	//5. �����¼�Ĵ�С��������Ӧ��RM�ļ�
	//����Լ�����ļ���Ϊ��relName.rm��
	char fileName[24];//21+".rm"
	memset(fileName, 0, 24);
	strcat(fileName, relName);
	strcat(fileName, ".rm");
	RM_CreateFile(fileName,rmRecordSize);
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
RC DropTable(char* relName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

	//2. �ж��Ƿ���ڱ�relName
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[2];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 21; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[0], 1, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	//	 2.1. ����Ѿ������򱨴�
	if (rc != SUCCESS || !rmRecord.bValid) {//�Ѿ�����relName���ж�bValidֻ��Ϊ��ȷ����ȷ
		return SQL_SYNTAX;
	}
	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. �����SYSTABLES��ɾ��relName��Ӧ��һ���¼
	DeleteRec(dbInfo.sysFileHandle_Vec[0],&rmRecord.rid);

	//3. ɾ��rm�ļ���
	//	 3.1.��dbInfo��rmFileHandle_Map�в���relName��Ӧ��rm�ļ��Ƿ񱻴�
	std::map<std::string, RM_FileHandle*>::iterator rmIter;
	RM_FileHandle* rmFileHandle;
	std::string rmFileName="";
	rmFileName += relName;
	rmFileName += ".rm";
	rmIter = dbInfo.rmFileHandle_Map.find(rmFileName);
	if (rmIter != dbInfo.rmFileHandle_Map.end()) {
		//����Ѿ��������close����������Map��ɾ����
		if(RM_CloseFile(rmIter->second))
			return SQL_SYNTAX;
		free(rmIter->second);
		dbInfo.rmFileHandle_Map.erase(rmIter);
	}
	//	 3.2.ɾ��relName��Ӧ��rm�ļ�
	DeleteFile((LPCWSTR)rmFileName.c_str());

	//4. ɾ��ix�ļ�

	//	 4.1.��ȡSYSCOLUMNS���õ�relName��atrrname��
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 1, conditions)) {//��SYSCOLUMNS�ļ��в���relName�������
		return SQL_SYNTAX;
	}

	std::map<std::string, IX_IndexHandle*>::iterator ixIter;
	IX_IndexHandle* ixIndexHandle;
	std::string ixFileName = "";
	while (!GetNextRec(&rmFileScan, &rmRecord)) { //�޷�����RM_EOF���������������޷���������
		//	 4.2.�ж��Ƿ񴴽������������������������
		if ((*(rmRecord.pData + 21 + 21 + 4 + 4 + 4)) == (char)1) {
			//		 4.2.1 ��dbInfo��ixIndexHandle_Map�в���relName_atrrname��Ӧ��ix�ļ��Ƿ񱻴�
			ixFileName += rmRecord.pData + 21 + 21 + 4 + 4 + 4 + 1;
			ixIter = dbInfo.ixIndexHandle_Map.find(ixFileName);
			if (ixIter != dbInfo.ixIndexHandle_Map.end()) {
				//����Ѿ��������close����������Map��ɾ��
				if (CloseIndex(ixIter->second))
					return SQL_SYNTAX;
				free(ixIter->second);
				dbInfo.ixIndexHandle_Map.erase(ixIter);
			}
			//		 4.2.2 ɾ��relName_atrrname��Ӧ��ix�ļ�
			DeleteFile((LPCWSTR)ixFileName.c_str());
		}
		//	 4.3.ɾ��SYSCOLUMNS��atrrname��Ӧ�ļ�¼��
		DeleteRec(dbInfo.sysFileHandle_Vec[1],&rmRecord.rid);
	}
	
	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

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
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

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
	str_one[0] = 1;
	conditions[2].attrType = chars; conditions[2].bLhsIsAttr = 1; conditions[2].bRhsIsAttr = 0;
	conditions[2].compOp = EQual; conditions[2].LattrLength = 1; conditions[2].LattrOffset = +21 + 21 + 4 + 4 + 4 + 1;
	conditions[2].Lvalue = NULL; conditions[2].RattrLength = 1; conditions[2].RattrOffset = 0;
	conditions[2].Rvalue = str_one;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 3, conditions))
		return SQL_SYNTAX;
}