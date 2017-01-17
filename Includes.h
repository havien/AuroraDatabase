#pragma once
#include "../AuroraUtility/Includes.h"
#include "../AuroraUtility/AuroraDefine.h"

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#include <sql.h>
	#include <sqlext.h>
#endif

namespace Aurora
{
	namespace Database
	{
		const UInt16 DBWorkCount = 4;
		const UInt16 MAX_QUERY_ARRAY_COUNT = 5;
		const UInt16 MAX_SQL_STRING_LEN = 255 + 1;
		const UInt16 MAX_WORK_TABLE_LEN = 64 + 1;

		enum class EVendor : Byte
		{
			MSSQL = 0,
			MySQL,
			MariaDB,
			SQLite,
			Max,
		};

		enum class EDBWorkType : Byte
		{
			Query = 0,
			StoredProcedure,
			Max,
		};

		enum class EQueryType : Int32
		{
			Select = 0,
			Insert,
			Update,
			Delete,
			Max,
		};

		enum class EMSServerType : Byte
		{
			S2005 = 0,
			S2008,
			S2008R2,
			S2012,
			Max,
		};

		enum class EMSConnectType : Byte
		{
			DSN = 0,
			DriverDirect,
			Max,
		};

		enum class ESQLiteBindType : Byte
		{
			Int32 = 0,
			Int64,
			NCHAR,
			NVARCHAR,
		};
	}
}