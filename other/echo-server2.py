# echo-server.py

import socket
import subprocess

HOST = "192.168.0.95"  # My IPv4 address (at home)
PORT = 6333  # Port to listen on (non-privileged ports are > 1023)
SIZE = 1024
FORMAT = "utf-8"
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print(f"Connected by {addr}")
        saved=[]
        while True:
            data=conn.recv(1024).decode(FORMAT)
            saved.append(data)
            if not data:
                break
            #conn.sendall(data)
            conn.sendall("aman".encode(FORMAT))
    action=[]
    for i in saved:
        if i!="" :
            data1=eval(i)
            action.append(data1)
    for i in action:
        n=str(i)
        result = subprocess.run(i, stdout=subprocess.PIPE , stderr=subprocess.PIPE, text=True)
        print(result.stdout)