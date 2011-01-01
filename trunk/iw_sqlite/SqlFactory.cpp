#include "StdAfx.h"
#include "SqlFactory.h"
#include "ProcSqlite.h"

namespace ivrworx
{
	SqlFactory::SqlFactory(void)
	{
	}

	SqlFactory::~SqlFactory(void)
	{
	}

	LightweightProcess *SqlFactory::Create(LpHandlePair pair, ConfigurationPtr conf)
	{
		return new ProcSqlite(conf,pair);

	}

	IW_SQLLITE_API IProcFactory* GetIwFactory()
	{
		return new SqlFactory();
	}

}

