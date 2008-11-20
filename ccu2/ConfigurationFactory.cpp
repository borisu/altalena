#include "StdAfx.h"
#include "ConfigurationFactory.h"
#include "JSONConfiguration.h"

ConfigurationFactory::ConfigurationFactory(void)
{
}

ConfigurationFactory::~ConfigurationFactory(void)
{
}

CcuConfigurationPtr
ConfigurationFactory::CreateJsonConfiguration(const wstring &filename)
{
	CcuConfigurationPtr res ((CcuConfiguration*)NULL);
	
	shared_ptr<JSONConfiguration> ptr(new JSONConfiguration());

	if (CCU_SUCCESS(ptr->InitFromFile(filename)))
	{
		res = ptr;
	}

	return res;
}


