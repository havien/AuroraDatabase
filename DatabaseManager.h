#pragma once

#include "../AuroraUtility/AuroraDefine.h"
#include "../AuroraUtility/AuroraSingleton.h"

#include "DBWork.h"
#include "Includes.h"

namespace Aurora
{
	namespace Database
	{
		class DatabaseManager : public Singleton<DatabaseManager>
		{
			friend class Singleton<DatabaseManager>;
		private:
			DatabaseManager( void );
			NON_COPYABLE( DatabaseManager );
		public:
			virtual ~DatabaseManager( void );

			bool ConnectToDB( const WCHAR* pServer );
			bool Prepare( EQueryType CurWork, const WCHAR* pExecuteString );
			bool BindParameter( void* pBindType, void* pBindValue, size_t BindValueLength = 0 );

			inline void SetDBVendor( EVendor pVendor ) { _vendor = pVendor; }
		private:
			EVendor _vendor;
		};
	};
}