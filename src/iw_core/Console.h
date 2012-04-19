//------------------------------------------------------------------------------
// Console.h: interface for the Console manipulators.
//------------------------------------------------------------------------------

#if !defined( CONSOLE_MANIP_H__INCLUDED )
#define CONSOLE_MANIP_H__INCLUDED

//------------------------------------------------------------------------------

//------------------------------------------------------------------"includes"--
#include <iostream>
#include <iomanip>

#ifdef WIN32
#include <windows.h>
#endif

namespace JadedHoboConsole
{

#ifdef WIN32
    static const WORD bgMask( BACKGROUND_BLUE      |
                              BACKGROUND_GREEN     |
                              BACKGROUND_RED       |
                              BACKGROUND_INTENSITY   );
    static const WORD fgMask( FOREGROUND_BLUE      |
                              FOREGROUND_GREEN     |
                              FOREGROUND_RED       |
                              FOREGROUND_INTENSITY   );

    static const WORD fgBlack    ( 0 );
    static const WORD fgLoRed    ( FOREGROUND_RED   );
    static const WORD fgLoGreen  ( FOREGROUND_GREEN );
    static const WORD fgLoBlue   ( FOREGROUND_BLUE  );
    static const WORD fgLoCyan   ( fgLoGreen   | fgLoBlue );
    static const WORD fgLoMagenta( fgLoRed     | fgLoBlue );
    static const WORD fgLoYellow ( fgLoRed     | fgLoGreen );
    static const WORD fgLoWhite  ( fgLoRed     | fgLoGreen | fgLoBlue );
    static const WORD fgGray     ( fgBlack     | FOREGROUND_INTENSITY );
    static const WORD fgHiWhite  ( fgLoWhite   | FOREGROUND_INTENSITY );
    static const WORD fgHiBlue   ( fgLoBlue    | FOREGROUND_INTENSITY );
    static const WORD fgHiGreen  ( fgLoGreen   | FOREGROUND_INTENSITY );
    static const WORD fgHiRed    ( fgLoRed     | FOREGROUND_INTENSITY );
    static const WORD fgHiCyan   ( fgLoCyan    | FOREGROUND_INTENSITY );
    static const WORD fgHiMagenta( fgLoMagenta | FOREGROUND_INTENSITY );
    static const WORD fgHiYellow ( fgLoYellow  | FOREGROUND_INTENSITY );
    static const WORD bgBlack    ( 0 );
    static const WORD bgLoRed    ( BACKGROUND_RED   );
    static const WORD bgLoGreen  ( BACKGROUND_GREEN );
    static const WORD bgLoBlue   ( BACKGROUND_BLUE  );
    static const WORD bgLoCyan   ( bgLoGreen   | bgLoBlue );
    static const WORD bgLoMagenta( bgLoRed     | bgLoBlue );
    static const WORD bgLoYellow ( bgLoRed     | bgLoGreen );
    static const WORD bgLoWhite  ( bgLoRed     | bgLoGreen | bgLoBlue );
    static const WORD bgGray     ( bgBlack     | BACKGROUND_INTENSITY );
    static const WORD bgHiWhite  ( bgLoWhite   | BACKGROUND_INTENSITY );
    static const WORD bgHiBlue   ( bgLoBlue    | BACKGROUND_INTENSITY );
    static const WORD bgHiGreen  ( bgLoGreen   | BACKGROUND_INTENSITY );
    static const WORD bgHiRed    ( bgLoRed     | BACKGROUND_INTENSITY );
    static const WORD bgHiCyan   ( bgLoCyan    | BACKGROUND_INTENSITY );
    static const WORD bgHiMagenta( bgLoMagenta | BACKGROUND_INTENSITY );
    static const WORD bgHiYellow ( bgLoYellow  | BACKGROUND_INTENSITY );

    static class con_dev
    {
        private:
        HANDLE                      hCon;
        DWORD                       cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO  csbi;
        DWORD                       dwConSize;

        public:
        con_dev()
        {
            hCon = GetStdHandle( STD_OUTPUT_HANDLE );
        }
        private:
        void GetInfo()
        {
            GetConsoleScreenBufferInfo( hCon, &csbi );
            dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        }
        public:
        void Clear()
        {
            COORD coordScreen = { 0, 0 };

            GetInfo();
            FillConsoleOutputCharacter( hCon, TEXT(' '),
                                        dwConSize,
                                        coordScreen,
                                        &cCharsWritten );
            GetInfo();
            FillConsoleOutputAttribute( hCon,
                                        csbi.wAttributes,
                                        dwConSize,
                                        coordScreen,
                                        &cCharsWritten );
            SetConsoleCursorPosition( hCon, coordScreen );
        }
        void SetColor( WORD wRGBI, WORD Mask )
        {
            GetInfo();
            csbi.wAttributes &= Mask;
            csbi.wAttributes |= wRGBI;
            SetConsoleTextAttribute( hCon, csbi.wAttributes );
        }
    } console;

    inline std::ostream& clr( std::ostream& os )
    {
        os.flush();
        console.Clear();
        return os;
    };

    inline std::ostream& fg_red( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiRed, bgMask );

        return os;
    }

    inline std::ostream& fg_green( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiGreen, bgMask );

        return os;
    }

    inline std::ostream& fg_blue( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiBlue, bgMask );

        return os;
    }

    inline std::ostream& fg_white( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiWhite, bgMask );

        return os;
    }

    inline std::ostream& fg_cyan( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiCyan, bgMask );

        return os;
    }

    inline std::ostream& fg_magenta( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiMagenta, bgMask );

        return os;
    }

    inline std::ostream& fg_yellow( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgHiYellow, bgMask );

        return os;
    }

    inline std::ostream& fg_black( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgBlack, bgMask );

        return os;
    }

    inline std::ostream& fg_gray( std::ostream& os )
    {
        os.flush();
        console.SetColor( fgGray, bgMask );

        return os;
    }

    inline std::ostream& bg_red( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiRed, fgMask );

        return os;
    }

    inline std::ostream& bg_green( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiGreen, fgMask );

        return os;
    }

    inline std::ostream& bg_blue( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiBlue, fgMask );

        return os;
    }

    inline std::ostream& bg_white( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiWhite, fgMask );

        return os;
    }

    inline std::ostream& bg_cyan( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiCyan, fgMask );

        return os;
    }

    inline std::ostream& bg_magenta( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiMagenta, fgMask );

        return os;
    }

    inline std::ostream& bg_yellow( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgHiYellow, fgMask );

        return os;
    }

    inline std::ostream& bg_black( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgBlack, fgMask );

        return os;
    }

    inline std::ostream& bg_gray( std::ostream& os )
    {
        os.flush();
        console.SetColor( bgGray, fgMask );

        return os;
    }

#elif __linux__

    #define RESET   "\033[0m"
    #define BLACK   "\033[30m"      /* Black */
    #define RED     "\033[31m"      /* Red */
    #define GREEN   "\033[32m"      /* Green */
    #define YELLOW  "\033[33m"      /* Yellow */
    #define BLUE    "\033[34m"      /* Blue */
    #define MAGENTA "\033[35m"      /* Magenta */
    #define CYAN    "\033[36m"      /* Cyan */
    #define WHITE   "\033[37m"      /* White */
    #define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
    #define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
    #define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
    #define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
    #define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
    #define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
    #define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
    #define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

    #define BG_RESET   "\033[0m"
    #define BG_BLACK   "\033[40m"      /* Black */
    #define BG_RED     "\033[41m"      /* Red */
    #define BG_GREEN   "\033[42m"      /* Green */
    #define BG_YELLOW  "\033[43m"      /* Yellow */
    #define BG_BLUE    "\033[44m"      /* Blue */
    #define BG_MAGENTA "\033[45m"      /* Magenta */
    #define BG_CYAN    "\033[46m"      /* Cyan */
    #define BG_WHITE   "\033[47m"      /* White */
    #define BG_BOLDBLACK   "\033[1m\033[40m"      /* Bold Black */
    #define BG_BOLDRED     "\033[1m\033[41m"      /* Bold Red */
    #define BG_BOLDGREEN   "\033[1m\033[42m"      /* Bold Green */
    #define BG_BOLDYELLOW  "\033[1m\033[43m"      /* Bold Yellow */
    #define BG_BOLDBLUE    "\033[1m\033[44m"      /* Bold Blue */
    #define BG_BOLDMAGENTA "\033[1m\033[45m"      /* Bold Magenta */
    #define BG_BOLDCYAN    "\033[1m\033[46m"      /* Bold Cyan */
    #define BG_BOLDWHITE   "\033[1m\033[47m"      /* Bold White */

    inline std::ostream& clr( std::ostream& os )
    {
        os << RESET;
        return os;
    };

    inline std::ostream& fg_red( std::ostream& os )
    {
        os << RED;
        return os;
    }

    inline std::ostream& fg_green( std::ostream& os )
    {
        os << GREEN;
        return os;
    }

    inline std::ostream& fg_blue( std::ostream& os )
    {
        os << BLUE;
        return os;
    }

    inline std::ostream& fg_white( std::ostream& os )
    {
        os << WHITE;
        return os;
    }

    inline std::ostream& fg_cyan( std::ostream& os )
    {
        os << CYAN;
        return os;
    }

    inline std::ostream& fg_magenta( std::ostream& os )
    {
        os << MAGENTA;
        return os;
    }

    inline std::ostream& fg_yellow( std::ostream& os )
    {
        os << YELLOW;
        return os;
    }

    inline std::ostream& fg_black( std::ostream& os )
    {
        os << BLACK;
        return os;
    }

    inline std::ostream& bg_red( std::ostream& os )
    {
        os << BG_RED;
        return os;
    }

    inline std::ostream& bg_green( std::ostream& os )
    {
        os << BG_GREEN;
        return os;
    }

    inline std::ostream& bg_blue( std::ostream& os )
    {
        os << BG_BLUE;
        return os;

    }

    inline std::ostream& bg_white( std::ostream& os )
    {
        os << BG_WHITE;
        return os;
    }

    inline std::ostream& bg_cyan( std::ostream& os )
    {
        os << BG_CYAN;
        return os;
    }

    inline std::ostream& bg_magenta( std::ostream& os )
    {
       os << BG_MAGENTA;
       return os;
    }

    inline std::ostream& bg_yellow( std::ostream& os )
    {
        os << BG_YELLOW;
        return os;
    }

    inline std::ostream& bg_black( std::ostream& os )
    {
        os << BG_BLACK;
        return os;
    }


#else
 #error "Unsupported platform"
#endif

}

//------------------------------------------------------------------------------
#endif //!defined ( CONSOLE_MANIP_H__INCLUDED )

