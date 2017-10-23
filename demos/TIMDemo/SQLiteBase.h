#pragma once

#include <tchar.h>

// 使用sqlite的功能需要加上DSQLITE_HAS_CODEC宏
#define SQLITE_HAS_CODEC 1

#include "sqlite3\sqlite3.h"


/****************************************************************************
*SQLite3Query类
*用于获得sqlite3_stmt的查询结果
****************************************************************************/
class SQLite3Query
{
public:
	SQLite3Query(void);
	SQLite3Query(sqlite3_stmt* pStmt, bool bEof, bool bOwnStmt = true);
	virtual ~SQLite3Query(void);

public:

	//获取列 数量
	int GetFieldCount();
	void NextRow();
	bool IsEof();
	void Finalize();

	//获取列数 根据名称
	int GetNameIndex(const wchar_t* lpColName);

	//根据列 获取名称
	const wchar_t* GetIndexName(int nCol);
	const void* GetBlob(int nCol, int& nLen);
	double GetDouble(int col);
	int GetInt(int col);
	__int64 GetInt64(int col);
	const unsigned char* GetText(int col, int* size = NULL);
	const wchar_t* GetText16(int col, int* size = NULL);
	sqlite3_value* GetValue(int col);
	const wchar_t* GetText16(const wchar_t* lpValue, int* size = NULL);
	int GetColCount();
	int GetColType(int col);

	/*
	*要使用下列Name函数必须在预处理器定义中添加如下两个宏定义
	*SQLITE_ENABLE_RTREE
	*SQLITE_ENABLE_COLUMN_METADATA
	*/
	const char* GetDatabaseName(int col);
	const void* GetDatabaseName16(int col);
	const char* GetTableName(int col);
	const void* GetTableName16(int col);
	const char* GetOriginName(int col);
	const void* GetOriginName16(int col);
	const char* GetName(int col);
	const void* GetName16(int col);

private:
	//sqlite3*					m_pDB;
	sqlite3_stmt*			m_pStmt;
	bool						m_bEof;
	int							m_nCols;
	bool						m_bOwnStmt;
};


/************************************************************************/
/* 绑定 插入                                                            */
/************************************************************************/
class SQLite3Statement
{
public:
	SQLite3Statement();
	SQLite3Statement(const SQLite3Statement& rStatement);
	SQLite3Statement(sqlite3* pDB, sqlite3_stmt* pStmt);
	SQLite3Statement& operator=(const SQLite3Statement& rStatement);
	virtual ~SQLite3Statement();

	//执行
	int execDML();

	//获取最后一次插入数据的id值
	sqlite3_int64 GetLastInsertRowId();
	SQLite3Query execQuery();

	//绑定数据
	int bind(int nParam, const char* szValue);
	int bind(int nParam, const wchar_t* szValue);
	int bind(int nParam, int nValue);
	int bind(int nParam, __int64 nValue);
	int bind(int nParam, double dbValue);
	int bind(int nParam, const unsigned char* blobValue, int nLen);
	int bindNull(int nParam);

	/*
	int bindParameterIndex(const char* szParam);
	void bind(const char* szParam, const char* szValue);
	void bind(const char* szParam, const int nValue);
	void bind(const char* szParam, const double dwValue);
	void bind(const char* szParam, const unsigned char* blobValue, int nLen);
	void bindNull(const char* szParam);
	*/
	void Reset();
	void Finalize();

private:
	sqlite3* m_pDB;
	sqlite3_stmt* m_pStmt;
};


/****************************************************************************
*SQLite3DB类 对于sqlite3 操作的一些封装
*用于操作 Sqlite3 数据库
****************************************************************************/
class SQLite3DB
{
public:
	SQLite3DB(void);
	virtual ~SQLite3DB(void);
	
	int SetBusyTimeOut(int nMillisecs);
	bool Open(const wchar_t* lpDBFilePath);
	void Close();
	int execDML(const wchar_t* lpSQL);
	SQLite3Query execQuery(const wchar_t* lpSQL);
	SQLite3Statement CompileStatement(const wchar_t* lpSQL);
	
	int Begin();
	int RollBack();
	int Commit();

	const wchar_t* GetErrorText()
	{
		return (const wchar_t*)sqlite3_errmsg16(m_pDBase);
	}
protected:
	sqlite3_stmt* Compile(const wchar_t* lpSQL);
protected:	
	sqlite3 * m_pDBase;
	int m_nBusyTimeOutMs;
};

