import os
import shutil
import socket
import subprocess
import time

HOST = "192.168.0.95"  # Standard loopback interface address (localhost)
PORT = 6283            # Port to listen on (non-privileged ports are > 1023)
SIZE = 1024
FORMAT = "utf-8"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
print("Listening on port", PORT, "\n")
s.listen()
conn, addr = s.accept()
print(f"New conection at {addr} \n")

shutil.rmtree("temp")                       # Delete temp file and everything in it
os.mkdir("temp")                            # Create new temp file to store created files

r_actions = []                              # Store actions to do all at once

while True:
    t = os.getpid()%5 + 2
    #print(t)
    time.sleep(t)
    data=conn.recv(1024).decode(FORMAT)     # Recieved as a string
    conn.sendall("ack_s2".encode(FORMAT))   # Send acknowledgement back
    if (data != ""):
        pass_var = eval(data)
        print(f"~ {pass_var} ~\n")
        #r_actions.append(pass_var)
        result = subprocess.run(pass_var, stdout=subprocess.PIPE)   # Do processes as they come through (should just do whole actionset)
        send_back = str((result.stdout, result.returncode))         # Should write output to temp directory or something (see lecture on May 4th)         
        conn.sendall(send_back.encode(FORMAT))                      # Send back stdout of result & returncode
    if not data:
        break
    
# Can be used to do all actions at one time with threads
#print(f"Actions: {r_actions}")
#for action in r_actions:
#    result = subprocess.run(action, stdout=subprocess.PIPE)
#    send_back = str(result.stdout)
#    conn.sendall(send_back.encode(FORMAT))
    
s.close()   # Close server