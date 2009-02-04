#pragma once

#define IX_PROFILE_FUNCTION() ivrworx::FuncProfiler(__FUNCTIONW__)

#define IX_PROFILE_INIT() ivrworx::InitProfile()

#define IX_PROFILE_PRINT() ivrworx::PrintProfile()

namespace ivrworx
{
	
	void InitProfile();

	void PrintProfile();

	class FuncProfiler
	{
	public:

		FuncProfiler(wstring function);
		virtual ~FuncProfiler(void);

	private:

		__int64 _start;

		wstring _funcName;
	};

	

}

