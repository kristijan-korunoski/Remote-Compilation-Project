import subprocess
import socket
import threading

SIZE = 1024
FORMAT = "utf-8"

rakefile = open('Rakefile', 'r')
content = rakefile.readlines()

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
print("\n")

def send_to_server(server, send_data):
    s_array[server].sendall(send_data.encode(FORMAT))   # should select one based on cost, also does this fragment data?
    ack=s_array[server].recv(1024).decode(FORMAT)
    print(f"~ Recieved from server: {ack} ~\n")         # acks should contain more information
    msg=s_array[server].recv(1024).decode(FORMAT)
    print(f"~ Recieved from server: {msg} ~\n")

# Start using lines at point 2 as 0 was for default port and 1 was for hosts
n = 2
while n < len(all_lines):
    increment = 1
    if (all_lines[n][0][:9] == "actionset"):
        print(f"--- {all_lines[n][0]} ---\n")   # Request each server to start doing the cost query ???
    if (all_lines[n][0][0] == "\t"):
        if (all_lines[n][0][1:8] == "remote-"): # Remote actions start with 'remote-' (after \t is discared)
            # Do remote action
            pass_var = all_lines[n][1:]
            pass_var.insert(0, all_lines[n][0][8:])
            # Check if next line has the required files, try block to handle indexing error when last line
            #try:
            if (all_lines[n+1][0] == "\t\trequires"):
                
                s_array[0].sendall((all_lines[n+1][0]).encode(FORMAT))  #sned requires
                s_array[0].recv(1024).decode(FORMAT)     
                numFiles = len(all_lines[n+1])-1          
                s_array[0].sendall((str(numFiles)).encode(FORMAT)) #sned np foles
                s_array[0].recv(1024).decode(FORMAT)
                while numFiles!=0:
                    s_array[0].sendall((all_lines[n+1][numFiles]).encode(FORMAT))  #send filename
                    print(all_lines[n+1][numFiles])
                    s_array[0].recv(1024).decode(FORMAT)
                    #print(all_lines[n+1][0])
                    #print(all_lines[n+1][1])
                    file = open(all_lines[n+1][numFiles], 'r') #open file
                    content1 = file.read()
                    s_array[0].sendall(content1.encode(FORMAT))
                    s_array[0].recv(1024).decode(FORMAT)
                    s_array[0].sendall("\endendend".encode(FORMAT))
                    s_array[0].recv(1024).decode(FORMAT)
                    numFiles-=1
                increment = 2               # As next line is required for the current action the line increment is set to 2
            #except Exception as e:
            #    increment = 1
            # Send data to server
        
            send_data = str(pass_var)           # later implement next line files
            #for i in range(len(hosts)):         # Runs all remote actions on all servers
                                                # it is suggested we use select() instead of threads here
            #    new_thread = threading.Thread(target=send_to_server,args=(i, send_data))
            #    new_thread.start()
            #get file back
                
        else:
            # Format line so it can be passed to subprocess
            pass_var = all_lines[n][1:]
            pass_var.insert(0, all_lines[n][0][1:])
            # Check if next line has the required files, try block to handle indexing error when last line
            try:
                if (all_lines[n+1][0][1] == "\t"):
                    increment = 2               # As next line is required for the current action the line increment is set to 2
            except Exception as e:
                increment = 1
            # We should first add this to an array of actions to be done by client and use threading to complete at once
            # Subprocess on client
            result = subprocess.run(pass_var)
            print(f"- Return code: {result.returncode} -\n")
            
    # By default move by one line
    n += increment
s_array[0].sendall("donedonedonedone".encode(FORMAT))
data2=s_array[0].recv(1024).decode(FORMAT)
if data2=="newfile":
    s_array.sendall("ack_s1".encode(FORMAT))
    newfilename=s_array[0].recv(1024).decode(FORMAT)
    s_array.sendall("ack_s1".encode(FORMAT))
    newfile = open(newfilename, "w")
    while True:
        newfiledata=s_array[0].recv(1024).decode(FORMAT)
        s_array[0].sendall("ack_s1".encode(FORMAT))   # Send acknowledgement back

        if (newfiledata=="\endendend"):
            break
        newfile.write(newfiledata)
    newfile.close()