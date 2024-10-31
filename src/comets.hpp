#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include <limits.h>
#include "constants.h"

#define ENABLE_COMET

inline bool continue_on_error = false; // change behaviour of ERR_EXIT to continue if set to true

inline thread_local bool logfileOpen = false;
inline thread_local int logfileFD = -1;
inline thread_local char logfile_path[PATH_MAX]{'\0'};
inline thread_local int current_log_level = 0;
inline thread_local bool logging_syscall = false; // this variable tells the logger that syscall logging
                                                  // has started and we are not in setup phase

#ifndef COMET_MAX_LOG_LEVEL // COMET max log level. defaults to -1, where everything is logged
#define COMET_MAX_LOG_LEVEL -1
#endif

inline int COMET_LOG_LEVEL = COMET_MAX_LOG_LEVEL;

#ifdef ENABLE_COMET
#define ERR_EXIT(message, ...)       \
    log.log(message, ##__VA_ARGS__); \
    if (!continue_on_error)          \
    {                                \
        exit(EXIT_FAILURE);          \
    }
#define LOG(message, ...) log.log(message, ##__VA_ARGS__)
#define START_LOG(tid, message, ...) \
    Logger log(__func__, __FILE__, __LINE__, tid, message, ##__VA_ARGS__)
#define START_SYSCALL_LOGGING() logging_syscall = true
#define SUSPEND_SYSCALL_LOGGING() SyscallLoggingSuspender sls{};

/**
 * This macro is used to inject code into debug mode. It needs a self calling lambda function,
 * that is a lambda in the following form:
 *
 * [](){}()
 *
 * For example, a debug code to print a value might be injected in this way:
 *
 * int value = 10;
 * DBG(tid, [](int i){printf("%d", i);}(value) );
 *
 * This is useful to print or run debug actions with variables defined outside
 * of the scope of the given lambda function Be careful that the code defined inside the DBG
 * macro is not compiled when building in Release.
 *
 * Be even MORE CAREFUL to not use any STL code inside the DBG lambda function as it could be
 * captured by syscall_intercept (ie do not use std::cout unless you are 100% sure of what you are
 * doing)
 */
#define DBG(tid, lambda)                                                       \
    {                                                                          \
        START_LOG(tid, "[  DBG  ]~~~~~~~~~~~~ START ~~~~~~~~~~~~~~[  DBG  ]"); \
        lambda;                                                                \
        LOG("[  DBG  ]~~~~~~~~~~~~ END  ~~~~~~~~~~~~~~[  DBG  ]");             \
    }

#else

#define ERR_EXIT(message, ...) \
    if (!continue_on_error)    \
    exit(EXIT_FAILURE)
#define LOG(message, ...)
#define START_LOG(tid, message, ...)
#define START_SYSCALL_LOGGING()
#define SUSPEND_SYSCALL_LOGGING()
#define SEM_CREATE_CHECK(sem, source) \
    if (sem == SEM_FAILED)            \
    {                                 \
        __SHM_CHECK_CLI_MSG;          \
    }
#define DBG(tid, lambda)

#endif