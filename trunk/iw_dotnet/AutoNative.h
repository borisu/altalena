/***
	CAutoNativePtr - A smart pointer for using native objects in managed code.

	Author	:	Nishant Sivakumar
	Email	:	voidnish@gmail.com	
	Blog	:	http://blog.voidnish.com
	Web		:	http://www.voidnish.com 	

	You may freely use this class as long as you include
	this copyright. 
	
	You may freely modify and use this class as long
	as you include this copyright in your modified version. 

	This code is provided "as is" without express or implied warranty. 
	
	Copyright © Nishant Sivakumar, 2006.
	All Rights Reserved.
***/

#pragma once

template<typename T> ref class CAutoNativePtr
{
private:
	T* _ptr;

public:
	CAutoNativePtr() : _ptr(nullptr)
	{
	}

	CAutoNativePtr(T* t) : _ptr(t)
	{
	}

	CAutoNativePtr(CAutoNativePtr<T>% an) : _ptr(an.Detach())
	{
	}

	template<typename TDERIVED> 
		CAutoNativePtr(CAutoNativePtr<TDERIVED>% an) : _ptr(an.Detach())
	{
	}

	!CAutoNativePtr()
	{	
		delete _ptr;
	}

	~CAutoNativePtr()
	{
		this->!CAutoNativePtr();
	}

	CAutoNativePtr<T>% operator=(T* t)
	{
		Attach(t);
		return *this;
	}

	CAutoNativePtr<T>% operator=(CAutoNativePtr<T>% an)
	{
		if(this != %an)
			Attach(an.Detach());
		return *this;
	}

	template<typename TDERIVED> 
		CAutoNativePtr<T>% operator=(CAutoNativePtr<TDERIVED>% an)
	{
		Attach(an.Detach());
		return *this;
	}

	static T* operator->(CAutoNativePtr<T>% an)
	{
		return an._ptr;
	}

	static operator T*(CAutoNativePtr<T>% an)
	{
		return an._ptr;
	}

	T* Detach()
	{
		T* t = _ptr;
		_ptr = nullptr;
		return t;
	}

	void Attach(T* t)
	{
		if(t)
		{	
			if(_ptr != t)
			{
				delete _ptr;
				_ptr = t;
			}
		}
		else
		{
#ifdef _DEBUG
			throw gcnew Exception(
				"Attempting to Attach(...) a nullptr!");
#endif
		}		
	}

	void Destroy()
	{
		delete _ptr;
		_ptr = nullptr;
	}
};
