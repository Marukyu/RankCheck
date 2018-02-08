#ifndef SRC_SHARED_EXTERNAL_MINGWTHREADS_MINGWTHREADSAUTO_HPP_
#define SRC_SHARED_EXTERNAL_MINGWTHREADS_MINGWTHREADSAUTO_HPP_

#include <mutex>
#include <thread>
#include <condition_variable>

#include <Shared/Utils/OSDetect.hpp>
#ifdef WOS_WINDOWS
#include <Shared/External/MinGWThreads/mingw.mutex.h>
#include <Shared/External/MinGWThreads/mingw.thread.h>
#include <Shared/External/MinGWThreads/mingw.condition_variable.h>
#endif

#endif
