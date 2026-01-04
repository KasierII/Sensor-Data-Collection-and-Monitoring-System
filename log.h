// log.h
#ifndef LOG_H
#define LOG_H
#include "ipc.h"
void log_init(const char *filename);
void log_write(const SensorData *data);
void log_close(void);
#endif
