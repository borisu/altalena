#pragma once

namespace ivrworx
{
namespace interop
{

	public ref class Credentials
	{
	public:
		Credentials(void);

		property String^ User
		{
			String^ get() 
			{ 
				return _user;
			}

			void set(String^ value)
			{
				_user = value;
			}
		}

		property String^ Password
		{
			String^ get() 
			{ 
				return _password;
			}

			void set(String^ value)
			{
				_password = value;
			}
		}

		property String^ Realm
		{
			String^ get() 
			{ 
				return _realm;
			}

			void set(String^ value)
			{
				_realm = value;
			}
		}

	private:

		String ^_user;

		String ^_password;

		String ^_realm;
	};
}
}
