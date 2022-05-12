from fileinput import filename
import subprocess
import socket
import threading
import os

SIZE = 1024
FORMAT = "utf-8"

rakefile = open('Rakefile', 'r')
content = rakefile.readlines()

wd = os.getcwd()
os.chdir(wd+"/required")

all_lines = []
default_port = 0
hosts = []

# Reads Rakefile, discards any empty lines or # lines, otherwise adds all other lines to all_lines array
for line in content:
    if (line == "\n" or line[0] == "#"):
        continue
    else:
        split_line = line.split("\n")
        split_line = split_line[0].split(" ")
        all_lines.append(split_line)

# Display all lines as array
print(all_lines)

# Set default port number
default_port = all_lines[0][-1]
# Set hosts and if they have specific ports
for listed_hosts in all_lines[1][2:]:
    hosts.append(listed_hosts.split(":"))
print(f"\nDefault port is {default_port}")
print(f"Hosts are {hosts} \n")

# Connection functions
def defaut_connect(host):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, int(default_port)))
    s_array.append(s)
# Connection functions
def newport_connect(host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s_array.append(s)

# Make connections to hosts
s_array = []
for host in hosts:
    if (len(host) == 1):
        defaut_connect(host[0])
    else:
        newport_connect(host[0], int(host[1]))
# Display connections made
print("Connections made:")
for conn in s_array:
    print(conn.getpeername())

def send_to_server(server, send_data):
    s_array[server].sendall(send_data.encode(FORMAT))
    ack=s_array[server].recv(1024).decode(FORMAT)
    print(f"~ Recieved ack from server: {ack} ~\n")
    msg=s_array[server].recv(1024).decode(FORMAT)
    print(f"~ Recieved from server: {msg} ~\n")
    if (msg == "\FileTransfer"):    # RECIVE FILE
        s_array[server].sendall("ack_send_file".encode(FORMAT))
        filename=s_array[server].recv(1024).decode(FORMAT)                 # Recieve filename
        print(f"~ Recieved from server: {filename}\n")
        s_array[server].sendall("client_recieved_filename".encode(FORMAT))   # Send acknowledgement back
        file = open(f"{filename}", "wb")
        numsends=s_array[server].recv(1024).decode(FORMAT)
        s_array[server].sendall("client_recieved_loopnum".encode(FORMAT))   # Send acknowledgement back
        print("Start copying file...\n")
        #while True:
        for i in range(int(numsends)+1):
            filedata=s_array[server].recv(1024)#.decode(FORMAT)             # Recieve content
            file.write(filedata)
            #check = filedata.decode(FORMAT)
            #if (check=="\EoF"):
            #    conn.sendall("client_recieved_\EoF".encode(FORMAT))   # Send acknowledgement back
            #    print(f"~ Recieved from server: {check}\n")
            #    break
        s_array[server].sendall("client_recieved_filedata".encode(FORMAT))   # Send acknowledgement back
        file.close()
        s_array[server].recv(1024).decode(FORMAT)

def cost_query(server):
    server = int(server)
    s_array[server].sendall(("\CostQuery").encode(FORMAT))
    cost=s_array[server].recv(1024).decode(FORMAT)   
    print(f"~ Cost from server {server}: {cost} ~")
    serverCosts[server] = int(cost)
    updatedCosts[0] = updatedCosts[0] + 1

# Start using lines at point 2 as 0 was for default port and 1 was for hosts
n = 2
numlines = len(all_lines)
numhosts = len(hosts)
print("Number of hosts:", numhosts, "\n")
serverCosts = []
optimalServer = -1
for i in range(numhosts):
    serverCosts.append(-1)

while n < numlines:
    increment = 1
    #print(f"/// {all_lines[n]} ///\n")
    if (all_lines[n][0][:9] == "actionset"):
        print(f"--- {all_lines[n][0]} ---\n")   # Request each server to start doing the cost query ???
        updatedCosts = [0]
        print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
        for i in range(numhosts):
            new_thread = threading.Thread(target=cost_query,args=(str(i)))
            new_thread.start()
        while True:
            if (updatedCosts[0] == numhosts):
                break
        minCost = min(serverCosts)
        optimalServer = serverCosts.index(minCost)
        print("\nLowest cost =", minCost)
        print("Lowest cost server =", optimalServer)
        print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n")
    if (all_lines[n][0][0] == "\t"):
        if (all_lines[n][0][1:8] == "remote-"): # Remote actions start with 'remote-' (after \t is discared)
            # Do remote action
            pass_var = all_lines[n][1:]
            pass_var.insert(0, all_lines[n][0][8:])
            # Check if next line has the required files, try block to handle indexing error when last line
            #try:
            #print("number of lines = ", numlines)
            #print("current line = ", n)
            if n+1 < numlines:
                if (all_lines[n+1][0] == "\t\trequires"):   # SEND FILE
                    s_array[optimalServer].sendall(("\FileTransfer").encode(FORMAT))  # Send "\t\trequires" so server knows to recieve files
                    ack=s_array[optimalServer].recv(1024).decode(FORMAT)   
                    print(f"~ Recieved ack from server: {ack} ~\n")  
                    numFiles = len(all_lines[n+1])-1                        # Calculate number of files to send
                    s_array[optimalServer].sendall((str(numFiles)).encode(FORMAT))      # Send number of files to server
                    ack=s_array[optimalServer].recv(1024).decode(FORMAT)
                    print(f"~ Recieved ack from server: {ack} ~\n")
                    while numFiles!=0:                                      # Start sending files
                        fname = all_lines[n+1][numFiles]
                        filesize = os.path.getsize(fname)
                        s_array[optimalServer].sendall(fname.encode(FORMAT))    # Send filename
                        ack=s_array[optimalServer].recv(1024).decode(FORMAT)
                        print(f"~ Recieved ack from server: {ack} ~\n")
                        print("FILESIZE IS _______ ", filesize, "\nFILENAME IS ________ ", fname, "\n")
                        file = open(fname, 'rb') #open file
                        content1 = file.read()
                        numsends = int(filesize/1025)
                        s_array[optimalServer].sendall(str(numsends).encode(FORMAT))       # Send number of times to recv
                        ack=s_array[optimalServer].recv(1024).decode(FORMAT)
                        print(f"~ Recieved ack from server: {ack} ~\n")
                        s_array[optimalServer].sendall(content1)#.encode(FORMAT))                             # Send file content
                        ack=s_array[optimalServer].recv(1024).decode(FORMAT)
                        print(f"~ Recieved ack from server: {ack} ~\n")
                        s_array[optimalServer].sendall("client_file_sent".encode(FORMAT))
                        latest_file = fname
                        numFiles-=1
                    increment = 2               # As next line is required for the current action the line increment is set to 2
            # Send data to server (action)
            send_data = str(pass_var)
            #for i in range(len(hosts)):         # Runs all remote actions on all servers
                                                # it is suggested we use select() instead of threads here
            send_to_server(optimalServer, send_data)
        else:
            # Format line so it can be passed to subprocess
            pass_var = all_lines[n][1:]
            pass_var.insert(0, all_lines[n][0][1:])
            # Check if next line has the required files, try block to handle indexing error when last line
            #try:   Client compiling wont require sending to server
            #    if (all_lines[n+1][0][1] == "\t"):
            #        increment = 2               # As next line is required for the current action the line increment is set to 2
            #except Exception as e:
            #    increment = 1
            # We should first add this to an array of actions to be done by client and use threading to complete at once
            # Subprocess on client
            result = subprocess.run(pass_var)
            print(f"- Return code: {result.returncode} -\n")
    # By default move by one line
    n += increment
for i in range(numhosts):
    s_array[i].sendall("\EoR".encode(FORMAT))

print("<<<<<<<<<<>>>>>>>>>>")
print("<< Closing Client >>")
print("<<<<<<<<<<>>>>>>>>>>")