#include "StdAfx.h"
#include "SQLiteBase.h"

/****************************************************************************
*CQueryData类
*用于获得sqlite3_stmt的查询结果
****************************************************************************/
SQLite3Query::SQLite3Query(void)
	: m_pStmt(NULL)
	, m_bEof(true)
	, m_nCols(0)
	, m_bOwnStmt(false)
{

}

SQLite3Query::SQLite3Query(sqlite3_stmt* pStmt, bool bEof, bool bOwnStmt)
	: m_nCols(0)
{
	m_pStmt = pStmt;
	m_bEof = bEof;
	m_bOwnStmt = bOwnStmt;

	if(NULL == pStmt)
		m_bEof = true;
	else
		m_nCols = sqlite3_column_count(m_pStmt);
}


SQLite3Query::~SQLite3Query(void) 
{	
	Finalize();
}


/// <summary>
/// 释放
/// </summary>
void SQLite3Query::Finalize()
{
	if (m_pStmt != NULL && m_bOwnStmt)
	{
		int nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = NULL;
	}
}

int SQLite3Query::GetFieldCount()
{
	return m_nCols;
}


void SQLite3Query::NextRow() 
{
	if(NULL == m_pStmt)
	{
		return ;
	}

	int nRet = sqlite3_step(m_pStmt);
	if (SQLITE_DONE == nRet)
	{
		m_bEof = true;
	} 
	else if (SQLITE_ROW == nRet)
	{
		// more rows, nothing to do
	} 
	else 
	{
		nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = NULL;
	}
}


bool SQLite3Query::IsEof()
{
	return m_bEof;
}


int SQLite3Query::GetNameIndex(const wchar_t* lpColName)
{
	if(NULL == m_pStmt)
	{
		return -1;
	}

	if (NULL != lpColName) 
	{
		for (int nIndex = 0; nIndex < m_nCols; nIndex++) 
		{
			const wchar_t* lpName = (const wchar_t*)sqlite3_column_name16(m_pStmt, nIndex);
			if (0 == _tcscmp(lpName, lpColName))
			{
				return nIndex;
			}
		}
	}

	return -1;
}


const wchar_t* SQLite3Query::GetIndexName(int nCol) 
{
	if(NULL == m_pStmt)
	{
		return _T("");
	}

	if (nCol < 0 || nCol > m_nCols - 1) 
	{
		return _T("");
	}

	return (const wchar_t*)sqlite3_column_name16(m_pStmt, nCol);
}


//获取对应类型数据===========================
const void* SQLite3Query::GetBlob(int nCol, int& nLen)
{
	if(NULL == m_pStmt)
	{
		return NULL;
	}
	if (nCol < 0 || nCol >= m_nCols) 
	{
		return NULL;
	}
	nLen = sqlite3_column_bytes(m_pStmt, nCol);
	return sqlite3_column_blob(m_pStmt, nCol);
}

double SQLite3Query::GetDouble(int col) 
{
	return sqlite3_column_double(m_pStmt, col);
}

int SQLite3Query::GetInt(int col)
{
	return sqlite3_column_int(m_pStmt, col);
}

__int64 SQLite3Query::GetInt64(int col)
{
	return sqlite3_column_int64(m_pStmt, col);
}

const unsigned char * SQLite3Query::GetText(int col, int *size) 
{
	if (size != NULL) 
	{
		*size = sqlite3_column_bytes(m_pStmt, col);
	}
	const unsigned char* lpText = sqlite3_column_text(m_pStmt, col);
	return (NULL != lpText) ? lpText : (const unsigned char*)"";
}

const wchar_t* SQLite3Query::GetText16(int col, int *size)
{
	if (size != NULL)
	{
		*size = sqlite3_column_bytes16(m_pStmt, col);
	}
	const void* lpText = sqlite3_column_text16(m_pStmt, col);
	return (NULL != lpText) ? (const wchar_t*)lpText : L"";
}


const wchar_t* SQLite3Query::GetText16(const wchar_t* lpValue, int* size)
{
	int nCol = GetNameIndex(lpValue);
	return GetText16(nCol, size);
}

sqlite3_value *SQLite3Query::GetValue(int col)
{
	return sqlite3_column_value(m_pStmt, col);
}

int SQLite3Query::GetColCount() 
{
	return sqlite3_column_count(m_pStmt);
}

int SQLite3Query::GetColType(int col) 
{
	//SQLITE_INTEGER  
	//SQLITE_FLOAT  
	//SQLITE_TEXT  
	//SQLITE_BLOB  
	//SQLITE_NULL
	return sqlite3_column_type(m_pStmt, col);
}

const char *SQLite3Query::GetDatabaseName(int col) 
{
	return sqlite3_column_database_name(m_pStmt, col);
}

const void *SQLite3Query::GetDatabaseName16(int col) 
{
	return sqlite3_column_database_name16(m_pStmt, col);
}

const char *SQLite3Query::GetTableName(int col) 
{
	return sqlite3_column_table_name(m_pStmt, col);
}

const void *SQLite3Query::GetTableName16(int col)
{
	return sqlite3_column_table_name16(m_pStmt, col);
}

const char *SQLite3Query::GetOriginName(int col) 
{
	return sqlite3_column_origin_name(m_pStmt, col);
}

const void *SQLite3Query::GetOriginName16(int col)
{
	return sqlite3_column_origin_name16(m_pStmt, col);
}

const char *SQLite3Query::GetName(int col) 
{
	return sqlite3_column_name(m_pStmt, col);
}

const void *SQLite3Query::GetName16(int col)
{
	return sqlite3_column_name16(m_pStmt, col);
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
SQLite3Statement::SQLite3Statement()
{
	m_pDB = NULL;
	m_pStmt = NULL;
}


SQLite3Statement::SQLite3Statement(const SQLite3Statement& rStatement)
{
	m_pDB = rStatement.m_pDB;
	m_pStmt = rStatement.m_pStmt;
	// Only one object can own VM
	const_cast<SQLite3Statement&>(rStatement).m_pStmt = NULL;
}


SQLite3Statement::SQLite3Statement(sqlite3* pDB, sqlite3_stmt* pStmt) 
{
	m_pDB = pDB;
	m_pStmt = pStmt;
}


SQLite3Statement::~SQLite3Statement() 
{	
	Finalize();
}


SQLite3Statement& SQLite3Statement::operator=(const SQLite3Statement& rStatement)
{
	m_pDB = rStatement.m_pDB;
	m_pStmt = rStatement.m_pStmt;
	// Only one object can own VM
	const_cast<SQLite3Statement&>(rStatement).m_pStmt = NULL;
	return *this;
}


int SQLite3Statement::execDML() 
{
	if(NULL == m_pDB || NULL == m_pStmt)
		return -1;

	int nRet = sqlite3_step(m_pStmt);
	if (SQLITE_DONE == nRet) 
	{
		int nRowsChanged = sqlite3_changes(m_pDB);
		nRet = sqlite3_reset(m_pStmt);
		if (SQLITE_OK != nRet)
		{			
			return -1;
		}
		//int nLastRowId = sqlite3_last_insert_rowid(m_pDB);
		return nRowsChanged;
	}
	
	nRet = sqlite3_reset(m_pStmt);
	return -1;	
}

SQLite3Query SQLite3Statement::execQuery() 
{
	if(NULL == m_pDB || NULL == m_pStmt)
		return SQLite3Query(NULL, true, false);

	int nRet = sqlite3_step(m_pStmt);
	if (SQLITE_DONE == nRet)
	{
		return SQLite3Query(m_pStmt, true, false);
	} 
	else if (SQLITE_ROW == nRet) 
	{
		return SQLite3Query(m_pStmt, false, false);
	} 
	
	nRet = sqlite3_reset(m_pStmt);
	return SQLite3Query(NULL, true, false);
}

void SQLite3Statement::Reset()
{
	if(NULL == m_pDB || NULL == m_pStmt)
		return ;

	int nRet = sqlite3_reset(m_pStmt);
	if (SQLITE_OK != nRet) 
	{
		
	}
}

void SQLite3Statement::Finalize() 
{
	if(NULL == m_pStmt)
		return ;

	int nRet = sqlite3_finalize(m_pStmt);
	m_pStmt = NULL;
	if (SQLITE_OK != nRet)
	{
		
	}
}

int SQLite3Statement::bind(int nParam, const char* szValue) 
{
	if(NULL == m_pStmt)
	{
		return -1;
	}
	return sqlite3_bind_text(m_pStmt, nParam, szValue, -1, SQLITE_TRANSIENT);
	
}

int SQLite3Statement::bind(int nParam, const wchar_t* szValue)
{
	if(NULL == m_pStmt)
	{
		return -1;
	}
	return sqlite3_bind_text16(m_pStmt, nParam, szValue, -1, SQLITE_TRANSIENT);
	
}

int SQLite3Statement::bind(int nParam, int nValue) 
{
	if(NULL == m_pStmt)
	{
		return -1;
	}

	return sqlite3_bind_int(m_pStmt, nParam, nValue);
}

int SQLite3Statement::bind(int nParam, __int64 nValue) 
{
	if(NULL == m_pStmt)
	{
		return -1;
	}

	return sqlite3_bind_int64(m_pStmt, nParam, nValue);
}

int SQLite3Statement::bind(int nParam, double dbValue) 
{
	if(NULL == m_pStmt)
	{
		return -1;
	}

	return sqlite3_bind_double(m_pStmt, nParam, dbValue);
}

int SQLite3Statement::bind(int nParam, const unsigned char* blobValue, int nLen) 
{
	if(NULL == m_pStmt)
	{
		return -1;
	}

	return sqlite3_bind_blob(m_pStmt, nParam, blobValue, nLen, SQLITE_TRANSIENT);
}

int SQLite3Statement::bindNull(int nParam)
{
	if(NULL == m_pStmt)
	{
		return -1;
	}

	return sqlite3_bind_null(m_pStmt, nParam);
}

sqlite3_int64 SQLite3Statement::GetLastInsertRowId()
{
	if(NULL == m_pDB)
		return 0;

	return sqlite3_last_insert_rowid(m_pDB);
}


/****************************************************************************
*CSqlite类
*用于操作Sqlite3数据库
****************************************************************************/
SQLite3DB::SQLite3DB(void)
	: m_pDBase(NULL)
	, m_nBusyTimeOutMs(6000)	// 6 seconds
{

}

SQLite3DB::~SQLite3DB(void) 
{
	Close();
}

//
int SQLite3DB::SetBusyTimeOut(int nMillisecs)
{
	if(NULL == m_pDBase) 
		return -1;

	m_nBusyTimeOutMs = nMillisecs;

	return sqlite3_busy_timeout(m_pDBase, m_nBusyTimeOutMs);
}


/// <summary>
/// 打开Sqlite数据库
/// </summary>
/// <param name="sDBFilePath">The db file path.</param>
/// <returns>int.</returns>
bool SQLite3DB::Open(const wchar_t* lpDBFilePath) 
{
	int nRet = sqlite3_open16((PVOID)lpDBFilePath, &m_pDBase);
	if (SQLITE_OK != nRet) 
	{
		return false;
	}
	/*
	//这个版本 没有加密   暂时没找到免费版的  好像有
	wchar_t lKey[] = L"etimes2011@";
	nRet = sqlite3_key(m_pDBase, lKey, wcslen(lKey));
	if (nRet != SQLITE_OK){
	LPCTSTR lpError = (LPCTSTR)sqlite3_errmsg16(m_pDBase);
	throw CSQLite3Exception(nRet, (LPTSTR)lpError, false);
	}*/
	return true;
}

/// <summary>
/// 关闭Sqlite数据库
/// </summary>
/// <returns>int.</returns>
void SQLite3DB::Close() 
{
	if (NULL == m_pDBase) 
	{
		return;
	}

	int nRet = sqlite3_close(m_pDBase);
	if (SQLITE_OK == nRet) 
	{
		m_pDBase = NULL;
	} 
}

sqlite3_stmt* SQLite3DB::Compile(const wchar_t* lpSQL)
{
	sqlite3_stmt* pStmt = NULL;
	
	if (NULL == m_pDBase) 
	{
		return NULL;
	}

	int nRet = sqlite3_prepare16(m_pDBase, lpSQL, -1, &pStmt, NULL);
	if (SQLITE_OK != nRet)
	{
		pStmt = NULL;
	}
	
	return pStmt;
}

int SQLite3DB::execDML(const wchar_t* lpSQL)
{
	int nRet = 0;
	sqlite3_stmt* pStmt = NULL;
	do
	{
		pStmt = Compile(lpSQL);
		if(NULL == pStmt)
		{
			return -1;
		}
		nRet = sqlite3_step(pStmt);
		if (nRet == SQLITE_ERROR)
		{
			return -1;
		}
		nRet = sqlite3_finalize(pStmt);

	} while (nRet == SQLITE_SCHEMA);

	return nRet;
}

SQLite3Query SQLite3DB::execQuery(const wchar_t* lpSQL)
{
	sqlite3_stmt* pStmt = Compile(lpSQL);
	int nRet = sqlite3_step(pStmt);
	if (SQLITE_DONE == nRet) 
	{
		//no rows 
		return SQLite3Query(pStmt, true);
	}
	else if (SQLITE_ROW == nRet)
	{
		//at least 1 row 
		return SQLite3Query(pStmt, false);
	}
	
	nRet = sqlite3_finalize(pStmt);
	return SQLite3Query(NULL, true);
}

SQLite3Statement SQLite3DB::CompileStatement(const wchar_t* lpSQL) 
{
	sqlite3_stmt* pStmt = Compile(lpSQL);
	return SQLite3Statement(m_pDBase, pStmt);
}


/// <summary>
/// 释放表数据
/// </summary>
/// <param name="azResult">The az result.</param>
//void CSQLite3DB::FreeTable(char **azResult){
//	sqlite3_free_table(azResult);
//}

/// <summary>
/// 开始事物
/// </summary>
/// <returns>int.</returns>
int SQLite3DB::Begin()
{
	return execDML(L"Begin Transaction;");
}

/// <summary>
/// 回滚
/// </summary>
/// <returns>int.</returns>
int SQLite3DB::RollBack()
{
	return execDML(L"Rollback Transaction;");
}

/// <summary>
/// 提交事物
/// </summary>
/// <returns>int.</returns>
int SQLite3DB::Commit() 
{
	return execDML(L"Commit Transaction;");
}

