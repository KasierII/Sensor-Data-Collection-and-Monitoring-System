# client.py
import socket
import sys

s = socket.socket()
s.connect(("127.0.0.1", 9999))
print("已连接到监控服务器，按 Ctrl+C 退出")
while True:
    data = s.recv(1024)
    if not data: break
    print(data.decode('utf-8', errors='ignore'), end='')
