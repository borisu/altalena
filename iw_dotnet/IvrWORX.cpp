#include "StdAfx.h"
#include "IvrWORX.h"


//using namespace std;
using namespace System;
using namespace System::Runtime::InteropServices;
using namespace ivrworx;


IvrWORX::IvrWORX(String ^dt_confFile):
_threadId(::GetCurrentThreadId()),
_configuration(NULL),
_forking(NULL)
{
	
	int res = 0;
	try
	{
		
		std::string conffile;

		if (System::String::IsNullOrEmpty(dt_confFile) == NULL)
		{
			cerr << "init:g_conf file is NULL using conf.json" << endl;
			dt_confFile = "conf.json";
		} 
		

		pin_ptr<const wchar_t> wch = PtrToStringChars(dt_confFile);
		std::wstring wconffile(wch);
		conffile = WStringToString(wconffile);

		cout << "loading " << conffile << "..." << endl;

		ApiErrorCode err_code = API_SUCCESS;

		_configuration = new ConfigurationPtr();
		*_configuration = ConfigurationFactory::CreateJsonConfiguration(conffile,err_code);

		if (IW_FAILURE(err_code))
		{
			cerr << "init:error reading configuration file:" << conffile << endl;
			throw gcnew Exception("Error reading configuration file");
		}


		if (!InitLog(*_configuration))
		{
			cerr << "init:error initiating logging infrastructure:" << endl;
			throw gcnew Exception("init:error initiating logging infrastructure");
		}

		// from here you may use generic clean up routine
		LogInfo(">>>>>> IVRWORX (.NET) START <<<<<<");
		Start_CPPCSP();


		_pair = new shared_ptr<LpHandlePair>(new HANDLE_PAIR);

		_factoryList = new shared_ptr<FactoryPtrList>(new FactoryPtrList());
		if (IW_FAILURE(LoadConfiguredModules(
			*_configuration,
			**_factoryList)))
		{
			goto error;
		};

		// this is something THAT IS NOT ADVISED by CSP
		// but no way I can do it any other way
		_forking = new shared_ptr<ScopedForking>(new ScopedForking());
		_procHandles = new shared_ptr<HandlePairList>(new HandlePairList());
		_shutdownHandles = new shared_ptr<HandlesVector>(new HandlesVector());

		if (IW_FAILURE(BootModulesSimple(
			*_configuration,
			**_forking,
			**_factoryList,
			**_procHandles,
			**_shutdownHandles)))
		{
			goto error;
		};

	} 
	catch (exception e)
	{
		cerr << endl << "Exception caught during program execution e:" << e.what() << endl;
		goto error;
	}


	return ;

error:


	End_CPPCSP();
	LogInfo(">>>>>> IVRWORX END <<<<<<");
	ExitLog();

	throw gcnew Exception("generic initialization failure");;
}


IvrWORX::~IvrWORX()
{

	

}