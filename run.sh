#!/bin/bash
make clean
make || { echo "构建失败"; exit 1; }

# 尝试多种终端
if command -v gnome-terminal &> /dev/null; then
    gnome-terminal --title="服务端" -- bash -c './collector; exec bash'
elif command -v konsole &> /dev/null; then
    konsole -e bash -c './collector; exec bash'
elif command -v xfce4-terminal &> /dev/null; then
    xfce4-terminal -e "bash -c './collector; exec bash'"
elif command -v xterm &> /dev/null; then
    xterm -e bash -c './collector; exec bash'
else
    echo "⚠️ 未检测到图形终端，服务将在后台运行"
    ./collector &
    COLLECTOR_PID=$!
    trap 'kill $COLLECTOR_PID 2>/dev/null' EXIT
fi

sleep 2

# 启动客户端（新终端）
if command -v gnome-terminal &> /dev/null; then
    gnome-terminal --title="客户端" -- bash -c 'python3 client.py; exec bash'
elif command -v konsole &> /dev/null; then
    konsole -e bash -c 'python3 client.py; exec bash'
else
    echo "🚀 运行客户端（当前终端）..."
    python3 client.py
fi
