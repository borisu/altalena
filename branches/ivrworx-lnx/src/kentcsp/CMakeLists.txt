include(CheckIncludeFiles)
include(CheckTypeSize)

include_directories(.)
set(kentcsp_SOURCES src/cppcsp.cpp src/kernel.cpp src/process.cpp src/atomic.cpp src/alt.cpp)

check_include_files(boost/version.hpp CPPCSP_BOOST)

set(OS_MAJOR "#define CPPCSP_POSIX")
set(OS_MINOR "#define CPPCSP_LINUX")
set(OS_TYPE "${OS_MAJOR}
${OS_MINOR}")

set(SCHED_TYPE  "#define CPPCSP_LONGJMP")
set(CONFIG_CPU_FLAGS "#define CPPCSP_X86")


check_type_size(void* CALC_SIZEOF_VOIDP)

configure_file(cppcsp_config.h.in cppcsp_config.h)

add_definitions(-ggdb3 -Wall -O2 -D_REENTRANT -I. -Woverloaded-virtual)
add_definitions(-Wctor-dtor-privacy -Wsign-promo -Wextra -Wfloat-equal -Wpointer-arith)
add_definitions(-Wcast-align -Wwrite-strings -Wconversion -Wsign-compare -dM) 
 


add_library( libcppcsp2 SHARED ${kentcsp_SOURCES})



