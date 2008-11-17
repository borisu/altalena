
#ifndef SYSLOG_H__
#define SYSLOG_H__

#include "CommonIncludes.h"
#include <iomanip>

const int LOG_EMERG       = 0;       /* system is unusable */
const int LOG_ALERT       = 1;       /* action must be taken immediately */
const int LOG_CRIT        = 2;       /* critical conditions */
const int LOG_ERR         = 3;       /* error conditions */
const int LOG_WARNING     = 4;       /* warning conditions */
const int LOG_NOTICE      = 5;       /* normal but significant condition */
const int LOG_INFO        = 6;       /* informational */
const int LOG_DEBUG       = 7;       /* debug-level messages */                     
const int LOG_SPECIAL_DEBUG = 12;
#define LOG_PRIMASK   0x07	/* mask to extract priority part (internal) */
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)	/* extract priority */
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))

#ifdef SYSLOG_NAMES
#define	INTERNAL_NOPRI	0x10	/* the "no priority" priority */
#define	INTERNAL_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0)
typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

CODE prioritynames[] =
  {
    { "alert", LOG_ALERT },
    { "crit", LOG_CRIT },
    { "debug", LOG_DEBUG },
    { "emerg", LOG_EMERG },
    { "err", LOG_ERR },
    { "error", LOG_ERR },		/* DEPRECATED */
    { "info", LOG_INFO },
    { "none", INTERNAL_NOPRI },		/* INTERNAL */
    { "notice", LOG_NOTICE },
    { "panic", LOG_EMERG },		/* DEPRECATED */
    { "warn", LOG_WARNING },		/* DEPRECATED */
    { "warning", LOG_WARNING },
    { NULL, -1 }
  };

#endif

const int LOG_KERN		= (0<<3);	/* kernel messages */
const int LOG_USER		= (1<<3);	/* random user-level messages */
const int LOG_MAIL		= (2<<3);	/* mail system */
const int LOG_DAEMON	= (3<<3);	/* system daemons */
const int LOG_AUTH		= (4<<3);	/* security/authorization messages */
const int LOG_SYSLOG	= (5<<3);	/* messages generated internally by syslogd */
const int LOG_LPR		= (6<<3);	/* line printer subsystem */
const int LOG_NEWS		= (7<<3);	/* network news subsystem */
const int LOG_UUCP		= (8<<3);	/* UUCP subsystem */
const int LOG_CRON		= (9<<3);	/* clock daemon */
const int LOG_AUTHPRIV	= (10<<3);	/* security/authorization messages (private) */
const int LOG_FTP		= (11<<3);	/* ftp daemon */

/* other codes through 15 reserved for system use */
const int LOG_LOCAL0	= (16<<3);	/* reserved for local use */
const int LOG_LOCAL1	= (17<<3);	/* reserved for local use */
const int LOG_LOCAL2	= (18<<3);	/* reserved for local use */
const int LOG_LOCAL3	= (19<<3);	/* reserved for local use */
const int LOG_LOCAL4	= (20<<3);	/* reserved for local use */
const int LOG_LOCAL5	= (21<<3);	/* reserved for local use */
const int LOG_LOCAL6	= (22<<3);	/* reserved for local use */
const int LOG_LOCAL7	= (23<<3);	/* reserved for local use */
const int LOG_LOCAL8	= (24<<3);	/* reserved for local use */
const int LOG_LOCAL9	= (25<<3);	/* reserved for local use */
const int LOG_LOCAL10	= (26<<3);	/* reserved for local use */
const int LOG_LOCAL11	= (27<<3);	/* reserved for local use */
const int LOG_LOCAL12	= (28<<3);	/* reserved for local use */
const int LOG_LOCAL13	= (29<<3);	/* reserved for local use */
const int LOG_LOCAL14	= (30<<3);	/* reserved for local use */
const int LOG_LOCAL15	= (31<<3);	/* reserved for local use */
const int LOG_LOCAL16	= (32<<3);	/* reserved for local use */
const int LOG_LOCAL17	= (33<<3);	/* reserved for local use */
const int LOG_LOCAL18	= (34<<3);	/* reserved for local use */
const int LOG_LOCAL19	= (35<<3);	/* reserved for local use */
const int LOG_LOCAL20	= (36<<3);	/* reserved for local use */
const int LOG_LOCAL21	= (37<<3);	/* reserved for local use */
const int LOG_LOCAL22	= (38<<3);	/* reserved for local use */
const int LOG_LOCAL23	= (39<<3);	/* reserved for local use */
const int LOG_LOCAL24	= (40<<3);	/* reserved for local use */
const int LOG_LOCAL25	= (41<<3);	/* reserved for local use */
const int LOG_LOCAL26	= (42<<3);	/* reserved for local use */
const int LOG_LOCAL27	= (43<<3);	/* reserved for local use */
const int LOG_LOCAL28	= (44<<3);	/* reserved for local use */
const int LOG_LOCAL29	= (45<<3);	/* reserved for local use */

// BEGIN COSMOCOM SPECIFIC SECTION ************************************
const int LOG_ORE		= LOG_LOCAL0;
const int LOG_CPXY		= LOG_LOCAL1;
const int LOG_CSV		= LOG_LOCAL2;
const int LOG_CFM		= LOG_LOCAL3;
const int LOG_LGSV		= LOG_LOCAL4;
const int LOG_MCS		= LOG_LOCAL5;
const int LOG_PSTN		= LOG_LOCAL6;
const int LOG_STAT		= LOG_LOCAL7;
const int LOG_AIS		= LOG_LOCAL8;
const int LOG_PINGPONG	= LOG_LOCAL9;
const int LOG_DEMO_ORE	= LOG_LOCAL10;
const int LOG_ACOM		= LOG_LOCAL11;
const int LOG_STS		= LOG_LOCAL12;
const int LOG_STSC		= LOG_LOCAL13;
const int LOG_RTR		= LOG_LOCAL14;
const int LOG_VCS		= LOG_LOCAL15;
const int LOG_IPC		= LOG_LOCAL16;
const int LOG_STSPBLSH	= LOG_LOCAL17;
const int LOG_STSRQSTS	= LOG_LOCAL18;
const int LOG_PDS		= LOG_LOCAL19;
const int LOG_CIS		= LOG_LOCAL20;
const int LOG_DTR		= LOG_LOCAL21;
const int LOG_WEBMCS	= LOG_LOCAL22;
// END COSMOCOM SPECIFIC SECTION ************************************

#define	LOG_NFACILITIES	46	/* current number of facilities */
#define	LOG_FACMASK	0x03f8	/* mask to extract facility part */
#define	LOG_FAC(p)	(((p) & LOG_FACMASK) >> 3)	/* facility of pri */

#ifdef SYSLOG_NAMES
CODE facilitynames[] =
  {
    { "auth", LOG_AUTH },
    { "authpriv", LOG_AUTHPRIV },
    { "cron", LOG_CRON },
    { "daemon", LOG_DAEMON },
    { "ftp", LOG_FTP },
    { "kern", LOG_KERN },
    { "lpr", LOG_LPR },
    { "mail", LOG_MAIL },
    { "mark", INTERNAL_MARK },		/* INTERNAL */
    { "news", LOG_NEWS },
    { "security", LOG_AUTH },		/* DEPRECATED */
    { "syslog", LOG_SYSLOG },
    { "user", LOG_USER },
    { "uucp", LOG_UUCP },
    { "local0", LOG_LOCAL0 },		
    { "local1", LOG_LOCAL1 },
    { "local2", LOG_LOCAL2 },
    { "local3", LOG_LOCAL3 },
    { "local4", LOG_LOCAL4 },
    { "local5", LOG_LOCAL5 },
    { "local6", LOG_LOCAL6 },
    { "local7", LOG_LOCAL7 },
    { "local8", LOG_LOCAL8 },
    { "local9", LOG_LOCAL9 },
    { "local10", LOG_LOCAL10 },
    { "local11", LOG_LOCAL11 },
    { "local12", LOG_LOCAL12 },
    { "local13", LOG_LOCAL13 },
    { "local14", LOG_LOCAL14 },
    { "local15", LOG_LOCAL15 },
    { "local16", LOG_LOCAL16 },
    { "local17", LOG_LOCAL17 },
    { "local18", LOG_LOCAL18 },
    { "local19", LOG_LOCAL19 },
    { "local20", LOG_LOCAL20 },
    { "local21", LOG_LOCAL21 },
    { "local22", LOG_LOCAL22 },
    { "local23", LOG_LOCAL23 },
    { "local24", LOG_LOCAL24 },
    { "local25", LOG_LOCAL25 },
    { "local26", LOG_LOCAL26 },
    { "local27", LOG_LOCAL27 },
    { "local28", LOG_LOCAL28 },
    { "local29", LOG_LOCAL29 },
// BEGIN COSMOCOM SPECIFIC SECTION ************************************
    { "ore",		LOG_ORE	},		// SAME AS LOG_LOCAL0 thru 14...
    { "cpxy",		LOG_CPXY },
    { "csv",		LOG_CSV	},
    { "cfm",		LOG_CFM	},
    { "lgsv",		LOG_LGSV },
    { "mcs",		LOG_MCS	},
    { "pstn",		LOG_PSTN },
    { "stat",		LOG_STAT },
    { "ais",		LOG_AIS	},
    { "pingpong",	LOG_PINGPONG },
    { "demo_ore",	LOG_DEMO_ORE },
    { "acom",		LOG_ACOM },
    { "sts",		LOG_STS	},
    { "stsc",		LOG_STSC },
    { "rtr",		LOG_RTR },
    { "vcs",		LOG_VCS },
    { "ipc",		LOG_IPC },
	{ "sts_pblsh",  LOG_STSPBLSH},
	{ "sts_rqst",   LOG_STSRQSTS},
	{ "pds",		LOG_PDS},
	{ "cis",		LOG_CIS},
	{ "dtr",		LOG_DTR},
	{ "webmcs",		LOG_WEBMCS},
// END COSMOCOM SPECIFIC SECTION ************************************
    { NULL, -1 }
  };

#endif

const int LOG_PID					= 0x01;    /* log the pid with each message */
const int LOG_CONS					= 0x02;    /* log on the console if errors in sending */
const int LOG_OUTPUTDEBUGSTRING     = 0x04;    /* log to the DebugView as well*/ 
const int LOG_OUTPUTDEBUGSTRINGONLY = 0x08;    /* log to the DebugView only. This option is intended to be used for client side applications */ 
const int LOG_THREADID				= 0x10;	   /* log the tid with each message */
const int LOG_PERROR				= 0x20;    /* log to stderr as well */    

extern void closelog(void);
extern void openlog(const char * __ident, int __option, int __facility,const char * __host, int __port);
extern void openlog(const wchar_t * __ident, int __option, int __facility,const wchar_t * __host, int __port);
extern bool isOpen(); // check if syslog is already open
extern void syslog(int __pri, const char *__fmt, ...);
extern void syslog(int __pri, const wchar_t *__fmt, ...);

typedef enum
{
	SLOG_DEFAULT	= 0,			// this level makes the syslog() API substitute the FAC code that openlog() was called with.
	SLOG_KERN		= LOG_KERN,
	SLOG_USER		= LOG_USER,
	SLOG_MAIL		= LOG_MAIL,
	SLOG_DAEMON		= LOG_DAEMON,
	SLOG_AUTH		= LOG_AUTH,
	SLOG_SYSLOG		= LOG_SYSLOG,
	SLOG_LPR		= LOG_LPR,
	SLOG_NEWS		= LOG_NEWS,
	SLOG_UUCP		= LOG_UUCP,
	SLOG_CRON		= LOG_CRON,
	SLOG_AUTHPRIV	= LOG_AUTHPRIV,
	SLOG_FTP		= LOG_FTP,
	SLOG_LOCAL0		= LOG_LOCAL0,
	SLOG_LOCAL1		= LOG_LOCAL1,
	SLOG_LOCAL2		= LOG_LOCAL2,
	SLOG_LOCAL3		= LOG_LOCAL3,
	SLOG_LOCAL4		= LOG_LOCAL4,
	SLOG_LOCAL5		= LOG_LOCAL5,
	SLOG_LOCAL6		= LOG_LOCAL6,
	SLOG_LOCAL7		= LOG_LOCAL7,
	SLOG_LOCAL8		= LOG_LOCAL8,
	SLOG_LOCAL9		= LOG_LOCAL9,
	SLOG_LOCAL10	= LOG_LOCAL10,
	SLOG_LOCAL11	= LOG_LOCAL11,
	SLOG_LOCAL12	= LOG_LOCAL12,
	SLOG_LOCAL13	= LOG_LOCAL13,
	SLOG_LOCAL14	= LOG_LOCAL14,
	SLOG_LOCAL15	= LOG_LOCAL15,
	SLOG_LOCAL16	= LOG_LOCAL16,
	SLOG_LOCAL17	= LOG_LOCAL17,
	SLOG_LOCAL18	= LOG_LOCAL18,
	SLOG_LOCAL19	= LOG_LOCAL19,
	SLOG_LOCAL20	= LOG_LOCAL20,
	SLOG_LOCAL21	= LOG_LOCAL21,
	SLOG_LOCAL22	= LOG_LOCAL22,
	SLOG_LOCAL23	= LOG_LOCAL23,
	SLOG_LOCAL24	= LOG_LOCAL24,
	SLOG_LOCAL25	= LOG_LOCAL25,
	SLOG_LOCAL26	= LOG_LOCAL26,
	SLOG_LOCAL27	= LOG_LOCAL27,
	SLOG_LOCAL28	= LOG_LOCAL28,
	SLOG_LOCAL29	= LOG_LOCAL29,
// BEGIN COSMOCOM SPECIFIC SECTION ************************************
	SLOG_ORE		= LOG_ORE,
	SLOG_CPXY		= LOG_CPXY,
	SLOG_CSV		= LOG_CSV,
	SLOG_CFM		= LOG_CFM,
	SLOG_LGSV		= LOG_LGSV,
	SLOG_MCS		= LOG_MCS,
	SLOG_PSTN		= LOG_PSTN,
	SLOG_STAT		= LOG_STAT,
	SLOG_AIS		= LOG_AIS,
	SLOG_PINGPONG	= LOG_PINGPONG,
	SLOG_DEMO_ORE	= LOG_DEMO_ORE,
	SLOG_ACOM		= LOG_ACOM,
	SLOG_STS		= LOG_STS,
	SLOG_STSC		= LOG_STSC,
	SLOG_RTR		= LOG_RTR,
	SLOG_VCS		= LOG_VCS,
	SLOG_IPC		= LOG_IPC,
	SLOG_STSPBLSH	= LOG_STSPBLSH,
	SLOG_STSRQSTS	= LOG_STSRQSTS,
	SLOG_PDS		= LOG_PDS,
	SLOG_CIS		= LOG_CIS,
	SLOG_DTR		= LOG_DTR,
	SLOG_WEBMCS		= LOG_WEBMCS,
// END COSMOCOM SPECIFIC SECTION ************************************
}	slog_class_t;

typedef enum
{
	SLOG_EMERGENCY = 1,
	SLOG_ALERT,
	SLOG_CRITICAL,
	SLOG_ERROR,
	SLOG_WARNING,
	SLOG_NOTICE,
	SLOG_INFO,
	SLOG_DEBUG,
	SLOG_SPECIAL_DEBUG = 13
}	slog_level_t;

extern slog_level_t	g_slog_level_;

/// @brief
/// @ingroup common_subsystem
class Slog : public basic_streambuf<char,char_traits<char> >, public basic_ostream<char, char_traits<char> >
{
private:
	typedef char_traits<char>			_Tr;
	typedef basic_ostream<char, _Tr>	_Myt;
	typedef basic_streambuf<char, _Tr>	_Mysb;

	char m_msgbuf[256];
	int  m_msgpos;
	int	 m_priority;
	bool m_enable;

	bool use_global_tls_buffer;

	void initNewLine();

	void enabled(bool b);

	int  priority();
	void priority(int p);

	bool isTLSAvailable();
public:
	Slog(bool use_global = false);
	virtual ~Slog();

	void level(slog_level_t enable)		{ g_slog_level_ = enable; }
	int  level()						{ return g_slog_level_; }

	void option(int opt);
	int option() const;

	bool isOpen()						{ return ::isOpen(); }
	bool enabled();

	
	void open(const char * ident, int opt, slog_class_t grp, const char * host, int port);
	void open(const char * ident, int opt, slog_class_t grp);
	void close(void);

	void doNotUseTLS(){use_global_tls_buffer=false;}

	_Tr::int_type overflow(_Tr::int_type c);
	std::streamsize xsputn(const char* pChar, std::streamsize n);
	std::streamsize xsputn_impl(const char* pChar, std::streamsize n);
		
	Slog &operator()(const char *ident, slog_class_t grp = SLOG_DEFAULT, slog_level_t level = SLOG_ERROR);
	Slog &operator()(slog_level_t level, slog_class_t grp = SLOG_DEFAULT);
	Slog &operator()(void);

	//////////////////////////////////////////////////////////////////////////////
	//Inserter overloads
	//
	// Overloading the inserter for Slog serves two purposes:
	//
	// (1)	To ensure that we can avoid as much overhead as possible in the 
	//		case where logging is disabled 
	//		(between 2-10x faster, depending on types being serialized)
	//
	// (2)	To give us more flexibility in formatting and accepting different
	//		arguments. 
	//
	//		For example:
	//
	//		Pointers are automatically formatted using sprintf "0x%08lx"
	//
	//		string and wstring can be used interchangeably with slog
	//		NOTE 1:	This causes the use of wslog to be deprecated.
	//		NOTE 2: This causes the use of wide string literals to be deprecated.
	//
	//		We can implicitly convert 64bit Integers, rendering 
	//		the use of makeStringFromUInt64 obsolete with slog so long as
	//		the default formatting ("0x%016I64x") is acceptable.
	//////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////
	//	Supported types
	//
	//	Via Member Inserters:
	//
	//	char *
	//	wchar_t *
	//	bool ("true" | "false")
	//	short
	//	unsigned short
	//	int
	//	unsigned int
	//	long
	//	unsigned long
	//	__int64 ("0x%016I64x" | "-0x%016I64x")
	//	unsigned __int64 ("0x%016I64x")
	//	float
	//	double
	//	long double
	//	void * ("0x%08lx")
	//	std::string
	//	std::wstring
	//
	//	Via NonMember Inserters:
	//	
	//	CommonException (in Common Subsystems, CommonException.h)
	//	endl
	//	setbase(int)
	///////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	//PROTOTYPE FOR ADDING NON-MEMBER INSERTERS
	/*****************************************************************************
	Slog & operator<<(Slog& _O, const TYPE &_X)
	{
		if(_O.enabled())
		{
		}
		return _O;
	}
	*****************************************************************************/

	Slog& operator<<(Slog& (__cdecl *_F)(Slog&));
	Slog& operator<<(const char *_X);
	Slog& operator<<(const wchar_t *_X);
	Slog& operator<<(const bool &_X);
	Slog& operator<<(const short &_X);
	Slog& operator<<(const unsigned short &_X);
	Slog& operator<<(const int &_X);
	Slog& operator<<(const unsigned int &_X);
	Slog& operator<<(const long &_X);
	Slog& operator<<(const unsigned long &_X);
	Slog& operator<<(const __int64 &_X);
	Slog& operator<<(const unsigned __int64 &_X);
	Slog& operator<<(const float &_X);
	Slog& operator<<(const double &_X);
	Slog& operator<<(const long double &_X);
	Slog& operator<<(const void *_X);
	Slog& operator<<(const std::string &_X);
	Slog& operator<<(const std::wstring &_X);
};

extern Slog& __cdecl endl(Slog& _O);
extern Slog& __cdecl operator<<(Slog& _O, const _Fillobj<char>& _X);
extern Slog& __cdecl operator<<(Slog& _O, const _Smanip<int>& _M);

extern Slog  slog;

//useage of wslog is deprecated
#define wslog slog

#endif
