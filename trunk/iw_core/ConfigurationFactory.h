#pragma once
#include "CcuConfiguration.h"

typedef 
shared_ptr<CcuConfiguration> CcuConfigurationPtr;

class ConfigurationFactory
{
public:

	ConfigurationFactory(void);

	~ConfigurationFactory(void);
	

	static CcuConfigurationPtr CreateJsonConfiguration(const wstring &filename);

};