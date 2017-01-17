#pragma once
#include "../AuroraUtility/AuroraDefine.h"
#include "Includes.h"

namespace Aurora
{
	namespace Database
	{
		class IDatabase
		{
		public:
			IDatabase( void ) { }
			virtual ~IDatabase( void ) { }

			virtual bool ConnectToDB( void ) = 0;
			virtual void CloseConnection( void ) = 0;

			virtual short Prepare( const EQueryType queryType, const WCHAR* pQuery, const UInt16 SPAgumentCount = 0 ) = 0;
			virtual short ExecuteDirect( WCHAR* pStrQuery ) = 0;
			virtual short Execute( void ) = 0;
			virtual short Fetch( void ) = 0;
		protected:
			inline const bool IsConnected( void )
			{
				return connected;
			}

			bool connected;
			EVendor dbVendor;
		};
	}
}