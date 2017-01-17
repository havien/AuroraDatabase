#pragma once
#include "../ExternalLibs/sqlite/sqlite3.h"
#include "../AuroraUtility/AuroraDefine.h"
#include "../AuroraUtility/AuroraSingleton.h"

#include "Includes.h"

namespace Aurora
{
	namespace Database
	{
		class SQLiteManager : public Singleton<SQLiteManager>
		{
			const UInt16 MAX_FETCH_COLUMN_COUNT = 50 + 1;

			friend class Singleton<SQLiteManager>;
		private:
			SQLiteManager( void );
			NON_COPYABLE( SQLiteManager );
		public:
			virtual ~SQLiteManager( void );

			static Int32 DBResultCallBack( void* pData, int argc, char** argv, char** ColumnName );

			bool ConnectToDB( const char* pSQLFileName );
			void Close( void );
			void Reset( void );
			bool Prepare( EQueryType type, const char* pExecuteString );
			bool BindParameter( ESQLiteBindType BindType, void* pBindValue, size_t BindValueLength = 0 );

			bool Fetch( void );

			Int32 FetchSingleDataInt( void );
			Int64 FetchSingleDataInt64( void );
			Int64 GetLastInsertID( void );

			void PrintLastError( void );

			std::vector<char*>* GetFetchedData( void ) { return &_fetchedDatas; }

			bool ExecuteNonSelect( void )
			{
				return _Step();
			}
		private:
			bool _BindParameterInt32( const Int32* pBindVal );
			bool _BindParameterInt64( const Int64* pBindVal );
			bool _BindParameterMultibyteString( const char* pBindValue, const Int32 length );

			bool _Step( void );

			inline void ResetBindPosition( void ) { _bindPosition = 1; }

			bool _connected;
			sqlite3* _pDBHandle;
			sqlite3_stmt* _pStatement[DBWorkCount];

			std::vector<char*> _fetchedDatas;

			EQueryType _latestPrepareWork;
			Int32 _bindPosition;

			char* _lastErrorMessage;
			char _curQuery[MAX_SQL_STRING_LEN];
			Int32 _returnCode;
			Int64 _lastInsertID;
		};
	};

	#define AuroraSQLiteManager SQLiteManager::Instance()
}