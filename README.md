传感器数据采集与监控系统

一个基于 C 语言 + Linux 多进程 的实时传感器模拟与监控系统，完整实现数据生成、进程通信、阈值告警、日志记录与网络输出，适用于嵌入式、物联网及系统编程学习与展示。

🔧 核心功能
多传感器模拟：多个独立子进程定时生成随机温湿度数据（如温度 0~50℃，湿度 20%~90%）
高效进程通信：使用 共享内存 + 消息队列 + 信号量 实现主控进程与传感器间安全通信
实时阈值告警：一旦温度/湿度超出设定范围，立即在控制台输出红色警告
结构化日志记录：所有数据按时间戳写入 sensor_log.txt，格式清晰可分析
远程监控支持：可选 TCP Socket 接口，配合 Python 客户端实现实时数据可视化
一键启动：通过 run.sh 自动编译并运行整个系统

🏗️ 技术栈与能力体现
语言：C（POSIX 标准）
并发模型：多进程（fork）、信号（SIGALRM 定时）
IPC 机制：共享内存（shmget）、消息队列（msgsnd/msgrcv）、信号量（semop）
系统编程：文件 I/O、日志系统、TCP Socket 服务端
工程实践：Makefile 自动化构建、Bash 脚本部署、模块化代码设计

▶ 快速启动

git clone https://github.com/KaiserII/Sensor-Data-Collection-and-Monitoring-System.git

cd Sensor-Data-Collection-and-Monitoring-System

chmod +x run.sh

./run.sh                # 一键编译并运行

tail -f sensor_log.txt  # 查看实时日志
