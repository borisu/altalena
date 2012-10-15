#pragma once
#include "AutoNative.h"

namespace ivrworx
{
namespace interop
{
	public ref class AbstractOffer 
	{
	public:

		AbstractOffer();

		~AbstractOffer();

		property String^ Body
		{
			String^ get() 
			{ 
				return _body;
			}

			void set(String^ value)
			{
				_body = value;
			}
		}

		property String^ Type
		{
			String^ get() 
			{ 
				return _type;
			}

			void set(String^ value)
			{
				_type = value;
			}
		}

	private:

		String ^_body;

		String ^_type;


	};

}
}

