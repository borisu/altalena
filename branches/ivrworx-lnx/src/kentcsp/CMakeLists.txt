include_directories(src src/common)
set (kentcsp_SOURCES src/alt.cpp    src/atomic_impl.h        src/channel_base.h      src/channel.h   src/csprocess.h  src/mutex.h         src/run.h
src/alt.h      src/barrier.h            src/channel_buffers.h    src/kernel.cpp   src/net_channels.h  src/thread_local.h
src/atomic.cpp  src/bucket.h             src/channel_ends.h      src/cppcsp.cpp  src/kernel.h     src/process.cpp     src/time.h
src/atomic.h    src/buffered_channel.h  src/channel_factory.h   src/cppcsp.h    src/mobile.h    src/process.h src/common/barrier_bucket.h src/common/basic.h)

add_library( libcppcsp2 SHARED ${kentcsp_SOURCES})


