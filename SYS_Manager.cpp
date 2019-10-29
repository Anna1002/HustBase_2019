#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>


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
//1. ���dbpath��dbname�ĺϷ��ԡ�
//2. �ж�dbpath�Լ�dbname�Ƿ��dbInfo�б�����ͬ�������ͬ�����CloseDb��������dbInfo��
//3. ��OS������dbpath·�������ļ��С�
//4. ����SYSTABLES�ļ���SYSCOLUMNS�ļ���
//5. ����dbPath��curDbName��
RC CreateDB(char *dbpath,char *dbname){

	//1. ���dbpath��dbname�ĺϷ��ԡ�
	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	//2. �ж�dbpath�Լ�dbname�Ƿ��dbInfo�б�����ͬ�������ͬ�����CloseDb��
	if ( dbInfo.path.size() && dbInfo.curDbName.size())
		if(dbInfo.path.compare(dbpath) != 0 || dbInfo.curDbName.compare(dbname) != 0)
			if(CloseDB())
				return SQL_SYNTAX;

	RC rc;
	
	//3. ��OS������dbpath·�������ļ��С�
	std::string dbPath = dbpath;
	dbPath += "\\";
	dbPath +=dbname;
	if (CreateDirectory(dbPath.c_str(), NULL)) {
		if (SetCurrentDirectory(dbpath)){
			//4. ����SYSTABLES�ļ���SYSCOLUMNS�ļ�
			std::string sysTablePath = dbPath + "\\SYSTABLES";
			std::string sysColumnsPath = dbPath + "\\SYSCOLUMNS";
			if ((rc= RM_CreateFile((char *)sysTablePath.c_str(), SIZE_SYS_TABLE)) ||  (rc= RM_CreateFile((char *)sysColumnsPath.c_str(), SIZE_SYS_COLUMNS)))

				return rc;
			//5. ����dbPath��curDbName��
			dbInfo.path = dbpath;
			dbInfo.curDbName = dbname;
			return SUCCESS;
		}
		return SQL_SYNTAX;
	}
	//����������������ġ�����ֻ�����������֣�
	return SQL_SYNTAX;
}


//
//Ŀ�ģ�ɾ�����ݿ��Ӧ��Ŀ¼�Լ�Ŀ¼�µ������ļ�
//      Ĭ�ϲ��������ļ��У����Բ����еݹ鴦��
//1. �ж�dbInfo.path�Ƿ�Ϊ�գ�����dbInfo.path���Ƿ����dbname������������򱨴�
//2. �ж�ɾ�����Ƿ��ǵ�ǰDB�������Ҫ�رյ�ǰĿ¼.
//3. �����õ�dbnameĿ¼�µ������ļ�����ɾ��,ע��Ҫ����.��..Ŀ¼
//4. ɾ��dbnameĿ¼
RC DropDB(char *dbname){

	//1. �ж�dbInfo.path·�����Ƿ����dbname������������򱨴�
	std::string dbPath = dbInfo.path+"\\"+dbname;
	if (!dbInfo.path.size() || access(dbPath.c_str(),0)==-1)
		return SQL_SYNTAX;

	//2. �ж�ɾ�����Ƿ��ǵ�ǰDB
	if (dbInfo.curDbName.compare(dbname)==0) {
		//�رյ�ǰĿ¼
		CloseDB();
	}

	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	dbPath += "\\*.*";
	hFile = FindFirstFile(dbPath.c_str(), &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return SQL_SYNTAX;

	//3. ����ɾ���ļ����µ������ļ�
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//����.��..Ŀ¼
			continue;
		if(!DeleteFile(pNextInfo.cFileName))
			return SQL_SYNTAX;
	}

	//4. ɾ��dbnameĿ¼
	if (!RemoveDirectory(dbname))
		return SQL_SYNTAX;

	return SUCCESS;
}

//
//Ŀ�ģ��ı�ϵͳ�ĵ�ǰ���ݿ�Ϊ dbName ��Ӧ���ļ����е����ݿ�
//1. �����ǰ����db����رյ�ǰ�򿪵�Ŀ¼��
//2. ��SYSTABLES��SYSCOLUMNS,����dbInfo
//3. ����curDbName
RC OpenDB(char *dbname){

	RC rc;

	//1.  �رյ�ǰ�򿪵�Ŀ¼����SYSTABLES��SYSCOLUMNS������sysFileHandle_Vec
	if (dbInfo.curDbName.size() && CloseDB())
		return SQL_SYNTAX;

	//2. ��SYSTABLES��SYSCOLUMNS,����dbInfo
	RM_FileHandle* sysTables, * sysColumns;
	sysTables = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	sysColumns = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));

	std::string sysTablePath = dbname;
	sysTablePath+="\\SYSTABLES";
	std::string sysColumnsPath = dbname;
	sysColumnsPath+="\\SYSCOLUMNS";
	if ((rc = RM_OpenFile((char *)sysTablePath.c_str(), sysTables)) || (rc = RM_OpenFile((char*)sysColumnsPath.c_str(), sysColumns)))
		return SQL_SYNTAX;
	
	dbInfo.sysTables=sysTables;
	dbInfo.sysColumns=sysColumns;

	//3. ����curDbName
	dbInfo.curDbName = dbname;
	
	return SUCCESS;
}



//
//Ŀ��:�رյ�ǰ���ݿ⡣�رյ�ǰ���ݿ��д򿪵������ļ�
//1. ��鵱ǰ�Ƿ����DB�����curDbNameΪ���򱨴���������curDbNameΪ�ա�
//2. ���ñ����SYS�ļ����close�������ر�SYS�ļ���
RC CloseDB(){
	//1. ��鵱ǰ�Ƿ����DB�����curDbNameΪ���򱨴���������curDbNameΪ�ա�
	if (!dbInfo.curDbName.size())
		return SQL_SYNTAX;
	dbInfo.curDbName.clear();

	//2. ���ñ����SYS�ļ����close�������ر�SYS�ļ���
	RM_CloseFile(dbInfo.sysTables);
	RM_CloseFile(dbInfo.sysColumns);
	free(dbInfo.sysTables);
	free(dbInfo.sysColumns);
	
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
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. ����Ƿ��Ѿ�����relName��
	//��Ϊû��ֱ�ӵ�ɨ�躯��������ֻ��ͨ��RM��Scan���������ɨ��
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[1];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	OpenScan(&rmFileScan, dbInfo.sysTables, 1, conditions); 

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
	char sysData[SIZE_SYS_COLUMNS];
	int* sysData_attrcount;
	RID rid;
	//������tablename�� ռ 21 ���ֽڣ� ������Ϊ��󳤶�Ϊ 20 ���ַ�����
	memset(sysData, 0, SIZE_SYS_COLUMNS);
	strncpy(sysData, relName, SIZE_TABLE_NAME);
	//���Ե�������attrcount�� Ϊ int ���ͣ� ռ 4 ���ֽڡ�
	sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME);
	*sysData_attrcount = attrCount;
	
	if (rc = InsertRec(dbInfo.sysTables, sysData, &rid))//����ʧ��
		//����ֱ�ӷ���û������������ʵ��������ģ�����Ϊ�˽��͸��Ӷ���ʱ����ʵ����
		return SQL_SYNTAX;

	//��SYSCOLUMNS��������ݡ�
	//SYSCOLUMNS:tablename attrname attrtype attrlength attroffset ix_flag indexname
	for (int i = 0; i < attrCount; i++) {
		strncpy(sysData + SIZE_TABLE_NAME, attributes[i].attrName, SIZE_ATTR_NAME-1);//������tablename�� ����������attrname�� ��ռ 21 ���ֽ�
		//attrtype int����		
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);// ���Ե����ͣ�attrtype�� Ϊ int ���ͣ� ռ 4 ���ֽ�
		*sysData_attrcount = attributes[i].attrType;
		//atrrlength int����
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);// ���Եĳ��ȣ�attrlength�� Ϊ int ���ͣ���ռ 4 ���ֽ�
		*sysData_attrcount = attributes[i].attrLength;
		rmRecordSize += attributes[i].attrLength;
		//atrroffset int����
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH);// ��¼�е�ƫ������atrroffset�� Ϊ int ���ͣ���ռ 4 ���ֽ�
		*sysData_attrcount = i;
		//ix_flag bool����
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET);//�����������Ƿ���������ı�ʶ(ix_flag)ռ 1 ���ֽ�
		*sysData_attrcount = 0;									//û�����������ʼ��û������
		//indexname string����
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG);
		memset(sysData_attrcount, 0, SIZE_INDEX_NAME);					//����������(indexname)ռ 21 ���ֽ�

		if (rc = InsertRec(dbInfo.sysColumns, sysData, &rid))//����ʧ��
			return SQL_SYNTAX;
	}

	//5. �����¼�Ĵ�С��������Ӧ��RM�ļ�
	//����Լ�����ļ���Ϊ��relName.rm��
	std::string filePath = dbInfo.curDbName + "\\" + relName + ".rm";
	if(RM_CreateFile((char *)filePath.c_str(),rmRecordSize))
		return SQL_SYNTAX;

	return SUCCESS;
}

//
//Ŀ�ģ�������Ϊ relName �ı��Լ��ڸñ��Ͻ���������������
//1. ��鵱ǰ�Ƿ��Ѿ�����һ�����ݿ⣬���û���򱨴�
//2. �ж��Ƿ���ڱ�relName.
//	 2.1. ����Ѿ������򱨴�
//	 3.2. �����SYSTABLES��ɾ��relName��Ӧ��һ���¼
//3. ɾ��relName��Ӧ��rm�ļ�
//4. ɾ��ix�ļ�
//	 4.1.��ȡSYSCOLUMNS���õ�relName��atrrname��
//	 4.2.�ж��Ƿ񴴽������������������������ɾ��relName_atrrname��Ӧ��ix�ļ�
//	 4.3.ɾ��SYSCOLUMNS��atrrname��Ӧ�ļ�¼��
RC DropTable(char* relName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. �ж��Ƿ���ڱ�relName
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[2];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysTables, 1, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	//	 2.1. ����Ѿ������򱨴�
	if (rc != SUCCESS || !rmRecord.bValid) {//�Ѿ�����relName���ж�bValidֻ��Ϊ��ȷ����ȷ
		return SQL_SYNTAX;
	}
	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. �����SYSTABLES��ɾ��relName��Ӧ��һ���¼
	if(DeleteRec(dbInfo.sysTables,&rmRecord.rid))
		return SQL_SYNTAX;

	//3. ɾ��relName��Ӧ��rm�ļ�
	std::string rmFileName="";
	rmFileName += relName;
	rmFileName += ".rm";

	if(!DeleteFile((LPCSTR)rmFileName.c_str()))
		return SQL_SYNTAX;


	//4. ɾ��ix�ļ�

	//	 4.1.��ȡSYSCOLUMNS���õ�relName��atrrname��
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 1, conditions)) {//��SYSCOLUMNS�ļ��в���relName�������
		return SQL_SYNTAX;
	}

	std::string ixFileName ;
	while (!GetNextRec(&rmFileScan, &rmRecord)) { //�޷�����RM_EOF���������������޷���������

		//	 4.2.�ж��Ƿ񴴽������������������������ɾ��relName_atrrname��Ӧ��ix�ļ�
		if ((*(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET)) == (char)1) {
			ixFileName=dbInfo.curDbName+"\\";
			ixFileName += (char *)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG);
			if(!DeleteFile(ixFileName.c_str()))
				return SQL_SYNTAX;

		}
		//	 4.3.ɾ��SYSCOLUMNS��atrrname��Ӧ�ļ�¼��
		if(DeleteRec(dbInfo.sysColumns,&rmRecord.rid))
			return SQL_SYNTAX;
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
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	char ixFileName[SIZE_TABLE_NAME+SIZE_ATTR_NAME+3];//relname+"_"+attrName+".ix"-1
	memset(ixFileName, 0, SIZE_TABLE_NAME+SIZE_ATTR_NAME+3);
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
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;

	conditions[1].attrType = chars; conditions[1].bLhsIsAttr = 1; conditions[1].bRhsIsAttr = 0;
	conditions[1].compOp = EQual; conditions[1].LattrLength = SIZE_ATTR_NAME; conditions[1].LattrOffset = SIZE_TABLE_NAME;
	conditions[1].Lvalue = NULL; conditions[1].RattrLength = SIZE_ATTR_NAME; conditions[1].RattrOffset = 0;
	conditions[1].Rvalue = attrName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 2, conditions))
		return SQL_SYNTAX;

	 GetNextRec(&rmFileScan, &rmRecord);//û�а취�Դ�����д���

	char* ix_flag = rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;//�޷���Scan��Conditions�м���ix_flag���жϡ���ʹ��charsҲ�޷��Ƚϡ�
	if (rmRecord.bValid && (*ix_flag)==(char)1) {//�Ѿ������˶�Ӧ������
		return SQL_SYNTAX;
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. ������Ӧ��������
	//	 3.1. ��ȡrelName,atrrName��SYSCOLUMNS�еļ�¼��õ�atrrType
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 2, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	if (rc != SUCCESS || !rmRecord.bValid)
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 3.2. ����IX�ļ�
	int attrType = *((int*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);
	int attrLength = *((int*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);
	char* ix_Flag = (char*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	if (*ix_Flag != 0)//���ix_flag����0
		return SQL_SYNTAX;
	//RC CreateIndex(const char * fileName,AttrType attrType,int attrLength);
	if(CreateIndex(ixFileName,(AttrType)attrType,attrLength))
		return SQL_SYNTAX;

	//	 3.3. ����SYS_COLUMNS
	*ix_Flag =(char) 1;
	if(	UpdateRec(dbInfo.sysColumns,&rmRecord))
		return SQL_SYNTAX;
	
	return SUCCESS;
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

	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. ��SYSCOLUMNS�м�������Ƿ���ڡ�����������������Լ����indexname����Ψһ�������в��ң�
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[3];

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_INDEX_NAME; 
	conditions[0].LattrOffset = SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_INDEX_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = indexName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 1, conditions))
		return SQL_SYNTAX;

	GetNextRec(&rmFileScan, &rmRecord);//û�а취�Դ�����д���

	//	 2.1 ����������򱨴�
	char* ix_flag = rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;//�޷���Scan��Conditions�м���ix_flag���жϡ���ʹ��charsҲ�޷��Ƚϡ�
	if (!rmRecord.bValid || (*ix_flag) == (char)0)//û�д���������
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2.�����޸�SYSCOLUMNS�ж�Ӧ��¼��ix_flagΪ0.
	char* ix_Flag = rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	*ix_Flag =(char)0;
	if (UpdateRec(dbInfo.sysColumns, &rmRecord))
		return SQL_SYNTAX;
	
	//3. ɾ��ix�ļ�
	std::string ixFilePath = dbInfo.curDbName + "\\" + indexName;
	if(!DeleteFile((LPCSTR)ixFilePath.c_str()))
		return SQL_SYNTAX;

	return SUCCESS;
}

//
//Ŀ�ģ��ú��������� relName ���в������ָ������ֵ����Ԫ�飬
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
//2. ��鴫������ĺϷ���
//	 2.1. ɨ��SYSTABLE
//		 2.1.1 ��������relName�����ھͱ���
//		 2.1.2 ��������򱣴�attrcount�����attrcount�ʹ����nValues��һ���򱨴�
//	 2.2. ɨ��SYSCOLUMNS�ļ���
//		 2.2.1 ��鴫��values������ֵ�������Ƿ��SYSCOLUMNS��¼����ͬ��
//		 2.2.2 ��ÿһ�����潨����������attroffset��indexName
//3. ����relName��RM�ļ������ñ����indexName�����������ļ�����values��ÿһ����¼
//	 3.1. ����InsertRec��������rm�����¼
//	 3.2. �ñ����attroffse��ix�ļ�������������ļ������¼
//4. �ر��ļ����
RC Insert(char* relName, int nValues, Value* values) {
	//nValues Ϊ����ֵ������ values Ϊ��Ӧ������ֵ���顣 

	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. ��鴫������ĺϷ���
	//	 2.1. ɨ��SYSTABLE
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[3];
	RC rc;
	int attrCount=0;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysTables, 1, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	//		 2.1.1 ��������relName�����ھͱ���
	if (rc!=SUCCESS || !rmRecord.bValid )
		return SQL_SYNTAX;
	//		 2.1.2 ��������򱣴�attrcount�����attrcount�ʹ����nValues��һ���򱨴�
	attrCount = *((int *)rmRecord.pData+SIZE_TABLE_NAME);
	if (attrCount != nValues)
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. ɨ��SYSCOLUMNS�ļ���
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 1, conditions))
		return SQL_SYNTAX;

	int site_Value = 0;
	std::vector<int> attrOffsets;
	std::vector<std::string> indexNames;
	std::string str_indexName;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	while (rc == SUCCESS && rmRecord.bValid){
		//		 2.2.1 ��鴫��values������ֵ�������Ƿ��SYSCOLUMNS��¼����ͬ��
		if (values[site_Value].type != (AttrType)(*((int*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME)))
			return SQL_SYNTAX;
		//		 2.2.2 ���潨����������attroffset��indexName
		if (*((char*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET) == (char)1) {
			attrOffsets.push_back(*((int*)rmRecord.pData  + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH));
			str_indexName = dbInfo.curDbName;
			str_indexName+=(char *)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_ATTR_OFFSET);
			indexNames.push_back(str_indexName);
		}
		site_Value++;
		rc = GetNextRec(&rmFileScan, &rmRecord);
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. ����relName��RM�ļ������ñ����indexName�����������ļ�����values��ÿһ����¼
	std::string rmFileName="";
	int numIndexs = indexNames.size();
	RM_FileHandle rmFileHandle;
	std::vector<IX_IndexHandle> ixIndexHandles;
	IX_IndexHandle curIndexHanle;

	//��RM�ļ�
	rmFileName = dbInfo.curDbName + "\\" + relName + ".rm";
	if (RM_OpenFile((char *)rmFileName.c_str(), &rmFileHandle))
		return SQL_SYNTAX;

	//��ix�ļ�
	for (int i = 0; i < numIndexs; i++) {
		if (OpenIndex(indexNames[i].c_str(), &curIndexHanle))
			return SQL_SYNTAX;
		ixIndexHandles.push_back(curIndexHanle);
	}

	site_Value = 0;
	std::vector<RID> rmInsertRids;
	RID rid;
	char* insertData;
	while (site_Value < nValues) {
		//	 3.1. ����InsertRec��������rm�����¼
		if (InsertRec(&rmFileHandle, (char*)(values[site_Value].data), &rid))
			return SQL_SYNTAX;
		//	 3.2. �ñ����attroffset��ix�ļ�������������ļ������¼
		insertData = (char*)values[site_Value].data;
		for (int i = 0; i < numIndexs; i++){			
			curIndexHanle = ixIndexHandles[i];
			if (InsertEntry(&curIndexHanle, (void*)(insertData + attrOffsets[i]), &rid))
				return SQL_SYNTAX;
		}
	}

	//4. �ر��ļ����
	if (RM_CloseFile(&rmFileHandle))
		return SQL_SYNTAX;
	for (int i = 0; i < numIndexs; i++) {
		curIndexHanle = ixIndexHandles[i];
		if (CloseIndex(&curIndexHanle))
			return SQL_SYNTAX;
	}

	return SUCCESS;
}


//
//Ŀ�ģ��ú�������ɾ�� relName ������������ָ��������Ԫ���Լ���Ԫ���Ӧ����
//��� ���û��ָ�������� ��˷���ɾ�� relName ��ϵ������Ԫ�顣 �����
//����������� ����Щ����֮��Ϊ���ϵ��
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
//2. ��鴫������ĺϷ���
//	 2.1. ɨ��SYSTABLE.��������relName�����ھͱ���
//	 2.2. ɨ��SYSCOLUMNS�ļ�����ÿһ�����潨����������indexName
//3. ����relName��RM�ļ������ñ����indexName�����������ļ���
//4. �������nConditions��conditionsת����RM_FileScan�Ĳ�����
//5. ��rm�����nConditions��conditions����rmFileScan����ɸѡ����ÿһ���¼
//	 3.1. ����rm�����DeleteRec��������ɾ����
//	 3.2. �ñ���ix�ļ������ɾ��ix��¼��
//6. �ر��ļ����
RC Delete(char* relName, int nConditions, Condition* conditions) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴�
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. ��鴫������ĺϷ���
	//	 2.1. ɨ��SYSTABLE.��������relName�����ھͱ���
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con cons[3];
	RC rc;
	int attrCount = 0;

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysTables, 1, cons))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	if (rc != SUCCESS || !rmRecord.bValid)//������relName��
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. ɨ��SYSCOLUMNS�ļ�����ÿһ�����潨����������indexName
	int site_Value = 0;
	std::vector<std::string> indexNames;
	std::string str_indexName;

	char str_one[1];
	str_one[0]=(char)1;
	cons[1].attrType = chars; cons[1].bLhsIsAttr = 1; cons[1].bRhsIsAttr = 0;
	cons[1].compOp = EQual; cons[1].LattrLength = 1; 
	cons[1].LattrOffset = SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	cons[1].Lvalue = NULL; cons[1].RattrLength = 1; cons[1].RattrOffset = 0;
	cons[1].Rvalue = str_one;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 2, cons))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	while (rc == SUCCESS && rmRecord.bValid) {
		//���潨����������indexName
		str_indexName = dbInfo.curDbName; 
		str_indexName += (char *)(rmRecord.pData  + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_ATTR_OFFSET + SIZE_IX_FLAG);
		indexNames.push_back(str_indexName);
		rc = GetNextRec(&rmFileScan, &rmRecord);
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. ����relName��RM�ļ������ñ����indexName�����������ļ���
	std::string rmFileName=dbInfo.curDbName+"\\"+relName+".rm";//relName+".rm";
	int numIndexs = indexNames.size();
	RM_FileHandle rmFileHandle;
	std::vector<IX_IndexHandle> ixIndexHandles;
	IX_IndexHandle curIndexHanle;

	//��RM�ļ�
	if (RM_OpenFile((char *)rmFileName.c_str(), &rmFileHandle))
		return SQL_SYNTAX;

	//��ix�ļ�
	for (int i = 0; i < numIndexs; i++) {
		if (OpenIndex(indexNames[i].c_str(), &curIndexHanle))
			return SQL_SYNTAX;
		ixIndexHandles.push_back(curIndexHanle);
	}

	//4. �������nConditions��conditionsת����RM_FileScan�Ĳ�����
	

	return SUCCESS;
}


//Ŀ�ģ���SYSTABLES����һ�� relName/attrCount ��¼
//1. ���relName�Ƿ��Ѿ����ڣ�����Ѿ����ڣ��򷵻ش���TABLE_EXIST
//2. ��SYSTABLES�����¼
RC TableMetaInsert(char* relName, int attrCount) {
	RC rc;
	RID rid;
	RM_Record rmRecord;
	if (TableMetaSearch(relName, &rmRecord) == SUCCESS)
		return TABLE_EXIST;

	char insertData[SIZE_TABLE_NAME + SIZE_ATTR_COUNT];
	int* insertAttrCount;
	
	memset(insertData, 0, sizeof(insertData));
	strcpy(insertData,relName);
	insertAttrCount = (int *)(insertData + SIZE_TABLE_NAME);
	*insertAttrCount = attrCount;

	if (rc = InsertRec(dbInfo.sysTables, insertData, &rid))
		return rc;

	return SUCCESS;
}

//
// Ŀ�ģ���SYSTABLE����ɾ��relNameƥ�����һ��
// relName: ������
// 1. �ж� relName �Ƿ����, �����ڷ��� TABLE_NOT_EXIST
// 2. ����������SYSTABLE��ɾ��relNameƥ��ļ�¼��
RC TableMetaDelete(char* relName){
	RC rc;
	RM_Record rmRecord;

	// 1. �ж� relName �Ƿ����, �����ڷ��� TABLE_NOT_EXIST
	if ((rc = TableMetaSearch(relName, rmRecord)))
		return rc;

	// 2. ����������SYSTABLE��ɾ��relNameƥ��ļ�¼��
	if ((rc = DeleteRec(dbInfo.sysTables, &rmRecord.rid)))
		return rc;

	return SUCCESS;
}

//
// Ŀ�ģ���SYSTABLES���в���relName�Ƿ���ڡ�
// relName: ������
// ����ֵ��
// 1. �����ڷ��� TABLE_NOT_EXIST
// 2. ���� ���� SUCCESS
// 
RC TableMetaSearch(char* relName,RM_Record* rmRecord) {
	RM_FileScan rmFileScan;
	RC rc;
	Con cons[1];

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	if ((rc = OpenScan(&rmFileScan, dbInfo.sysTables, 1, cons)) ||
		(rc = GetNextRec(&rmFileScan, rmRecord)) || (rc = CloseScan(&rmFileScan)))
		return rc;

	// 1. �����ڷ��� TABLE_NOT_EXIST
	if (rc != SUCCESS || !rmRecord->bValid)
		return TABLE_NOT_EXIST;

	// 2. ���� ���� SUCCESS
	return SUCCESS;
}

// 
// Ŀ�ģ���ӡ��SYSTABLES���ݱ�
// RC ���� RM Scan �����з����Ĵ��� (��ȥ RM_EOF)
// ��� Scan ���� Success
// �����ʽ�����ÿ�Щ������ mysql ������
// 
RC TableMetaShow() {
	return SUCCESS;
}

//
// Ŀ�ģ� ɨ��SYSCOLUMNS��relName��attrName�Ƿ���ڡ�����������򷵻�ATTR_NOT_EXIST
RC ColumnSearchAttr(char* relName, char* attrName, RM_Record* rmRecord) {
	RM_FileScan rmFileScan;
	RC rc;
	Con cons[2];
	
	RID rid;

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	cons[1].attrType = chars; cons[1].bLhsIsAttr = 1; cons[1].bRhsIsAttr = 0;
	cons[1].compOp = EQual; cons[1].LattrLength = SIZE_ATTR_NAME; cons[1].LattrOffset = SIZE_TABLE_NAME;
	cons[1].Lvalue = NULL; cons[1].RattrLength = SIZE_ATTR_NAME; cons[1].RattrOffset = 0;
	cons[1].Rvalue = attrName;

	rmRecord->bValid = false;

	if ((rc = OpenScan(&rmFileScan, dbInfo.sysColumns, 2, cons)) ||
		(rc = GetNextRec(&rmFileScan, rmRecord)) || (rc = CloseScan(&rmFileScan)))
		return rc;

	// ɨ��SYSCOLUMNS��relName��attrName�Ƿ���ڡ�����������򷵻�ATTR_NOT_EXIST
	if (rc != SUCCESS || !rmRecord->bValid)
		return ATTR_NOT_EXIST;

	return SUCCESS;
}

//
// Ŀ�ģ�������������д��pData�У�������SYSCOLUMNS���в���
RC ToData(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName,char* pData) {
	int* pDataAttrType, * pDataAttrLength, * pDataAttrOffset;
	char* pDataIxFlag;
	memset(pData, 0, SIZE_SYS_COLUMNS);
	strcpy(pData, relName);
	strcpy(pData + SIZE_TABLE_NAME, attrName);
	pDataAttrType = (int*)(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);
	*pDataAttrType = attrType;
	pDataAttrLength = (int*)(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);
	*pDataAttrLength = attrLength;
	pDataAttrOffset = (int*)(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH);
	*pDataAttrOffset = attrOffset;
	pDataIxFlag = pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	*pDataIxFlag = ixFlag;
	strcpy(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG, indexName);
	
	return SUCCESS;
}

//
// Ŀ�ģ���SYSCOLUMNS�в���һ����¼
// 1.���relName���Ƿ���ڣ�����������򷵻ش���
// 2.�������relName��attrName���Ƿ��Ѿ����ڡ�������ڷ���ATTR_EXIST
// 3.��SYSCOLUMNS�в����¼ֵ
//	 3.1. ���relName��attrName��indexname�Ƿ񳬹�ָ����Χ
//	 3.2. ������ĸ���ֵ����
//	 3.3. ��SYSCOLUMNS�����
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName) {
	RC rc;
	RM_Record rmRecord;
	// 1.���relName���Ƿ���ڣ�����������򷵻ش���
	if ((rc = TableMetaSearch(relName, &rmRecord)))
		return rc;

	// 2.�������relName��attrName���Ƿ��Ѿ����ڡ�������ڷ���ATTR_EXIST
	if (  ColumnSearchAttr(relName, attrName,&rmRecord) == SUCCESS)
		return ATTR_EXIST;

	// 2.��SYSCOLUMNS�в����¼ֵ

	//	 2.1. ���relName��attrName�Ƿ񳬹�ָ����Χ
	if (strlen(relName) >= SIZE_TABLE_NAME || strlen(attrName) >= SIZE_ATTR_NAME 
		||strlen(indexName)>= SIZE_INDEX_NAME)
		return NAME_TOO_LONG;

	//	 2.2. ������ĸ���ֵ����
	RID rid;
	char pData[SIZE_SYS_COLUMNS];
	ToData(relName, attrName, attrType, attrLength, attrOffset, ixFlag, indexName, pData);

	//	 3.3. ��SYSCOLUMNS�����
	if((rc=InsertRec(dbInfo.sysColumns,pData,&rid)))
		return rc;

	return SUCCESS;
}

//
// Ŀ�ģ�ɾ��ͬһ�����е�������������Ϣ
// 1. ���relName���Ƿ����
// 2. ����SYSCOLUMNS��ɾ��relName��ص����м�¼
RC ColumnMetaDelete(char* relName) {
	// 1. ���relName���Ƿ����
	RC rc;
	RM_Record rmRecord;
	if ((rc = TableMetaSearch(relName, &rmRecord)))
		return rc;

	// 2. ����SYSCOLUMNS��ɾ��relName��ص����м�¼
	RM_FileScan rmFileScan;
	Con cons[1];
	RID rid;

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;
	rmRecord.bValid = false;

	if ((rc = OpenScan(&rmFileScan, dbInfo.sysColumns, 1, cons)))
		return rc;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	while (rc == SUCCESS && rmRecord.bValid) {
		if ((rc = DeleteRec(dbInfo.sysColumns, &rmRecord.rid)) ||
			(rc = GetNextRec(&rmFileScan, &rmRecord)))
			return rc;
	}

	if((rc=CloseScan(&rmFileScan)))
		return rc;

	return SUCCESS;
}


//
// Ŀ�ģ�// ����SYSCOLUMNSԪ���ݱ��е�ĳһ���¼ 
// 1. ���relName��attrName�Ƿ���ڡ�����������򱨴�
// 2. ����SYSCOLUMNS
RC ColumnMetaUpdate(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName) {
	// 1. ���relName��attrName�Ƿ���ڡ�����������򱨴�
	RC rc;
	RM_Record rmRecord;
	if ((rc = TableMetaSearch(relName, &rmRecord)) ||
		(rc=ColumnSearchAttr(relName,attrName,&rmRecord)))
		return rc;

	// 2. ����SYSCOLUMNS
	RID rid;
	char pData[SIZE_SYS_COLUMNS];
	ToData(relName, attrName, attrType, attrLength, attrOffset, ixFlag, indexName, pData);
	rmRecord.pData = pData;

	if ((rc = UpdateRec(dbInfo.sysColumns,&rmRecord)))
		return rc;

	return SUCCESS;
}


//
// Ŀ�ģ�ɨ���ȡrelName���attrName���Ե����͡�����
// 1. �ж�relName��attrName�Ƿ����
// 2. ����Ϣ��װ��attribute
RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute) {
	// 1. ���relName��attrName�Ƿ���ڡ�����������򱨴�
	RC rc;
	RM_Record rmRecord;
	if ((rc = TableMetaSearch(relName, &rmRecord)) ||
		(rc = ColumnSearchAttr(relName, attrName, &rmRecord)))
		return rc;

	strcpy(attrName, attribute->attrName);
	AttrType* attrType = (AttrType*)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);
	attribute->attrType = *attrType;
	int* attrLength = (int*)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);
	attribute->attrLength = *attrLength;

	return SUCCESS;
}