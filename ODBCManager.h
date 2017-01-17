#pragma once

#include "../AuroraUtility/AuroraDefine.h"
#include "../AuroraUtility/AuroraSingleton.h"
#include "Includes.h"

namespace Aurora
{
	namespace Database
	{
		class ODBCManager : public Singleton<ODBCManager>
		{
			friend class Singleton<ODBCManager>;
		private:
			ODBCManager( void );
			NON_COPYABLE( ODBCManager )
		public:
			virtual ~ODBCManager( void );

			static UInt32 __stdcall WorkerThread( void* pArg );

			bool ConnectToDB( const WCHAR* pDBServerIP, const WCHAR* pDBName, const WCHAR* pDBAccountID,
							  const WCHAR* pDBPassword, EMSConnectType connectType = EMSConnectType::DriverDirect,
							  EMSServerType serverType = EMSServerType::S2012 );

			void CloseConnection( void );

			void FailedConnectMessage( void );
			void FailedExecuteMessage( void );
			void FailedExecuteMessages( void );

			SQLSMALLINT GetSQLCType( const Int32 sqlType );

			template <typename T>
			SQLRETURN Bind( const Int16 bindType, T& bindVal, DWORD bufferLength );

			template <typename T, typename R>
			SQLRETURN BindParameter( T& bindVal, const SQLINTEGER bindSQLType, 
									 const R parameterType );

			SQLRETURN BindInteger( const Int32 bindVal, SQLINTEGER ParameterType = SQL_PARAM_INPUT );
			SQLRETURN BindVARCHAR( const char* pBindVal, SQLVARCHAR ParameterType = SQL_PARAM_INPUT );
			SQLRETURN BindSmallInt( const Int16 bindVal, SQLSMALLINT ParameterType = SQL_PARAM_INPUT );
			SQLRETURN BindTinyInt( const Byte bindVal, SQLSMALLINT ParameterType = SQL_PARAM_INPUT );
			SQLRETURN BindWCHAR( const WCHAR* pBindVal, SQLWCHAR ParameterType = SQL_PARAM_INPUT );
			SQLRETURN BindWVarCHAR( const WCHAR* pBindVal, SQLWCHAR ParameterType = SQL_PARAM_INPUT );

			SQLRETURN Prepare( EDBWorkType workType, const WCHAR* pStrQuery, UInt16 SPAgumentCount );
			SQLRETURN ExecuteDirect( WCHAR* pStrQuery );
			SQLRETURN Execute( void );
			SQLRETURN Fetch( void );

			const bool IsConnected( void ) const { return _connected; }
			inline void ResetBindPosition( void ) { _bindPos = 1; }

			WCHAR tempQueryArray[MAX_QUERY_ARRAY_COUNT][MAX_SQL_STRING_LEN];
		private:
			bool _connected;
			SQLRETURN _SQLReturnValue;

			SQLHENV _hEnv;
			SQLHDBC _hDBC;
			SQLHSTMT _hStmt;

			SQLWCHAR _outConnString[MAX_SQL_STRING_LEN];
			SQLSMALLINT _outConnStringLen;

			SQLINTEGER _nativeError;
			SQLWCHAR _SQLState[6];

			SQLWCHAR _SQLMessage[SQL_MAX_MESSAGE_LENGTH];
			SQLSMALLINT _SQLMessageLen;

			SQLUSMALLINT _bindPos;
			SQLINTEGER _bindLength;

			WCHAR _tempQueryBuffer[MAX_SQL_STRING_LEN];

			inline const SQLHENV GetSQLHENV( void ) { return _hEnv; }
			inline const SQLHDBC GetSQLHDBC( void ) { return _hDBC; }
			inline const SQLHSTMT GetSQLHSTMT( void ) { return _hStmt; }
		};
	}

	#define AuroraODBCManager ODBCManager::Instance()
}