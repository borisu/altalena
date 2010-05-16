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

	LightweightProcess *SqlFactory::Create(LpHandlePair pair, Configuration &conf)
	{
		return new ProcSqlite(conf,pair);

	}

}

