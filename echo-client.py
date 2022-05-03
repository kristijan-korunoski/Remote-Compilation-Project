# echo-client.py

import subprocess
import socket

HOST = "127.0.01"  # The server's hostname or IP address
PORT = 65432  # The port used by the server
SIZE = 1024
FORMAT = "utf-8"
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    #s.sendall(b"Hello, world")
    file = open("Rakefile" , "r")
    Lines=file.readlines()
    action=[]
    count=0
    for line in Lines:
        if line.startswith(('\t')):
            print("{}".format(line.strip()))
            words=[]
            #words.append(line.strip())
            for word in line.split():
                words.append(word)
            action.append(words) 
                   
        #count+=1
        #a=(line.strip()).encode()
        #s.sendall(a)
        #print("{}".format(line.strip()))
    
    for i in action:
        n=str(i)


        s.sendall(n.encode(FORMAT))
        result = subprocess.run(i, stdout=subprocess.PIPE , stderr=subprocess.PIPE, text=True)
        print(result.stdout)

    #result = subprocess.run(["cat", "square.c"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    msg=s.recv(1024).decode(FORMAT)
    print(msg)
    print(action)
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as ss:
    ss.connect((HOST, PORT))
    ss.sendall(b"Hello, world")
    file = open("Rakefile" , "r")
    Lines=file.readlines()
    action=[]
    count=0
    for line in Lines:
        if line.startswith(('\t')):
            print("{}".format(line.strip()))
            words=[]
            #words.append(line.strip())
            for word in line.split():
                words.append(word)
                s.sendall(word.encode(FORMAT))
                s.sendall("/n".encode(FORMAT))
            action.append(words) 
                   
        #count+=1
        #a=(line.strip()).encode()
        #s.sendall(a)
        #print("{}".format(line.strip()))
    
    for i in action:
        result = subprocess.run(i, stdout=subprocess.PIPE , stderr=subprocess.PIPE, text=True)
        print(result.stdout)

    #result = subprocess.run(["cat", "square.c"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    msg=s.recv(1024).decode(FORMAT)
    print(msg)
    print(action)    
    

#print(result)

print(f"Received {msg!r}") 