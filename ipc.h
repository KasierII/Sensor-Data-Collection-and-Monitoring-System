// ipc.h
#ifndef IPC_H
#define IPC_H

#include <time.h>

#define SENSOR_COUNT 4          // 4个传感器
#define SENSOR_NAMES {"Temp1", "Temp2", "Humi1", "Pressure"}

typedef struct {
    int sensor_id;              // 0~3
    char name[16];
    double value;
    time_t timestamp;
} SensorData;

// 阈值定义
#define TEMP_MIN    10.0
#define TEMP_MAX    40.0
#define HUMI_MIN    20.0
#define HUMI_MAX    80.0
#define PRESS_MIN   900.0
#define PRESS_MAX   1100.0

#endif
