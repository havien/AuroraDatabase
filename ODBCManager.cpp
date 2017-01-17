#pragma once
#include "../AuroraUtility/StringManager.h"
#include "../AuroraUtility/MiscManager.h"

#include "ODBCManager.h"

using namespace Aurora;
using namespace Aurora::Database;

ODBCManager::ODBCManager( void )
{
	_connected = true;
	_bindPos = 1;

	AuroraStringManager->Clear( _tempQueryBuffer, MAX_SQL_STRING_LEN );
	memset( tempQueryArray, 0, sizeof( tempQueryArray ) );

	/*_tcscpy_s( mArrQuery[0], _T( "UPDATE dbo.TB_CHARACTER SET NAME = '표지열' WHERE id = 1" ) );
	_tcscpy_s( mArrQuery[1], _T( "INSERT INTO dbo.TB_CHARACTER VALUES( '김예나' )" ) );
	_tcscpy_s( mArrQuery[2], _T( "DELETE FROM dbo.TB_CHARACTER WHERE NAME = '박유나'" ) );
	_tcscpy_s( mArrQuery[3], _T( "INSERT INTO dbo.TB_CHARACTER VALUES( '박유나' )" ) );
	_tcscpy_s( mArrQuery[4], _T( "SELECT id, NAME FROM dbo.TB_CHARACTER" ) );*/
	///_tcscpy_s( mArrQuery[ 4 ], _T( "DELETE FROM dbo.TB_CHARACTER WHERE NAME = '김예나'" ) );

	//printf( "Constructor ODBCManager\n" );*/
}

ODBCManager::~ODBCManager( void )
{
}

UInt32 ODBCManager::WorkerThread( void* pArg )
{
	if( pArg )
	{
		ODBCManager* pInstance = reinterpret_cast<ODBCManager*>(pArg);
		if( nullptr == pInstance )
		{
			return 1;
		}

		if( NULL != pInstance )
		{
			SYSTEMTIME st, lt;
			GetLocalTime( &st );

			SQLINTEGER id = 0;
			SQLWCHAR Name[50] = { 0 };

			while( true )
			{
				if( false == pInstance->IsConnected() )
				{
					printf( "ReConnect to DB Server...\n" );
					pInstance->ConnectToDB( L"192.168.0.11", L"DEV_TEST", L"160123", 
											L"djf83os", EMSConnectType::DriverDirect, EMSServerType::S2008 );
				}

				int randNum = rand() % 5;

				WCHAR printBuffer[ 512 ] = {0};
				wsprintf( printBuffer, L"RandNum : %d, Query = %s\n", randNum, pInstance->tempQueryArray[randNum] );				
				//printf( L"%s", printBuffer );

				pInstance->ResetBindPosition();

				pInstance->Bind( SQL_INTEGER, id, 0 /*sizeof( id )*/ );
				pInstance->Bind( SQL_C_WCHAR, Name, sizeof( Name ) );

				SQLRETURN SQLRet = pInstance->ExecuteDirect( pInstance->tempQueryArray[4] );
				if( SQL_STILL_EXECUTING == SQLRet )
				{
					printf( "SQL_STILL_EXECUTING\n" );
					continue;
				}

				if( SQL_SUCCEEDED( SQLRet ) || SQL_NO_DATA == SQLRet )
				{
					// SQL_NO_DATA는 성공으로 간주함.
					while( SQL_NO_DATA != pInstance->Fetch() )
					{
						WCHAR Buffer[512] = { 0 };
						wsprintf( Buffer, L"%d - %s", id, Name );
						wsprintf( Buffer, L"%s\n", Buffer );
					}

					GetLocalTime( &lt );
					printf( "START TIME [%d-%d-%d %02d:%02d:%02d] / [%d-%d-%d %02d:%02d:%02d] SEND QUERY SUCCESS!\n",
							st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
							lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond );
				}
				else
				{
					GetLocalTime( &lt );
					printf( "START TIME [%d-%d-%d %02d:%02d:%02d] / [%d-%d-%d %02d:%02d:%02d] SEND QUERY FAILED!\n",
							st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
							lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond );
					break;
				}
			}
		}
	}
	else
	{
		return 0;
	}

	return 0;
}


bool ODBCManager::ConnectToDB( const WCHAR* pDBServerIP, const WCHAR* pDBName, const WCHAR* pDBAccountID,
							   const WCHAR* pDBPassword, EMSConnectType connectType,
							   EMSServerType serverType )
{
	_connected = false;

	if( pDBServerIP && pDBName && pDBAccountID && pDBPassword )
	{
		_hEnv = SQL_NULL_HENV;
		_hDBC = SQL_NULL_HDBC;
		_hStmt = SQL_NULL_HSTMT;

		_SQLReturnValue = NULL;

		_SQLReturnValue = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv );
		if( SQL_SUCCEEDED( _SQLReturnValue ) )
		{
			_SQLReturnValue = SQLSetEnvAttr( _hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0 );
			if( SQL_SUCCESS == _SQLReturnValue )
			{
				_SQLReturnValue = SQLAllocHandle( SQL_HANDLE_DBC, _hEnv, &_hDBC );
				if( SQL_SUCCESS == _SQLReturnValue )
				{
					// DEFAULT TIMEOUT 0
					// require Background Dummy Query.
					_SQLReturnValue = SQLSetConnectAttr( _hDBC, SQL_LOGIN_TIMEOUT, reinterpret_cast<SQLPOINTER>(10), 0 );
					_SQLReturnValue = SQLSetConnectAttr( _hDBC, SQL_ATTR_CONNECTION_TIMEOUT, reinterpret_cast<SQLPOINTER>(10), 0 );

					SQLWCHAR connectionString[MAX_SQL_STRING_LEN] = {0};
					switch ( connectType )
					{
						case EMSConnectType::DSN:
						{
							// DSN only for use in-house tools.
							wsprintf( connectionString, L"FileDsn=c:\\TEST_DATA.dsn;UID=LP_INTERNAL_TEST;PWD=password" );
							break;
						}
						case EMSConnectType::DriverDirect:
						{
							WCHAR driverName[MAX_NORMAL_STRING_LEN] ={0};
							switch ( serverType )
							{
								case EMSServerType::S2005:
								{
									wcscpy_s( driverName, (MAX_NORMAL_STRING_LEN-1), L"SQL Native Client" );
									break;
								}
								case EMSServerType::S2008:
								{
									wcscpy_s( driverName, (MAX_NORMAL_STRING_LEN - 1), L"SQL Server Native Client 10.0" );
									break;
								}
								case EMSServerType::S2012:
								{
									wcscpy_s( driverName, (MAX_NORMAL_STRING_LEN - 1), L"SQL Server Native Client 11.0" );
									break;
								}
								default:
									break;
							}

							// Success to Test for SQL 2005 environment.
							// Driver={SQL Native Client};Server=%s;Database=%s;Uid=%s;Pwd=%s;Trusted_Connection=No;
								
							// SQL 2012.
							//Driver={SQL Server Native Client 11.0};Server=myServerAddress;Database=myDataBase;Trusted_Connection=yes;

							// SQL 2008.
							wsprintf( connectionString, L"Driver={%s};Server=%s;Database=%s;Uid=%s;Pwd=%s;Trusted_Connection=no;AutoTranslate=yes",
									  driverName, pDBServerIP, pDBName, pDBAccountID, pDBPassword );

							_SQLReturnValue = SQLDriverConnect( _hDBC, nullptr, reinterpret_cast<SQLWCHAR*>(connectionString),
																_countof( connectionString ), _outConnString, MAX_SQL_STRING_LEN,
																&_outConnStringLen, SQL_DRIVER_COMPLETE );
							break;
						}
					}

					if( SQL_SUCCEEDED( _SQLReturnValue ) )
					{
						// if Success to Connect DB Server, Required to Allocate STMT Handle.
						_connected = true;
						_SQLReturnValue = SQLAllocHandle( SQL_HANDLE_STMT, _hDBC, &_hStmt );

						/*printf( "DB Connect Success...!\n" );*/
						//_beginthreadex( NULL, 0, SendDummyQuery, (LPVOID)this, 0, &ThreadID );
					}
					else
					{
						FailedConnectMessage();
						CloseConnection();
						return false;
					}
				}
				else
				{
					FailedConnectMessage();
					CloseConnection();
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return _connected;
}

void ODBCManager::CloseConnection( void )
{
	if( true == _connected )
	{
		_connected = false;
		SQLDisconnect( _hDBC );
	}

	SQLFreeHandle( SQL_HANDLE_DBC, _hDBC );
	SQLFreeHandle( SQL_HANDLE_ENV, _hEnv );
}

void ODBCManager::FailedConnectMessage( void )
{
	AuroraStringManager->Clear( _SQLMessage, static_cast<UInt16>(SQL_MAX_MESSAGE_LENGTH) );

	_SQLReturnValue = SQLGetDiagRec( SQL_HANDLE_DBC, _hDBC, 1, _SQLState, 
									 &_nativeError, _SQLMessage, sizeof(_SQLMessage), &_SQLMessageLen );

	wsprintf( _tempQueryBuffer, L"[Msg - %s]", _SQLMessage );
}

void ODBCManager::FailedExecuteMessage( void )
{
	AuroraStringManager->Clear( _SQLMessage, static_cast<UInt16>(SQL_MAX_MESSAGE_LENGTH) );

	_SQLReturnValue = SQLGetDiagRec( SQL_HANDLE_STMT, _hStmt, 1, _SQLState, &_nativeError, 
									 _SQLMessage, sizeof(_SQLMessage), &_SQLMessageLen );

	wsprintf( _tempQueryBuffer, L"[STATE - %s] %s\n", _SQLState, _SQLMessage );
	PRINT_NORMAL_LOG( _tempQueryBuffer );
}

void ODBCManager::FailedExecuteMessages( void )
{
	SQLSMALLINT counter = 1;
	while ( SQLGetDiagRec( SQL_HANDLE_STMT, _hStmt, counter++, 
						   _SQLState, &_nativeError, _SQLMessage, 
						   sizeof(_SQLMessage), &_SQLMessageLen ) != SQL_NO_DATA )
	{
		wsprintf( _tempQueryBuffer, L"[STATE - %s] %s\n", _SQLState, _SQLMessage );
		PRINT_NORMAL_LOG( _tempQueryBuffer );
	}
}

SQLSMALLINT ODBCManager::GetSQLCType( const Int32 sqlType )
{
	switch( sqlType )
	{
		case SQL_INTEGER:
		{
			return SQL_C_LONG;
		}
		case SQL_FLOAT:
		{
			return SQL_C_FLOAT;
		}
		case SQL_TINYINT:
		{
			return SQL_C_TINYINT;
		}
		case SQL_SMALLINT:
		{
			return SQL_C_SSHORT;
		}
		case SQL_BIT:
		{
			return SQL_C_BIT;
		}
		case SQL_CHAR:
		case SQL_VARCHAR:
		{
			return SQL_C_CHAR;
		}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		{
			return SQL_C_WCHAR;
		}
	}

	return SQL_C_SLONG;
}

template <typename T>
SQLRETURN ODBCManager::Bind( const Int16 bindType, T& bindVal, const DWORD bufferLength )
{
	printf( "bindPosition - %d\n", _bindPos );

	SQLRETURN bindResult = SQLBindCol( _hStmt, _bindPos, bindType, 
									   &bindVal, bufferLength, reinterpret_cast<SQLLEN*>(&_bindLength) );

	if( SQL_SUCCESS == bindResult )
	{
		++_bindPos;
	}
	else
	{
		FailedExecuteMessage();
	}

	return bindResult;
}

template <typename T, typename R>
SQLRETURN ODBCManager::BindParameter( T& bindVal, const SQLINTEGER bindSQLType, const R parameterType )
{
	SQLLEN *pSQLLength = nullptr;
	Int64 length = 0;

	if( SQL_VARCHAR == bindSQLType )
	{
		length = strlen( reinterpret_cast<const char*>(&bindVal) );
		pSQLLength = (SQLLEN *)&length;
	}
	else if( SQL_WVARCHAR == bindSQLType )
	{
		length = wcslen( reinterpret_cast<const WCHAR*>(&bindVal) );
		pSQLLength = (SQLLEN *)&length;
	}

	parameterType;
	SQLRETURN bindResult = 0;/* SQLBindParameter( _hStmt, _bindPos, parameterType,
											 GetSQLCType( bindSQLType ), bindSQLType, 0, 
											 0, reinterpret_cast<SQLPOINTER>(&bindVal), 0,
											 pSQLLength );
	if( SQL_SUCCESS == bindResult )
	{
		++_bindPos;
	}
	else
	{
		FailedExecuteMessage();
	}
	*/

	return bindResult;
}

// Non-Support normal char.
SQLRETURN ODBCManager::BindInteger( const Int32 bindVal, SQLINTEGER ParameterType )
{
	return BindParameter( bindVal, SQL_INTEGER, ParameterType );
}

SQLRETURN ODBCManager::BindVARCHAR( const char* pBindVal, SQLVARCHAR ParameterType )
{
	return BindParameter( pBindVal, SQL_VARCHAR, ParameterType );
}

SQLRETURN ODBCManager::BindSmallInt( const Int16 bindVal, SQLSMALLINT ParameterType )
{
	return BindParameter( bindVal, SQL_SMALLINT, ParameterType );
}

SQLRETURN ODBCManager::BindTinyInt( const Byte bindVal, SQLSMALLINT ParameterType )
{
	return BindParameter( bindVal, SQL_TINYINT, ParameterType );
}

SQLRETURN ODBCManager::BindWCHAR( const WCHAR* pBindVal, SQLWCHAR ParameterType )
{
	return BindParameter( pBindVal, SQL_WCHAR, ParameterType );
}

SQLRETURN ODBCManager::BindWVarCHAR( const WCHAR* pBindVal, SQLWCHAR ParameterType )
{
	return BindParameter( pBindVal, SQL_WVARCHAR, ParameterType );
}

SQLRETURN ODBCManager::Prepare( EDBWorkType workType, const WCHAR* pStrQuery, UInt16 SPAgumentCount )
{
	if( true == _connected )
	{
		if( EDBWorkType::StoredProcedure == workType )
		{
			// prepare할 때, 인자가 있다면 알아서 ?를 추가하며 파싱.
			if( 0 == SPAgumentCount )
			{
				wsprintf( _tempQueryBuffer, L"{CALL dbo.%s}", pStrQuery );
			}
			else if( 0 < SPAgumentCount )
			{
				wsprintf( _tempQueryBuffer, L"{CALL dbo.%s(", pStrQuery );
				for ( USHORT Counter = 1; Counter <= SPAgumentCount; ++Counter )
				{
					if( Counter == SPAgumentCount )
					{
						wsprintf( _tempQueryBuffer, L"%s?", _tempQueryBuffer );
					}
					else
					{
						wsprintf( _tempQueryBuffer, L"%s?, ", _tempQueryBuffer );
					}
				}

				wsprintf( _tempQueryBuffer, L"%s)}", _tempQueryBuffer );
			}
		}
		else
		{
			wcscpy_s( _tempQueryBuffer, (MAX_SQL_STRING_LEN-1), pStrQuery );
		}

		//printf( "SQLPrepare - %s\n", _tempQueryBuffer );
		SQLRETURN prepareResult = SQLPrepare( _hStmt, reinterpret_cast<SQLWCHAR*>(_tempQueryBuffer), SQL_NTS );
		if( SQL_SUCCESS == prepareResult || SQL_SUCCESS_WITH_INFO == prepareResult )
		{
			ResetBindPosition();
		}

		if( SQL_SUCCESS != prepareResult )
		{
			FailedExecuteMessages();
		}

		return prepareResult;
	}
	else
	{
		FailedConnectMessage();
		return SQL_ERROR;
	}
}

SQLRETURN ODBCManager::ExecuteDirect( WCHAR* pStrQuery )
{
	if( nullptr == pStrQuery )
	{
		return SQL_ERROR;
	}

	if( (MAX_SQL_STRING_LEN < wcslen( pStrQuery )) )
	{
		return SQL_ERROR;
	}

	SQLAllocHandle( SQL_HANDLE_STMT, _hDBC, &_hStmt );
	_SQLReturnValue = SQLExecDirect( _hStmt, pStrQuery, static_cast<SQLINTEGER>(wcslen( pStrQuery )) );
	if( SQL_ERROR == _SQLReturnValue )
	{
		FailedExecuteMessage();
	}

	return _SQLReturnValue;
}

// db work pipeline [prepare -> bind -> execute -> fetch]
SQLRETURN ODBCManager::Execute( void )
{
	if( true == _connected )
	{
		_SQLReturnValue = SQLExecute( _hStmt );
		/*size_t QueryLength = wcslen( _tempQueryBuffer );

		_SQLReturnValue =  SQLExecDirect( _hStmt, L"{ ? = call dbo.SP_INSERT_TEST_TABLE(?) }", SQL_NTS );*/
		//_SQLReturnValue = SQLExecDirect( _hStmt, _tempQueryBuffer, (SQLINTEGER)QueryLength );
	}
	else
	{
		_SQLReturnValue = SQL_ERROR;
	}

	if( SQL_ERROR == _SQLReturnValue || SQL_SUCCESS_WITH_INFO == _SQLReturnValue )
	{
		FailedExecuteMessage();
	}

	return _SQLReturnValue;
}

SQLRETURN ODBCManager::Fetch( void )
{
	if( true == _connected /*&& SQL_SUCCESS == _SQLReturnValue*/ )
	{
		return SQLFetch( _hStmt );
	}
	else
	{
		return SQL_ERROR;
	}
}

// 앞으로 해야 할 것.
// 1. select 쿼리 날렸을 때 rows 받기.
// 2. 더미 스레드로 쿼리 날리기.
// 3. 커넥션이 끊어지면 다시 맺기.