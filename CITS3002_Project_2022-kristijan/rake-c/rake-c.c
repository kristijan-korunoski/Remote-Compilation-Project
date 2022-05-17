#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

void connect_socket(int array[], int index, char *host, int port)
{
  printf("CONNECTION STARTED: %s, %i\n", host, port);
  int socket_desc;
  struct sockaddr_in server;

  //Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }
  server.sin_addr.s_addr = inet_addr(host);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  //Connect to remote server
  if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    puts("connect error");
  }
  else
  {
    puts("Connected");
  }
  array[index] = socket_desc;
}

int main(int argc, char *argv[])
{
  FILE *ptr;
  char *status1;
  char *status2;
  char input1[255], input2[255];
  int numOfLines;

  // Read rakefile
  ptr = fopen("Rakefile", "r");
  if (NULL == ptr)
  {
    printf("file can't be opened \n");
  }
  printf("content of this file are: \n");

  // Counting the number of lines in the file
  // Not including comments
  do
  {

    status1 = fgets(input1, sizeof input1, ptr);
    if (strncmp(input1, "#", strlen("#")) != 0)
    {
      if (strcmp(input1, "") != 10)

        numOfLines += 1;
    }

    // Checking if character is not EOF.
    // If it is EOF stop reading.
  } while (status1);

  // Create array of strings
  char *lines[numOfLines];
  int i = 0;

  // Reopen file
  ptr = fopen("Rakefile", "r");

  // Looping over each line in file
  do
  {
    status2 = fgets(input2, sizeof input2, ptr);
    // Append lines that aren't comments to array
    if (strncmp(input2, "#", strlen("#")) != 0)
    {
      if (strcmp(input2, "") != 10)
      {
        lines[i] = malloc(strlen(input2) + 1);

        strcpy(lines[i], input2);
        i += 1;
      }
    }

    // Checking if character is not EOF.
    // If it is EOF stop reading.
  } while (status2);

  // Get port number from first line
  int defaultPort = atoi(strrchr(lines[0], ' ') + 1);

  printf("Default port is %i\n", defaultPort);

  // Loop through hosts
  char hostLine[256];
  strcpy(hostLine, lines[1]);

  char *hostsString1 = strtok(hostLine, " ");
  ;
  int numOfHosts = 0;
  while (hostsString1 != NULL)
  {
    if (isdigit(hostsString1[0]))
    {
      numOfHosts += 1;
    }
    hostsString1 = strtok(NULL, " ,");
  }

  int hosts_array[numOfHosts];

  hostsString1 = strtok(lines[1], " ");
  ;
  int index = 0;
  while (hostsString1 != NULL)
  {
    if (isdigit(hostsString1[0]))
    {

      if (strchr(hostsString1, ':') != NULL)
      {

        char *s;
        s = strtok(hostsString1, ":");
        int i = 2;
        char *host;
        int port;
        while (i != 0)
        {

          if (i == 2)
            host = s;
          if (i == 1)
            port = atoi(s);
          s = strtok(NULL, " ,");
          i--;
        }
        connect_socket(hosts_array, index, host, port);
        index += 1;
        break;
      }
      else
      {
        connect_socket(hosts_array, index, hostsString1, defaultPort);
        index += 1;
      }
    }
    hostsString1 = strtok(NULL, " ,");
  }

  int n = 2;
  printf("number of lines: %i\n", numOfLines);
  printf("number of hosts: %i\n", numOfHosts);
  char server_message[2000];
  while (n < numOfLines - 1)
  {
    printf("line: %i - %s\n", n, lines[n]);

    if (send(hosts_array[0], lines[n], strlen(lines[n]), 0) < 0)
    {
      printf("Unable to send message\n");
      return -1;
    }
    if (recv(hosts_array[0], server_message, sizeof(server_message), 0) < 0)
    {
      printf("Error while receiving server's msg\n");
      return -1;
    }
    else
    {
      printf("%s\n", server_message);
    }

    n += 1;
  }
}