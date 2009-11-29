#pragma once

namespace ivrworx
{
	class RtspFactory:
		public IProcFactory
	{
	public:

		RtspFactory(void);

		LightweightProcess *Create(LpHandlePair pair, Configuration &conf);

		virtual ~RtspFactory(void);
	};
	
}
