#pragma once
#include "SQLiteManager.h"
#include "../AuroraUtility/MiscManager.h"


using namespace Aurora;
using namespace Aurora::Database;


SQLiteManager::SQLiteManager( void ) :
	_connected( false ),
	_returnCode( SQLITE_ERROR ),
	_lastErrorMessage( nullptr ),
	_lastInsertID( 0 ),
	_bindPosition( 0 )
{
	memset( _curQuery, 0, MAX_SQL_STRING_LEN );
	for( auto current = static_cast<Byte>(EQueryType::Select); current <= static_cast<Byte>(EQueryType::Delete); ++current )
	{
		_pStatement[current] = nullptr;
	}
}

SQLiteManager::~SQLiteManager( void )
{
	Close();
}

Int32 SQLiteManager::DBResultCallBack( void* pData, Int32 argc, char**argv, char**ColumnName )
{
	auto pThis = reinterpret_cast<SQLiteManager*>(pData); pThis;
	for( auto i = 0; i < argc; i++ )
	{
		PRINT_NORMAL_LOG( L"%S = %S\n", ColumnName[i], argv[i] ? argv[i] : "NULL" );
		//pThis->_fetchedDatas.push_back( argv[i] );
	}

	return 0;
}

bool SQLiteManager::ConnectToDB( const char* pSQLFileName )
{
	if( pSQLFileName )
	{
		_returnCode = sqlite3_open_v2( pSQLFileName, &_pDBHandle, SQLITE_OPEN_READWRITE, NULL );
		if( SQLITE_OK == _returnCode )
		{
			_connected = true;
			PRINT_NORMAL_LOG( L"Opened database successfully\n" );
			return true;
		}
		else
		{
			PRINT_NORMAL_LOG( L"[SQLiteManager-LoadDBFile] Can't open database: (%S)\n",
							  sqlite3_errmsg( _pDBHandle ) );
			Close();
			return false;
		}
	}
	else
	{
		Close();
		return false;
	}
}

void SQLiteManager::Close( void )
{
	if( nullptr != _pDBHandle )
	{
		sqlite3_finalize( _pStatement[0] );
		sqlite3_finalize( _pStatement[1] );
		sqlite3_finalize( _pStatement[2] );
		sqlite3_finalize( _pStatement[3] );

		sqlite3_free( _lastErrorMessage );
		_returnCode = sqlite3_close( _pDBHandle );

		if( SQLITE_OK != _returnCode )
		{
			PRINT_NORMAL_LOG( L"[SQLiteManager::Close] Return Code is not OK\n" );
		}
	}
}

void SQLiteManager::Reset( void )
{
	_lastErrorMessage = nullptr;
	memset( _curQuery, 0, MAX_SQL_STRING_LEN );
	_returnCode = SQLITE_ERROR;
	_lastInsertID = 0;
}

bool SQLiteManager::Prepare( EQueryType queryType, const char* pExecuteString )
{
	if( true == _connected && pExecuteString )
	{
		auto castWorkType = static_cast<Int32>(queryType);
		if( EQueryType::Select <= queryType && queryType <= EQueryType::Delete )
		{
			sqlite3_reset( _pStatement[castWorkType] );

			_returnCode = sqlite3_prepare_v2( _pDBHandle, pExecuteString, 
											  static_cast<Int32>(strlen( pExecuteString )), &_pStatement[castWorkType],
											  nullptr );
			if( SQLITE_OK != _returnCode )
			{
				PRINT_NORMAL_LOG( L"[SQLiteManager-Prepare] Failed to Prepare :%S\n",
									sqlite3_errmsg( _pDBHandle ) );
				return false;
			}

				
			ResetBindPosition();
			_latestPrepareWork = queryType;
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool SQLiteManager::BindParameter( ESQLiteBindType bindType, void* pBindValue, size_t valueLength )
{
	if( true == _connected && _pDBHandle && pBindValue )
	{
		//sqlite3_reset( _pStatement[_latestPrepareWork] );

		_returnCode = SQLITE_ERROR;
		switch( bindType )
		{
			case ESQLiteBindType::Int32:
			{
				return _BindParameterInt32( reinterpret_cast<Int32*>(pBindValue) );
			}
			case ESQLiteBindType::Int64:
			{
				return _BindParameterInt64( reinterpret_cast<Int64*>(pBindValue) );
			}
			case ESQLiteBindType::NCHAR:
			case ESQLiteBindType::NVARCHAR:
			{
				return _BindParameterMultibyteString( reinterpret_cast<const char*>(pBindValue), static_cast<Int32>(valueLength) );
			}
		}
	}

	return false;
}

void SQLiteManager::PrintLastError( void )
{
	PRINT_NORMAL_LOG( L"[ExecuteQuery] SQL error: %S\n", _lastErrorMessage );
}

bool SQLiteManager::Fetch( void )
{
	if( SQLITE_OK == _returnCode && EQueryType::Select == _latestPrepareWork )
	{
		auto castWorkType = static_cast<Int32>(EQueryType::Select);

		auto SQLRows = sqlite3_step( _pStatement[castWorkType] ); SQLRows;
		auto columnCount = sqlite3_column_count( _pStatement[castWorkType] );
		if( 0 < columnCount )
		{
			for ( Int32 i = 0; i < columnCount; i++ )
			{
				Int32 type = sqlite3_column_type( _pStatement[castWorkType], i );
				if( SQLITE_NULL == type )
				{
					PRINT_NORMAL_LOG( L"Fetch Row is NULL\n" );
					continue;
				}
				if( SQLITE_TEXT == type )
				{
					/*char_value = sqlite3_column_text( stmt, i );
					len = sqlite3_column_bytes( stmt, i );*/
				}
				else if( SQLITE_INTEGER == type )
				{
					Int32 nValue = sqlite3_column_int( _pStatement[castWorkType], i );
					nValue;
				}
				else if( type == SQLITE_FLOAT )
				{
					double dlValue = sqlite3_column_double( _pStatement[castWorkType], i );
					dlValue;
				}

				//if( sqlite3_column_type( stmt, i ) != SQLITE_BLOB )
				//{
				//	blob_value = sqlite3_column_blob( stmt, i );				len = sqlite3_column_bytes( stmt, i );
				//	// gaiaFromSpatiaLiteBlobWkb ´Â spatialite.c ÀÇ 16621
				//	dbf_write->Geometry = gaiaFromSpatiaLiteBlobWkb( blob_value, len );
				//}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

Int32 SQLiteManager::FetchSingleDataInt( void )
{
	return true == Fetch() ? sqlite3_column_int( _pStatement[static_cast<Int32>(EQueryType::Select)], 0 ) : 0;
}

Int64 SQLiteManager::FetchSingleDataInt64( void )
{
	return true == Fetch() ? sqlite3_column_int64( _pStatement[static_cast<Int32>(EQueryType::Select)], 0 ) : 0;
}

Int64 SQLiteManager::GetLastInsertID( void )
{
	if( SQLITE_OK == _returnCode || SQLITE_DONE == _returnCode )
	{
		_lastInsertID = sqlite3_last_insert_rowid( _pDBHandle );
		return _lastInsertID;
	}

	return 0;
}

bool SQLiteManager::_BindParameterInt32( const Int32* pBindVal )
{
	if( pBindVal )
	{
		auto bindResult = sqlite3_bind_int( _pStatement[static_cast<Int32>(_latestPrepareWork)], _bindPosition++, *pBindVal );
		return SQLITE_OK == bindResult ? true : false;
	}

	return false;
}

bool SQLiteManager::_BindParameterInt64( const Int64* pBindVal )
{
	if( pBindVal )
	{
		auto BindResult = sqlite3_bind_int64( _pStatement[static_cast<Int32>(_latestPrepareWork)], _bindPosition++, *pBindVal );
		return SQLITE_OK == BindResult ? true : false;
	}
	
	return false;
}

bool SQLiteManager::_BindParameterMultibyteString( const char* pBindValue, const Int32 length )
{
	auto BindResult = sqlite3_bind_text( _pStatement[static_cast<Int32>(_latestPrepareWork)], _bindPosition++,
										 pBindValue, static_cast<Int32>(length),
										 SQLITE_TRANSIENT );

	return SQLITE_OK == BindResult ? true : false;
}


bool SQLiteManager::_Step( void )
{
	_returnCode = sqlite3_step( _pStatement[static_cast<Int32>(_latestPrepareWork)] );

	if( SQLITE_OK == _returnCode )
	{
		return true;
	}
	else if( SQLITE_DONE == _returnCode )
	{
		PRINT_NORMAL_LOG( L"Return CODE is SQLITE_DONE\n" );
		return true;
	}
	else
	{
		PrintLastError();
		return false;
	}
}