// sensors.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "ipc.h"

static int sensor_id;

void send_data(int sig) {
    SensorData data = {0}; //初始化为 0
    data.sensor_id = sensor_id;
    data.timestamp = time(NULL);

    if (sensor_id == 0) {
        strcpy(data.name, "Temp1");
        data.value = 25.5 + (rand() % 100) / 10.0;
    } else if (sensor_id == 1) {
        strcpy(data.name, "Temp2");
        data.value = 26.0 + (rand() % 90) / 10.0;
    } else if (sensor_id == 2) {
        strcpy(data.name, "Humi1");
        data.value = 50.0 + (rand() % 300) / 10.0;
    } else if (sensor_id == 3) {
        strcpy(data.name, "Pressure");
        data.value = 1013.25;
    } else {
        return;
    }

    write(STDOUT_FILENO, &data, sizeof(data));
    alarm(2); // 设置定时器
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: sensors <sensor_id>\n");
        exit(1);
    }
    sensor_id = atoi(argv[1]);

    signal(SIGALRM, send_data);
    alarm(2);
    while (1) pause();
}
