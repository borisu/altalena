#pragma once

using namespace resip;

namespace ivrworx
{
	class FreeContent :
		public Contents
	{
	public:

		FreeContent(const string &body, const string &type);

		FreeContent(const AbstractOffer &offer);

		virtual ~FreeContent(void);

		virtual EncodeStream& encodeParsed(EncodeStream& str) const;

		virtual void parse(ParseBuffer& pb);

		virtual Contents* clone() const;


	private:

		string _body;

		string _type;
	};

}


