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

#include "stdafx.h"


using namespace std;
namespace ivrworx
{

	wstring StringToWString(const string& s)
	{
		wstring temp(s.length(),L' ');
		copy(s.begin(), s.end(), temp.begin());

		return temp;
	}

	string WStringToString(const wstring& ws)
	{
		string temp(ws.begin(), ws.end());
		temp.assign(ws.begin(), ws.end());

		return temp;
	};

	void StringToUpper(string strToConvert)
	{
		std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);
	}

	void StringToLower(string strToConvert)
	{
		std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::tolower);
	}

	void WStringToUpper(wstring strToConvert)
	{
		std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);
	}

	void WStringToLower(wstring strToConvert)
	{
		std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::tolower);
	}

}
