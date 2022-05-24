import os
import shutil
import socket
import subprocess
import glob

HOST = "192.168.0.95"  # Standard loopback interface address (localhost)
PORT = 6283        # Port to listen on (non-privileged ports are > 1023)
SIZE = 1024
FORMAT = "utf-8"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
print("Listening on port", PORT, "\n")
s.listen()
conn, addr = s.accept()
print(f"New conection at {addr} \n")

os.mkdir("temp_s2")                            # Create new temp file to store created files

#ls = subprocess.run("ls")
wd = os.getcwd()
os.chdir(wd+"/temp_s2")

cost = os.getpid()%3 + 1                    # Cost is initialized as random int
latest_file = "\*ImpossibleFile*"
#r_actions = []                              # Store actions to do all at once

while True:
    data=conn.recv(1024).decode(FORMAT)     # Recieved as a string
    if(data=="\EoR"):
        print(f"Recieved from client: {data}\n")
        print("<<<<<<<<<<>>>>>>>>>>")
        print("<< Closing Server >>")
        print("<<<<<<<<<<>>>>>>>>>>")
        break
    elif (data == "\CostQuery"):
        print(f"Recieved from client: {data}\n")
        print("Cost ===", cost, "\n")
        conn.sendall((str(cost)).encode(FORMAT))   # Send acknowledgement back
        #print(t)
        #time.sleep(t)
    elif (data =="\FileTransfer"):
        print(f"Recieved from client: {data}\n")
        conn.sendall("ack_s2_begin_file_transfer".encode(FORMAT))   # Send acknowledgement back
        NoOfFiles = int(conn.recv(1024).decode(FORMAT))
        print(f"Number of files to come from client:")
        print(f"Recieved from client: {NoOfFiles}\n")
        conn.sendall("ack_s2_recieved_number_of_files".encode(FORMAT))   # Send acknowledgement back
        #print("Number of files:", NoOfFiles)
        while(NoOfFiles!=0):        # RECIVE FILE
            cost += 1 # Recieving a file increases cost for next time
            
            #conn.sendall("ack_send_file".encode(FORMAT))
            filename=conn.recv(1024).decode(FORMAT)                 # Recieve filename
            print(f"~ Recieved from client: {filename}\n")
            conn.sendall("ack_s2_recieved_filename".encode(FORMAT))   # Send acknowledgement back
            latest_file = filename
            file = open(f"{filename}", "wb")
            numsends=conn.recv(1024).decode(FORMAT)
            conn.sendall("server_recieved_loopnum".encode(FORMAT))   # Send acknowledgement back
            print("Start copying file...\n")
            for i in range(int(numsends)+1):
                filedata=conn.recv(1024)#.decode(FORMAT)             # Recieve content
                file.write(filedata)
            conn.sendall("client_recieved_filedata".encode(FORMAT))   # Send acknowledgement back
            file.close()
            # conn.recv(1024).decode(FORMAT)
            NoOfFiles-=1
    elif (data != ""):
        conn.sendall("ack_s2_recieved_action".encode(FORMAT))   # Send acknowledgement back
        cost += 1 # Completing a command increases cost for next time
        pass_var = eval(data)
        print(f"Recieved from client: {pass_var}\n")
        #r_actions.append(pass_var)
        result = subprocess.run(pass_var, stdout=subprocess.PIPE)   # Do processes as they come through (should just do whole actionset)
        send_back = str((result.stdout, result.returncode))         # Should write output to temp directory or something (see lecture on May 4th)         
        # If new file send back
        currDir = os.getcwd()
        list_of_files = glob.glob("*") # * means all if need specific format then *.csv
        newest_file = max(list_of_files, key=os.path.getctime)
        #print(newest_file)
        if ((newest_file != latest_file)):      # SEND FILE
            conn.sendall("\FileTransfer".encode(FORMAT))
            conn.recv(1024).decode(FORMAT)              # Request acknowledged start sending file
            conn.sendall(newest_file.encode(FORMAT))    # Send filename
            conn.recv(1024).decode(FORMAT)
            filesize = os.path.getsize(newest_file)
            print("FILESIZE IS _______ ", filesize, "\nFILENAME IS ________ ", newest_file, "\n")
            file = open(newest_file, 'rb') #open file
            content1 = file.read()
            numsends = int(filesize/1025)
            conn.sendall(str(numsends).encode(FORMAT))       # Send file content
            ack=conn.recv(1024).decode(FORMAT)
            conn.sendall(content1)#.encode(FORMAT))       # Send file content
            ack=conn.recv(1024).decode(FORMAT)
            print(ack)
            conn.sendall("ack_s2_file_sent".encode(FORMAT))
            latest_file = newest_file
        else:
            conn.sendall(send_back.encode(FORMAT))                      # Send back stdout of result & returncode
    
# Can be used to do all actions at one time with threads
#print(f"Actions: {r_actions}")
#for action in r_actions:
#    result = subprocess.run(action, stdout=subprocess.PIPE)
#    send_back = str(result.stdout)
#    conn.sendall(send_back.encode(FORMAT))
    
s.close()   # Close server
os.chdir(wd)
shutil.rmtree("temp_s2")                       # Delete temp file and everything in it