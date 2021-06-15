#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#define ERROR_MESSAGE_SIZE 256

char LOG_FILE[256] = "log.txt";

pthread_mutex_t log_mutex;

void log_event(char *tag, char *format, ...);