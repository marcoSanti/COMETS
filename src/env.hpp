#include <limits.h>

inline auto get_hostname()
{
    static char *hostname_prefix = nullptr;
    if (hostname_prefix == nullptr)
    {
        hostname_prefix = new char[HOST_NAME_MAX];
        gethostname(hostname_prefix, HOST_NAME_MAX);
    }
    return hostname_prefix;
}

inline auto get_log_dir()
{
    static char *posix_log_master_dir_name = nullptr;
    if (posix_log_master_dir_name == nullptr)
    {
        posix_log_master_dir_name = std::getenv("COMET_LOG_DIR");
        if (posix_log_master_dir_name == nullptr)
        {
            posix_log_master_dir_name = new char[strlen(COMET_DEFAULT_LOG_FOLDER)];
            strcpy(posix_log_master_dir_name, COMET_DEFAULT_LOG_FOLDER);
        }
    }
    return posix_log_master_dir_name;
}

inline auto get_log_prefix()
{
    static char *posix_logfile_prefix = nullptr;
    if (posix_logfile_prefix == nullptr)
    {
        posix_logfile_prefix = std::getenv("COMET_LOG_PREFIX");
        if (posix_logfile_prefix == nullptr)
        {
            posix_logfile_prefix = new char[strlen(COMET_LOG_POSIX_DEFAULT_LOG_FILE_PREFIX)];
            strcpy(posix_logfile_prefix, COMET_LOG_POSIX_DEFAULT_LOG_FILE_PREFIX);
        }
    }
    return posix_logfile_prefix;
}

inline auto get_posix_log_dir()
{
    static char *posix_log_dir_path = nullptr;
    if (posix_log_dir_path == nullptr)
    {
        // allocate space for a path in the following structure (including 10 digits for thread id
        // max
        //  log_master_dir_name/posix/hostname/logfile_prefix_<tid>.log
        auto len = strlen(get_log_dir()) + 7;
        posix_log_dir_path = new char[len]{0};
        sprintf(posix_log_dir_path, "%s/posix", get_log_dir());
    }
    return posix_log_dir_path;
}

inline auto get_host_log_dir()
{
    static char *host_log_dir_path = nullptr;
    if (host_log_dir_path == nullptr)
    {
        // allocate space for a path in the following structure (including 10 digits for thread id
        // max
        //  log_master_dir_name/posix/hostname/logfile_prefix_<tid>.log
        auto len = strlen(get_posix_log_dir()) + HOST_NAME_MAX;
        host_log_dir_path = new char[len]{0};
        sprintf(host_log_dir_path, "%s/%s", get_posix_log_dir(), get_hostname());
    }
    return host_log_dir_path;
}