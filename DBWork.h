#pragma once
#include "../AuroraUtility/AuroraDefine.h"
#include "Includes.h"

namespace Aurora
{
	namespace Database
	{
		class BaseDBWork
		{
			virtual bool Select( const WCHAR* pExecuteString, void* pFetchData ) = 0;
			virtual bool Update( const WCHAR* pExecuteString, OUT Int32& affectedRowCount ) = 0;
			virtual bool Insert( const WCHAR* pExecuteString, OUT Int32& affectedRowCount ) = 0;
			virtual bool Delete( const WCHAR* pExecuteString, OUT Int32& affectedRowCount ) = 0;
		};
		/*protected:
			WCHAR workTableName[MAX_WORK_TABLE_LEN];*/
	}
}