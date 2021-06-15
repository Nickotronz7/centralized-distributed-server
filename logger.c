#include "logger.h"

void log_event(char *tag, char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1023, format, args);

    time_t now;
    time(&now);

    if (tag == NULL)
    {
        tag = "INFO";
    }
    pthread_mutex_lock(&log_mutex);
    FILE *log_file = fopen(LOG_FILE, "a");
    fprintf(log_file, "%s [%s]: %s\n", ctime(&now), tag, buffer);

    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
}