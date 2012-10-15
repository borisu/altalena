#pragma once
#include "IvrWORX.h"
#include "AbstractOffer.h"
#include "Credentials.h"

using namespace System::Collections::Generic;

namespace ivrworx
{
namespace interop 
{

	public interface class GenericOfferAnswerSession 
	{
	public:

		 void CleanDtmfBuffer();

		 ApiErrorCode WaitForDtmf(
			OUT String ^signal, 
			IN  Int32 timeout);

		 ApiErrorCode RejectCall();

		 ApiErrorCode HangupCall();

		 ApiErrorCode BlindXfer(
			IN const String ^destination_uri);

		 void WaitTillHangup();

		 String^ Dnis();

		 String^ Ani();

		 AbstractOffer^ LocalOffer();

		 AbstractOffer^ RemoteOffer();

		 ApiErrorCode MakeCall(
			IN const String^					destinationUri, 
			IN const AbstractOffer^				localOffer,
			IN const Credentials^				credentials, 
			IN OUT  Dictionary<String^,Object^>^	keyValueMap,
			IN Int32							ringTimeout);

		 ApiErrorCode ReOffer(
			IN const AbstractOffer^				localOffer,
			IN OUT  Dictionary<String^,Object^>^	keyValueMap,
			IN Int32							ringTimeout);

		 ApiErrorCode Answer(
			IN const AbstractOffer^					 localOffer,
			IN const OUT  Dictionary<String^,Object^>^ keyValueMap,
			IN Int32	  ringTimeout);

		 string DtmfBuffer();

	};

}
}
