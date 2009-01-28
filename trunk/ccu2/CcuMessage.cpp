#include "stdafx.h"
#include "CcuMessage.h"



using namespace std;
using namespace boost;

int GenerateNewTxnId()
{
	static long txn_counter = 0;

	return ::InterlockedExchangeAdd(&txn_counter,1);

}


wstring DumpAsXml(CcuMsgPtr msg)
{
	return L"";

	std::wostringstream ostream;
	archive::xml_woarchive oa(ostream);

	CcuMessage *message = msg.get();
	oa & BOOST_SERIALIZATION_NVP(message);

	return ostream.str();

};