# echo-client.py

import subprocess
import socket

HOST = "127.0.01"  # The server's hostname or IP address
PORT = 65432  # The port used by the server
SIZE = 1024
FORMAT = "utf-8"

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    file = open("Rakefile" , "r")
    Lines=file.readlines()
    action=[]
    count=0
    for line in Lines:
        if line.startswith(('\t')):
            print("{}".format(line.strip()))
            words=[]
            for word in line.split():
                words.append(word)
            action.append(words) 
    for i in action:
        n=str(i)
        s.sendall(n.encode(FORMAT))
        result = subprocess.run(i, stdout=subprocess.PIPE , stderr=subprocess.PIPE, text=True)
        print(result.stdout)
    msg=s.recv(1024).decode(FORMAT)
    print(msg)
    print(action)

print(f"Received {msg!r}") 