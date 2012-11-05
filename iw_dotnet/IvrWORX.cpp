/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "StdAfx.h"
#include "IvrWORX.h"


using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{

string MarshalToString (const String ^ s) {
	if (s==nullptr)
		return "";
	using namespace Runtime::InteropServices;
	const char* chars = 
		(const char*)(Marshal::StringToHGlobalAnsi((String ^)s).ToPointer());
	return string(chars);

	Marshal::FreeHGlobal(IntPtr((void*)chars));
}

void 
FillTable(ivrworx::interop::MapOfAnyInterop IN ^inMap, ivrworx::MapOfAny OUT &outMap)
{
	for each(String ^key in inMap->Keys)
	{
		Object ^value = inMap[key];

		if (dynamic_cast<String^>(value) != nullptr )
		{
			outMap[MarshalToString(key)] = MarshalToString(dynamic_cast<String^>(value));
		}  
		else if (dynamic_cast<Int32^>(value) != nullptr )
		{
			int x = (int)(value);
			outMap[MarshalToString(key)] = x;
		}
		else if (dynamic_cast<Boolean^>(value) != nullptr )
		{
			bool x = (bool)(value);
			outMap[MarshalToString(key)] = x;
		}
	}
}


void 
FillManagedTable(ivrworx::interop::MapOfAnyInterop IN ^inMap, const ivrworx::MapOfAny OUT &outMap)
{
	
	
	for(ivrworx::MapOfAny::const_iterator iter  = outMap.begin(); 
		iter != outMap.end(); 
		iter++)
	{
		const any &v = iter->second;
		const string &key = iter->first;

		if (v.type() == typeid(float))
		{
			unsigned int value = (unsigned int)any_cast<float>(v);
			inMap[gcnew String(key.c_str())] = value;
		}
		else if (v.type() == typeid(double))
		{
			inMap[gcnew String(key.c_str())]	= (unsigned int)any_cast<double>(v);

		}
		else if (v.type() == typeid(int))
		{
			inMap[gcnew String(key.c_str())]	= (unsigned int)any_cast<int>(v);
		}

	}
}


wstring MarshalToWString (const  String ^ s) {
	if (s==nullptr)
		return L"";
	using namespace Runtime::InteropServices;
	const wchar_t* chars = 
		(const wchar_t*)(Marshal::StringToHGlobalUni((String ^)s)).ToPointer();
	return  wstring(chars);
	Marshal::FreeHGlobal(IntPtr((void*)chars));
}

IvrWORX::IvrWORX():
_threadId(::GetCurrentThreadId()),
_conf(NULL),
_factoriesList(NULL),
_procHandles (NULL),
_shutdownHandles (NULL),
_forking(NULL),
_isDisposed(false)
{
	
	
}

Int32 IvrWORX::GetServiceHandle(String^ handle)
{
	HandleId service_handle_id = IW_UNDEFINED;
	if (IW_FAILURE(GetConfiguredServiceHandle(service_handle_id, MarshalToString(handle), *_conf)))
	{
		return 0;
	}
	return service_handle_id;
}

void IvrWORX::Init(String ^dt_confFile)
{
	int res = 0;
	try
	{

		std::string conffile;
		if (System::String::IsNullOrEmpty(dt_confFile) == true)
		{
			cerr << "init:g_conf file is NULL using conf.json" << endl;
			dt_confFile = "conf.json";
		} 

		pin_ptr<const wchar_t> wch = PtrToStringChars(dt_confFile);
		std::wstring wconffile(wch);	
		conffile = WStringToString(wconffile);

		cout << "loading " << conffile << "..." << endl;

		ivrworx::ApiErrorCode err_code = ivrworx::API_SUCCESS;
		_conf = new ConfigurationPtr(ConfigurationFactory::CreateJsonConfiguration(conffile,err_code));

		if (IW_FAILURE(err_code))
		{
			cerr << "init:error reading configuration file:" << conffile << endl;
			throw gcnew Exception("Error reading configuration file");
		}


		if (!InitLog(*_conf))
		{
			cerr << "init:error initiating logging infrastructure:" << endl;
			throw gcnew Exception("init:error initiating logging infrastructure");
		}

		// from here you may use generic clean up routine
		LogInfo(">>>>>> IVRWORX (.NET) START <<<<<<");
		LogInfo("Loaded from " << MarshalToString(System::IO::Path::GetFullPath(
			gcnew String(conffile.c_str()))));

		Start_CPPCSP();


		LpHandlePair _ctxPair = HANDLE_PAIR;

		char buffer[1024];
		::sprintf_s(buffer, 1024,".NET IvrWORX(%d)",_threadId);
		
		_ctx = new RunningContext(_ctxPair,buffer);

		RegisterContext(_ctx);


		_factoriesList = new FactoryPtrList();
		if (IW_FAILURE(LoadConfiguredModules(
			*_conf,
			*_factoriesList)))
		{
			goto error;
		};

		// this is something THAT IS NOT ADVISED by CSP
		// but no way I can do it any other way
		_procHandles	 = new HandlePairList();
		_shutdownHandles = new HandlesVector();
		_forking		 = new ScopedForking();

		if (IW_FAILURE(BootModulesSimple(
			*_conf,
			*_forking,
			*_factoriesList,
			*_procHandles,
			*_shutdownHandles)))
		{
			goto error;
		};

	} 
	catch (exception &e)
	{
		cerr << endl << "Exception caught during program execution e:" << e.what() << endl;
		goto error;
	}


	return;

error:

	throw gcnew Exception("generic initialization failure");

}


IvrWORX::~IvrWORX()
{
	if (_isDisposed)
		return;
	
	if (_threadId != ::GetCurrentThreadId())
	{
		throw gcnew Exception("Attempt to dispose the object in thread different that it was created in.");
	}

	_isDisposed = true;

	try
	{
		//shutdown known modules
		ShutdownModules(*_procHandles,*_conf);
	
		// shutdown unknown modules
		LocalProcessRegistrar().Instance().UnReliableShutdownAll();

		if (_forking)
			delete _forking;

		if (_shutdownHandles)
			delete _shutdownHandles;

		if (_procHandles)
			delete _procHandles;

		if (_conf)
			delete _conf;

		if (_ctx)
			delete _ctx;

	
		End_CPPCSP();
		LogInfo(">>>>>> IVRWORX END <<<<<<");
		ExitLog();
	}
	catch (exception &e)
	{
		cerr << endl << "~IvrWORX::exception caught e:" << e.what() << endl;
	}
	
	
}


}
}