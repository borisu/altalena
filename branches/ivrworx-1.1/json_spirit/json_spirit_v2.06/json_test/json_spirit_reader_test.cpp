/* Copyright (c) 2007-2008 John W Wilkinson

   This source code can be used for any purpose as long as
   this comment is retained. */

// json spirit version 2.06

#include "json_spirit_reader_test.h"
#include "json_spirit_reader.h"
#include "json_spirit_value.h" 
#include "json_spirit_writer.h" 
#include "utils_test.h"

#include <sstream>
#include <boost/assign/list_of.hpp>
#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>

using namespace json_spirit;
using namespace std;
using namespace boost;
using namespace boost::assign;

namespace
{
    template< class Value_t >
    struct Test_runner
    {
        typedef typename Value_t::String_type     String_t;
        typedef typename Value_t::Object          Object_t;
        typedef typename Value_t::Array           Array_t;
        typedef typename String_t::value_type     Char_t;
        typedef typename String_t::const_iterator Iter_t;
        typedef Pair_impl< String_t >             Pair_t;
        typedef std::basic_istream< Char_t >      Istream_t;

        String_t to_str( const char* c_str )
        {
            return ::to_str< String_t >( c_str );
        }

        Pair_t make_pair( const char* c_name, const char* c_value )
        {
            return Pair_t( to_str( c_name ), to_str( c_value ) );
        }

        Pair_t p1;
        Pair_t p2;
        Pair_t p3;

        Test_runner()
        :   p1( make_pair("name 1", "value 1") )
        ,   p2( make_pair("name 2", "value 2") )
        ,   p3( make_pair("name 3", "value 3") )
        {
        }

        void check_eq( const Object_t& obj_1, const Object_t& obj_2 )
        {
            const typename Object_t::size_type size( obj_1.size() );

            assert_eq( size, obj_2.size() );

            for( typename Object_t::size_type i = 0; i < size; ++i )
            {
                assert_eq( obj_1[i], obj_2[i] ); 
            }
        }

        void test_syntax( const char* c_str, bool expected_result = true )
        {
            Value_t value;

            const bool success = read( to_str( c_str ), value );

            assert_eq( success, expected_result );
        }

        template< typename Int >
        void test_syntax( Int min_int, Int max_int )
        {
            ostringstream os;

            os << "[" << min_int << "," << max_int << "]";

            test_syntax( os.str().c_str() );
        }

        void test_syntax()
        {
            test_syntax( "{}" );
            test_syntax( "{ }" );
            test_syntax( "{\"\":\"\"}" );
            test_syntax( "{\"test\":\"123\"}" );
            test_syntax( "{\"test\" : \"123\"}" );
            test_syntax( "{\"testing testing testing\":\"123\"}" );
            test_syntax( "{\"\":\"abc\"}" );
            test_syntax( "{\"abc\":\"\"}" );
            test_syntax( "{\"\":\"\"}" );
            test_syntax( "{\"test\":true}" );
            test_syntax( "{\"test\":false}" );
            test_syntax( "{\"test\":null}" );
            test_syntax( "{\"test1\":\"123\",\"test2\":\"456\"}" );
            test_syntax( "{\"test1\":\"123\",\"test2\":\"456\",\"test3\":\"789\"}" );
            test_syntax( "{\"test1\":{\"test2\":\"123\",\"test3\":\"456\"}}" );
            test_syntax( "{\"test1\":{\"test2\":{\"test3\":\"456\"}}}" );
            test_syntax( "{\"test1\":[\"a\",\"bb\",\"cc\"]}" );
            test_syntax( "{\"test1\":[true,false,null]}" );
            test_syntax( "{\"test1\":[true,\"abc\",{\"a\":\"b\"},{\"d\":false},null]}" );
            test_syntax( "{\"test1\":[1,2,-3]}" );
            test_syntax( "{\"test1\":[1.1,2e4,-1.234e-34]}" );
            test_syntax( "{\n"
                          "\t\"test1\":\n"
                          "\t\t{\n"
                          "\t\t\t\"test2\":\"123\",\n"
                          "\t\t\t\"test3\":\"456\"\n"
                          "\t\t}\n"
                          "}\n" );
            test_syntax( "[]" );
            test_syntax( "[ ]" );
            test_syntax( "[1,2,3]" );
            test_syntax( "[ 1, -2, 3]" );
            test_syntax( "[ 1.2, -2e6, -3e-6 ]" );
            test_syntax( "[ 1.2, \"str\", -3e-6, { \"field\" : \"data\" } ]" );

            test_syntax( INT_MIN, INT_MAX );
            test_syntax( LLONG_MIN, LLONG_MAX );
            test_syntax( "[1 2 3]", false );
        }

        Value_t read_cstr( const char* c_str )
        {
            Value_t value;

            read( to_str( c_str ), value );

            return value;
        }

        bool read_cstr( const char* c_str, Value_t& value )
        {
            return read( to_str( c_str ), value );
        }

        void check_read( const char* c_str, Value_t& value, bool expected_result = true )
        {
            assert_eq( read_cstr( c_str, value ), expected_result );
        }

        void check_reading( const char* c_str )
        {
            Value_t value;

            String_t in_s( to_str( c_str ) );

            const bool success = read( in_s, value );

            assert_eq( success, true );

            const String_t result = write_formatted( value ); 

//            cout << in_s.c_str() << endl << result.c_str() << endl ;

            assert_eq( in_s, result );
        }

        template< typename Int >
        void check_reading( Int min_int, Int max_int )
        {
            ostringstream os;

            os << "[\n"
                   "    " << min_int << ",\n"
                   "    " << max_int << "\n"
                   "]";

            check_reading( os.str().c_str() );
        }

        void test_reading()
        {
            check_reading( "{\n}" );

            Value_t value;

            check_read( "{\n"
                        "    \"name 1\" : \"value 1\"\n"
                        "}", value );

            check_eq( value.get_obj(), list_of( p1 ) );

            check_read( "{\"name 1\":\"value 1\",\"name 2\":\"value 2\"}", value );

            check_eq( value.get_obj(), list_of( p1 )( p2 ) );

            check_read( "{\n"
                        "    \"name 1\" : \"value 1\",\n"
                        "    \"name 2\" : \"value 2\",\n"
                        "    \"name 3\" : \"value 3\"\n"
                        "}", value );

            check_eq( value.get_obj(), list_of( p1 )( p2 )( p3 ) );

            check_read( "{\n"
                        "    \"\" : \"value\",\n"
                        "    \"name\" : \"\"\n"
                        "}", value );

            check_eq( value.get_obj(), list_of( make_pair( "", "value" ) )( make_pair( "name", "" ) ) );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : {\n"
                            "        \"name 3\" : \"value 3\",\n"
                            "        \"name_4\" : \"value_4\"\n"
                            "    }\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : {\n"
                            "        \"name 3\" : \"value 3\",\n"
                            "        \"name_4\" : \"value_4\",\n"
                            "        \"name_5\" : {\n"
                            "            \"name_6\" : \"value_6\",\n"
                            "            \"name_7\" : \"value_7\"\n"
                            "        }\n"
                            "    }\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : {\n"
                            "        \"name 3\" : \"value 3\",\n"
                            "        \"name_4\" : {\n"
                            "            \"name_5\" : \"value_5\",\n"
                            "            \"name_6\" : \"value_6\"\n"
                            "        },\n"
                            "        \"name_7\" : \"value_7\"\n"
                            "    }\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : {\n"
                            "        \"name 3\" : \"value 3\",\n"
                            "        \"name_4\" : {\n"
                            "            \"name_5\" : \"value_5\",\n"
                            "            \"name_6\" : \"value_6\"\n"
                            "        },\n"
                            "        \"name_7\" : \"value_7\"\n"
                            "    },\n"
                            "    \"name_8\" : \"value_8\",\n"
                            "    \"name_9\" : {\n"
                            "        \"name_10\" : \"value_10\"\n"
                            "    }\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : {\n"
                            "        \"name 2\" : {\n"
                            "            \"name 3\" : {\n"
                            "                \"name_4\" : {\n"
                            "                    \"name_5\" : \"value\"\n"
                            "                }\n"
                            "            }\n"
                            "        }\n"
                            "    }\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : true,\n"
                            "    \"name 3\" : false,\n"
                            "    \"name_4\" : \"value_4\",\n"
                            "    \"name_5\" : true\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : null,\n"
                            "    \"name 3\" : \"value 3\",\n"
                            "    \"name_4\" : null\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : 123,\n"
                            "    \"name 3\" : \"value 3\",\n"
                            "    \"name_4\" : -567\n"
                            "}" );

            check_reading( "{\n"
                            "    \"name 1\" : \"value 1\",\n"
                            "    \"name 2\" : 1.200000000000000,\n"
                            "    \"name 3\" : \"value 3\",\n"
                            "    \"name_4\" : 1.234567890123456e+125,\n"
                            "    \"name_5\" : -1.234000000000000e-123,\n"
                            "    \"name_6\" : 1.000000000000000e-123,\n"
                            "    \"name_7\" : 1234567890.123456\n"
                            "}" );

            check_reading( "[\n]" );

            check_reading( "[\n"
                           "    1\n"
                           "]" );

            check_reading( "[\n"
                           "    1,\n"
                           "    1.200000000000000,\n"
                           "    \"john\",\n"
                           "    true,\n"
                           "    false,\n"
                           "    null\n"
                           "]" );

            check_reading( "[\n"
                           "    1,\n"
                           "    [\n"
                           "        2,\n"
                           "        3\n"
                           "    ]\n"
                           "]" );

            check_reading( "[\n"
                           "    1,\n"
                           "    [\n"
                           "        2,\n"
                           "        3\n"
                           "    ],\n"
                           "    [\n"
                           "        4,\n"
                           "        [\n"
                           "            5,\n"
                           "            6,\n"
                           "            7\n"
                           "        ]\n"
                           "    ]\n"
                           "]" );

            check_reading( "[\n"
                           "    {\n"
                           "        \"name\" : \"value\"\n"
                           "    }\n"
                           "]" );

            check_reading( "{\n"
                           "    \"name\" : [\n"
                           "        1\n"
                           "    ]\n"
                           "}" );

            check_reading( "[\n"
                           "    {\n"
                           "        \"name 1\" : \"value\",\n"
                           "        \"name 2\" : [\n"
                           "            1,\n"
                           "            2,\n"
                           "            3\n"
                           "        ]\n"
                           "    }\n"
                           "]" );

            check_reading( "{\n"
                           "    \"name 1\" : [\n"
                           "        1,\n"
                           "        {\n"
                           "            \"name 2\" : \"value 2\"\n"
                           "        }\n"
                           "    ]\n"
                           "}" );

            check_reading( "[\n"
                           "    {\n"
                           "        \"name 1\" : \"value 1\",\n"
                           "        \"name 2\" : [\n"
                           "            1,\n"
                           "            2,\n"
                           "            {\n"
                           "                \"name 3\" : \"value 3\"\n"
                           "            }\n"
                           "        ]\n"
                           "    }\n"
                           "]" );

            check_reading( "{\n"
                           "    \"name 1\" : [\n"
                           "        1,\n"
                           "        {\n"
                           "            \"name 2\" : [\n"
                           "                1,\n"
                           "                2,\n"
                           "                3\n"
                           "            ]\n"
                           "        }\n"
                           "    ]\n"
                           "}" );

            check_reading( INT_MIN, INT_MAX );
            check_reading( LLONG_MIN, LLONG_MAX );
        }

        void test_from_stream()
        {
            Value_t value;

            String_t in_s( to_str( "[1,2]" ) );

            basic_istringstream< Char_t > is( in_s );

            const bool success = read( is, value );

            assert_eq( success, true );

            assert_eq( in_s, write( value ) );
       }

        void test_escape_chars( const char* json_str, const char* c_str )
        {
            Value_t value;

            string s( string( "{\"" ) + json_str + "\" : \"" + json_str + "\"} " );

            check_read( s.c_str(), value );

            const Pair_t& pair( value.get_obj()[0] );

            assert_eq( pair.name_,  to_str( c_str ) );
            assert_eq( pair.value_, to_str( c_str ) );
        }

        void test_escape_chars()
        {
            test_escape_chars( "\\t", "\t");
            test_escape_chars( "a\\t", "a\t" );
            test_escape_chars( "\\tb", "\tb" );
            test_escape_chars( "a\\tb", "a\tb" );
            test_escape_chars( "a\\tb", "a\tb" );
            test_escape_chars( "a123\\tb", "a123\tb" );
            test_escape_chars( "\\t\\n\\\\", "\t\n\\" );
            test_escape_chars( "\\/\\r\\b\\f\\\"", "/\r\b\f\"" );
            test_escape_chars( "\\h\\j\\k", "" ); // invalid esc chars
            test_escape_chars( "\\x61\\x62\\x63", "abc" );
            test_escape_chars( "a\\x62c", "abc" );
            test_escape_chars( "\\x01\\x02\\x7F", "\x01\x02\x7F" ); // NB x7F is the greatest char spirit will parse
            test_escape_chars( "\\u0061\\u0062\\u0063", "abc" );
        }

        void check_is_null( const char* c_str  )
        {
            assert_eq( read_cstr( c_str ).type(), null_type ); 
        }

        template< typename T >
        void check_value( const char* c_str, const T& expected_value )
        {
            const Value_t v( read_cstr( c_str ) );
            
            assert_eq( v.template get_value< T >(), expected_value ); 
        }

        void test_values()
        {
            check_value( "1",        1 );
            check_value( "1.5",      1.5 );
            check_value( "\"Test\"", to_str( "Test" ) );
            check_value( "true",     true );
            check_value( "false",    false );
            check_is_null( "null" );
        }

        void check_read_fails( const char* c_str )
        {
            Value_t value;

            check_read( c_str, value, false );
        }

        void test_error_cases()
        {
            check_read_fails( "[\"1\\\\\",\"2\\\"]" );
            check_read_fails( "." );
            check_read_fails( "1 1" );
            check_read_fails( "\"\"\"" );
            check_read_fails( "'1'" );
            check_read_fails( "{1 2}" );
            check_read_fails( "1.263Q" );
            check_read_fails( "1.26 3" );
            check_read_fails( "'" );
            check_read_fails( "[1 2]" );
        }

        void run_tests()
        {
            test_syntax();
            test_reading();
            test_from_stream();
            test_escape_chars();
            test_values();
            test_error_cases();
        }
    };

#ifndef BOOST_NO_STD_WSTRING
    void test_wide_esc_u()
    {
        wValue value;

        const bool success = read( L"[\"\\uABCD\"]", value );

        assert( success );

        const wstring s( value.get_array()[0].get_str() );

        assert_eq( s.length(), static_cast< wstring::size_type >( 1u ) );
        assert_eq( s[0], 0xABCD );
    }
#endif

    void test_extended_ascii( const string& s )
    {
        Value value;

        const bool success = read( "[\"" + s + "\"]", value );

        assert_eq( success, true );

        assert_eq( value.get_array()[0].get_str(), "äöüß" );
    }

    void test_extended_ascii()
    {
        test_extended_ascii( "\\u00E4\\u00F6\\u00FC\\u00DF" );
        test_extended_ascii( "äöüß" );
    }
}

void json_spirit::test_reader()
{
    Test_runner< Value >().run_tests();

#ifndef BOOST_NO_STD_WSTRING
    Test_runner< wValue >().run_tests();
    test_wide_esc_u();
#endif

    test_extended_ascii();

    //Object obj;

    //for( int i = 0; i < 100000; ++i )
    //{
    //    obj.push_back( Pair( "\x01test\x7F", lexical_cast< string >( i ) ) );
    //}

    //const string s = write( obj );

    //Value value;

    //timer t;

    //read( s, value );

    //cout << t.elapsed() << endl;

    //cout << "obj size " << value.get_obj().size();
}
