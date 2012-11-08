#include "StdAfx.h"
#include "Selector.h"

namespace ivrworx
{
namespace interop
{
	Selector::Selector(void)
	{
	}

	Selector::~Selector(void)
	{
	}

	ApiErrorCode 
	Selector::Select(List<IActiveObject^> ^actors, Int32 timeout, Int32 %index, Int32 %eventId)
	{
		if (actors == nullptr || timeout < 0 || actors->Count == 0)
			return ApiErrorCode::API_WRONG_PARAMETER;

		HandlesVector handles_vector;
		for each(IActiveObject ^actor in actors)
		{
			LpHandlePtr h  = 
				LpHandlePtr(new LpHandle());
			h->HandleName("Active Object Listener");

			ActiveObject *ao = actor->GetOpaqueObject();
			ao->AddEventListener(h);

			handles_vector.push_back(h);

		}


		ivrworx::ApiErrorCode err	= API_SUCCESS;
		int selected_index  = -1;

		index = -1;
		IwMessagePtr event;
		err = SelectFromChannels(
			handles_vector, 
			Seconds(timeout),
			selected_index,
			event);


		index = selected_index;
		eventId = event->message_id;
		
		return ivrworx::interop::ApiErrorCode(err);


	}

}
}

