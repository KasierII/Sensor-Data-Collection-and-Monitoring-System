// log.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "log.h"

static FILE *log_fp = NULL;

void log_init(const char *filename) {
    log_fp = fopen(filename, "a");
    if (!log_fp) {
        perror("fopen log");
        exit(1);
    }
}

void log_write(const SensorData *data) {
    if (!log_fp) return;
    char timestr[64];
    struct tm *tm_info = localtime(&data->timestamp);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_fp, "[%s] %s (ID:%d) = %.2f\n",
            timestr, data->name, data->sensor_id, data->value);
    fflush(log_fp);
}

void log_close(void) {
    if (log_fp) fclose(log_fp);
}
