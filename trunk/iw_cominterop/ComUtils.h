#pragma once

#define GET_STR_PROPERTY(x) { _bstr_t bstrString = (x).c_str(); \
	*value = bstrString.copy(); };

#define PUT_STR_PROPERTY(x){ _bstr_t bstrString = value; \
	(x) = (char *)bstrString;};


class ComUtils
{
public:
	ComUtils(void);
	~ComUtils(void);

	static HRESULT CopyString(const OLECHAR * source, BSTR* dest);
};
