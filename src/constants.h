const char COMET_DEFAULT_LOG_FOLDER[] = "comets_logs\0";
const char COMET_LOG_POSIX_DEFAULT_LOG_FILE_PREFIX[] = "posix_thread_\0";
const char COMET_LOG_POSIX_SYSCALL_START[] = "\n+++++++++ SYSCALL %s (%d) +++++++++";
const char COMET_LOG_POSIX_SYSCALL_END[] = "~~~~~~~~~  END SYSCALL ~~~~~~~~~\n";
const int COMET_LOG_MAX_MSG_LEN                 = 4096;
const char COMET_LOG_PRE_MSG[]        = "at[%.15lu][%.40s]: ";