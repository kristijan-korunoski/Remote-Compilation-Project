import subprocess
import socket

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

# Start using lines at point 2 as 0 was for default port and 1 was for hosts
n = 2
while n < len(all_lines):
    increment = 1
    if (all_lines[n][0][:9] == "actionset"):
        print(f"--- {all_lines[n][0]} ---\n")  # Start doing the cost query
    if (all_lines[n][0][0] == "\t"):
        if (all_lines[n][0][1:8] == "remote-"): # Remote actions start with 'remote-' (after \t is discared)
            # Do remote action
            pass_var = all_lines[n][1:]
            pass_var.insert(0, all_lines[n][0][8:])
            # Check if next line has the required files, try block to handle indexing error when last line
            try:
                if (all_lines[n+1][0][1] == "\t"):
                    increment = 2               # As next line is required for the current action the line increment is set to 2
            except Exception as e:
                increment = 1
            # Send data to server
            send_data = str(pass_var)           # later implement next line files
            
            for i in range(len(hosts)):                         # Runs all remote actions on all servers
                s_array[i].sendall(send_data.encode(FORMAT))    # should select one based on cost, also does this fragment data?
                ack=s_array[i].recv(1024).decode(FORMAT)
                print(f"~ Recieved from server: {ack} ~\n")     # acks should contain more information
                msg=s_array[i].recv(1024).decode(FORMAT)
                print(f"~ Recieved from server: {msg} ~\n")
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
            # Subprocess on client
            result = subprocess.run(pass_var)
            print(f"- Return code: {result.returncode} -\n")
    # By default move by one line
    n += increment