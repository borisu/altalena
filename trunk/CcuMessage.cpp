#include "stdafx.h"
#include "CcuMessage.h"



using namespace std;
using namespace boost;

wstring DumpAsXml(CcuMsgPtr msg)
{
	std::wostringstream ostream;
	archive::xml_woarchive oa(ostream);

	CcuMessage *message = msg.get();
	oa & BOOST_SERIALIZATION_NVP(message);

	return ostream.str();

};