
#include "comets.hpp"
#include "env.hpp"


inline void setup_posix_log_filename()
{
    if (logfile_path[0] == '\0')
    {
        sprintf(logfile_path, "%s/%s%ld.log", get_host_log_dir(), get_log_prefix(),
                COMET_syscall(SYS_gettid));
    }
}

inline long long current_time_in_millis()
{
    timespec ts{};
    static long long start_time = -1;
    if (start_time == -1)
    {
        COMET_syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
        start_time = static_cast<long long>(ts.tv_sec) * 1000 + (ts.tv_nsec) / 1000000;
    }
    COMET_syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
    auto time_now = static_cast<long long>(ts.tv_sec) * 1000 + (ts.tv_nsec) / 1000000;
    return time_now - start_time;
}

inline void log_write_to(char *buffer, size_t bufflen)
{

    if (current_log_level < COMET_MAX_LOG_LEVEL || COMET_MAX_LOG_LEVEL < 0)
    {
        COMET_syscall(SYS_write, logfileFD, buffer, bufflen);
        COMET_syscall(SYS_write, logfileFD, "\n", 1);
    }
}

/**
 * @brief Class used to suspend the logging capabilities of COMET, by setting the logging_syscall
 * flag to false at instantiation, and restarting the logging at destruction
 *
 */
struct SyscallLoggingSuspender
{
    SyscallLoggingSuspender() { logging_syscall = false; }
    ~SyscallLoggingSuspender() { logging_syscall = true; }
};

/**
 * @brief Class that provides logging capabilities to COMET. It uses the STL it the component is not
 * the intercepting library, otherwise it uses POSIX defined systemcalls.
 *
 */
struct Logger
{
    char invoker[256]{0};
    char file[256]{0};
    char format[COMET_LOG_MAX_MSG_LEN]{0};

    inline Logger(const char invoker[], const char file[], int line, long int tid,
                  const char *message, ...)
    {
        if (!logfileOpen)
        {
            setup_posix_log_filename();
            current_log_level = 0; // reset after clone log level, so to not inherit it

            COMET_syscall(SYS_mkdir, get_log_dir(), 0755);
            COMET_syscall(SYS_mkdir, get_posix_log_dir(), 0755);
            COMET_syscall(SYS_mkdir, get_host_log_dir(), 0755);

            logfileFD = COMET_syscall(SYS_open, logfile_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);

            if (logfileFD == -1)
            {
                COMET_syscall(SYS_write, fileno(stdout),
                              "Err fopen file: ", strlen("Err fopen file: "));
                COMET_syscall(SYS_write, fileno(stdout), logfile_path, strlen(logfile_path));
                COMET_syscall(SYS_write, fileno(stdout), " ", 1);
                COMET_syscall(SYS_write, fileno(stdout), strerror(errno), strlen(strerror(errno)));
                COMET_syscall(SYS_write, fileno(stdout), "\n", 1);
                exit(EXIT_FAILURE);
            }
            else
            {
                logfileOpen = true;
            }
        }
        strncpy(this->invoker, invoker, sizeof(this->invoker));
        strncpy(this->file, file, sizeof(this->file));

        va_list argp, argpc;

        sprintf(format, COMET_LOG_PRE_MSG, current_time_in_millis(), this->invoker);
        size_t pre_msg_len = strlen(format);

        strcpy(format + pre_msg_len, message);

        va_start(argp, message);
        va_copy(argpc, argp);

        if (current_log_level == 0 && logging_syscall)
        {
            int syscallNumber = va_arg(argp, int);
            auto buf1 = reinterpret_cast<char *>(COMET_syscall(
                SYS_mmap, nullptr, 50, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
            sprintf(buf1, COMET_LOG_POSIX_SYSCALL_START, sys_num_to_string(syscallNumber),
                    syscallNumber);
            log_write_to(buf1, strlen(buf1));
            COMET_syscall(SYS_munmap, buf1, 50);
        }

        int size = vsnprintf(nullptr, 0U, format, argp);
        auto buf = reinterpret_cast<char *>(COMET_syscall(SYS_mmap, nullptr, size + 1,
                                                          PROT_READ | PROT_WRITE,
                                                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        vsnprintf(buf, size + 1, format, argpc);
        log_write_to(buf, strlen(buf));

        va_end(argp);
        va_end(argpc);
        COMET_syscall(SYS_munmap, buf, size);
        current_log_level++;
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    inline ~Logger()
    {
        current_log_level--;
        sprintf(format, COMET_LOG_PRE_MSG, current_time_in_millis(), this->invoker);
        size_t pre_msg_len = strlen(format);
        strcpy(format + pre_msg_len, "returned");

        log_write_to(format, strlen(format));
        if (current_log_level == 0 && logging_syscall)
        {
            log_write_to(const_cast<char *>(COMET_LOG_POSIX_SYSCALL_END),
                         strlen(COMET_LOG_POSIX_SYSCALL_END));
        }

    }

    inline void log(const char *message, ...)
    {
        va_list argp, argpc;

        sprintf(format, COMET_LOG_PRE_MSG, current_time_in_millis(), this->invoker);
        size_t pre_msg_len = strlen(format);

        strcpy(format + pre_msg_len, message);

        va_start(argp, message);
        va_copy(argpc, argp);
        int size = vsnprintf(nullptr, 0U, format, argp);
        auto buf = reinterpret_cast<char *>(COMET_syscall(SYS_mmap, nullptr, size + 1,
                                                          PROT_READ | PROT_WRITE,
                                                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        vsnprintf(buf, size + 1, format, argpc);

        log_write_to(buf, strlen(buf));

        va_end(argp);
        va_end(argpc);
        COMET_syscall(SYS_munmap, buf, size);
    }
};


