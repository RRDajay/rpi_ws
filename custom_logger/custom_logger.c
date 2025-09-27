#ifdef MG_ENABLE_CUSTOM_LOG
// Let user define their own mg_log_prefix() and mg_log()
#include <syslog.h>
#include "mongoose.h"

#define RESET   "\033[0m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"

// Define a buffer for the log prefix
static char log_prefix[1024];

// Function to create the log prefix
void mg_log_prefix(int level, const char *file, int line, const char *fname) {
    // Get current time
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    // Create log level string
    const char *level_str;
    const char *color;

    switch (level) {
        case MG_LL_ERROR:
            color = RED;
            level_str = "ERROR";
            break;
        case MG_LL_INFO:
            color = GREEN;
            level_str = "INFO";
            break;
        case MG_LL_DEBUG:
        case MG_LL_VERBOSE:
            color = YELLOW;
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    // Format the prefix with log level, time, file, line, and function name
    snprintf(log_prefix, sizeof(log_prefix),
             "[%s] %s[%s]%s [%s:%d] [%s]", time_buffer, color, level_str, RESET, file, line, fname);
}

// Log function that formats and outputs the log message
void mg_log(const char *fmt, ...) {
    va_list args;
    char msg[1024];

    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // Output the complete log message with the prefix
    printf("%s%s\n", log_prefix, msg);

    // Optionally, send log to syslog (if enabled)
    openlog("mongoose", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s%s", log_prefix, msg);
    closelog();
}

#endif // MG_ENABLE_CUSTOM_LOG