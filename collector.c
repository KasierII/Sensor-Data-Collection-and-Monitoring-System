#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "ipc.h"
#include "log.h"

#define MAX_CLIENTS 10

int server_fd = -1;
#define SERVER_PORT 9999

// 全局客户端管理
int client_socks[MAX_CLIENTS];
int client_count = 0;

void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_socks[i] = -1;
    }
    client_count = 0;
}

void check_alarm(const SensorData *data) {
    int alarm = 0;
    double minv = 0, maxv = 0;
    const char *type = "";
    switch(data->sensor_id) {
        case 0: case 1: minv=TEMP_MIN; maxv=TEMP_MAX; type="温度"; break;
        case 2: minv=HUMI_MIN; maxv=HUMI_MAX; type="湿度"; break;
        case 3: minv=PRESS_MIN; maxv=PRESS_MAX; type="气压"; break;
    }
    if (data->value < minv || data->value > maxv) {
        alarm = 1;
        printf("\033[41;37m【报警】%s %s 超出阈值！当前: %.2f (范围: %.1f~%.1f)\033[0m\n",
               data->name, type, data->value, minv, maxv);
    }

    // 广播给所有客户端
    char buf[256];
    int n = snprintf(buf, sizeof(buf), "%s%s: %.2f%s\n",
                    alarm ? "[ALARM] " : "", data->name, data->value, alarm ? " !!!" : "");

    for (int i = 0; i < client_count; i++) {
        if (client_socks[i] >= 0) {
            ssize_t sent = write(client_socks[i], buf, n);
            if (sent <= 0) {
                // 客户端断开
                printf("Client fd=%d disconnected.\n", client_socks[i]);
                close(client_socks[i]);
                client_socks[i] = client_socks[--client_count];
                i--; // 重查当前位置
            }
        }
    }
}

// 退出
volatile sig_atomic_t keep_running = 1;
void signal_handler(int sig) {
    keep_running = 0;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    init_clients();
    int pipes[SENSOR_COUNT][2];
    log_init("sensor_log.txt");
    printf("=== 传感器监控系统启动 ===\n");

    // 创建 TCP 服务器
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        server_fd = -1;
    } else {
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
        }
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_PORT);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(server_fd);
            server_fd = -1;
        } else if (listen(server_fd, 5) < 0) {
            perror("listen");
            close(server_fd);
            server_fd = -1;
        } else {
            printf("Socket 监控服务已开启，端口: %d\n", SERVER_PORT);
        }
    }

    // 启动传感器子进程
    for (int i = 0; i < SENSOR_COUNT; i++) {
        pipe(pipes[i]);
        if (fork() == 0) {
            // 子进程
            close(pipes[i][0]); // 关闭读端
            dup2(pipes[i][1], STDOUT_FILENO);
            close(pipes[i][1]);

            int nullfd = open("/dev/null", O_RDONLY);
            dup2(nullfd, STDIN_FILENO);
            close(nullfd);

            char id_str[16];
            snprintf(id_str, sizeof(id_str), "%d", i);
            execl("./sensors", "sensors", id_str, (char *)NULL);

            perror("execl failed");
            exit(1);
        }
        // 父进程
        close(pipes[i][1]); // 关闭写端
        printf("传感器进程 %d 已启动\n", i);
    }

    fd_set rfd;
    SensorData data;

    while (keep_running) {
        FD_ZERO(&rfd);
        int mf = 0;

        // 添加传感器管道到 select
        for (int i = 0; i < SENSOR_COUNT; i++) {
            FD_SET(pipes[i][0], &rfd);
            if (pipes[i][0] > mf) mf = pipes[i][0];
        }

        // 添加服务器 socket（用于 accept 新连接）
        if (server_fd != -1) {
            FD_SET(server_fd, &rfd);
            if (server_fd > mf) mf = server_fd;
        }

        struct timeval tv = {1, 0}; // 1秒超时，避免完全阻塞
        int activity = select(mf + 1, &rfd, NULL, NULL, &tv);

        if (activity > 0) {
            // 1. 处理新客户端连接
            if (server_fd != -1 && FD_ISSET(server_fd, &rfd)) {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
                if (client_fd >= 0) {
                    if (client_count < MAX_CLIENTS) {
                        client_socks[client_count++] = client_fd;
                        printf("✅ 新客户端连接 (fd=%d)，当前客户端数: %d\n", client_fd, client_count);
                    } else {
                        printf("❌ 客户端数量已达上限，拒绝新连接。\n");
                        close(client_fd);
                    }
                }
            }

            // 2. 处理传感器数据
            for (int i = 0; i < SENSOR_COUNT; i++) {
                if (FD_ISSET(pipes[i][0], &rfd)) {
                    ssize_t n = read(pipes[i][0], &data, sizeof(data));
                    if (n == sizeof(data)) {
                        char t[64];
                        strftime(t, 64, "%Y-%m-%d %H:%M:%S", localtime(&data.timestamp));
                        printf("[%s] %s: %.2f\n", t, data.name, data.value);
                        log_write(&data);
                        check_alarm(&data);
                    } else if (n <= 0) {
                        // 管道关闭（子进程退出）
                        printf("⚠️ 传感器 %d 管道关闭。\n", i);
                    }
                }
            }
        }
    }

    // 清理资源
    log_close();
    if (server_fd != -1) close(server_fd);
    for (int i = 0; i < client_count; i++) {
        close(client_socks[i]);
    }
    printf("\nCollector 已安全退出。\n");
    return 0;
}
