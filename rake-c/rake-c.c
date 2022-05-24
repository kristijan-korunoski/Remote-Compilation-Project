#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pthread.h>
#define LSIZ 128 
#define RSIZ 10 
#define SIZE 1024
#if defined(__linux__)
extern char *strdup(const char *str);
#endif

#define SQUOTE '\''
#define DQUOTE '"'

typedef unsigned char BYTE;

char server_message[1024]; // server message buffer

void convert_to_full_line(char *pass_line, char **line_array, int line_len)
{
  printf("len: %i\n", line_len);

  memmove(pass_line, pass_line + 7, strlen(pass_line));
  strcat(pass_line, "[");
  strcat(pass_line, "'");
  memmove(line_array[0], line_array[0] + 7, strlen(line_array[0]));

  if (line_len == 1)
  {
    strcat(pass_line, line_array[0]);
    pass_line[strlen(pass_line) - 1] = '\0';
    strcat(pass_line, "'");
    strcat(pass_line, "]");
  }
  else
  {
    int i = 1;
    strcat(pass_line, line_array[0]);
    strcat(pass_line, "'");
    while (i != line_len - 1)
    {
      strcat(pass_line, ",");
      strcat(pass_line, "'");
      strcat(pass_line, line_array[i]);
      strcat(pass_line, "'");
      i++;
    }
    strcat(pass_line, ",");
    strcat(pass_line, "'");
    strcat(pass_line, line_array[i]);
    pass_line[strlen(pass_line) - 1] = '\0';
    strcat(pass_line, "'");
    strcat(pass_line, "]");
  }
  printf("%s\n", pass_line);
}

_Bool starts_with(const char *restrict string, const char *restrict prefix)
{
  while (*prefix)
  {
    if (*prefix++ != *string++)
      return 0;
  }

  return 1;
}

char **strsplit(const char *str, int *nwords)
{
  char **words = NULL;
  *nwords = 0;

  char *copy = strdup(str);

  if (copy != NULL)
  {
    char *startcopy = copy;

    while (*copy)
    {
      char *start;

      //  SKIP LEADING WHITESPACE
      while (*copy == ' ')
      {
        ++copy;
      }
      if (*copy == '\0')
      {
        break;
      }

      //  COLLECT NEXT WORD - A QUOTED STRING WHICH CAN INCLUDE WHITESPACE
      if (*copy == SQUOTE || *copy == DQUOTE)
      {
        char quote = *copy;

        start = ++copy;
        while (*copy && *copy != quote)
        {
          ++copy;
        }
        if (*copy == '\0')
        { // no closing quote?
          break;
        }
        *copy++ = '\0'; // terminate string to be copied
      }
      //  COLLECT NEXT WORD - AN UNQUOTED STRING
      else
      {
        start = copy;
        while (*copy && (*copy != ' '))
        {
          ++copy;
        }
        if (copy == start)
        {
          break;
        }
        if (*copy != '\0')
        {
          *copy++ = '\0'; // terminate string to be copied
        }
      }

      //  DUPLICATE THE STRING    t <- [start..copy]
      char *word = strdup(start);
      if (word)
      {
        words = realloc(words, (*nwords + 2) * sizeof(words[0]));
        if (words)
        {
          words[*nwords] = word;
          *nwords += 1;
          words[*nwords] = NULL;
        }
        else
        {
          free(word);
          word = NULL;
        }
      }

      //  ANY ERRORS?  DEALLOCATE MEMORY BEFORE RETURNING
      if (word == NULL)
      {
        if (words)
        {
          for (int w = 0; w < *nwords; ++w)
          {
            free(words[w]);
          }
          free(words);
        }
        words = NULL;
        *nwords = 0;
        break;
      }
    }
    free(startcopy);
  }
  return words;
}

// void free_words(char **words)
// {
//   if (words != NULL)
//   {
//     char **t = words;

//     while (*t)
//     {
//       free(*t++);
//     }
//     free(words);
//   }
// }

void connect_socket(int array[], int index, char *host, int port)
{
  printf("CONNECTION STARTED: %s, %i\n", host, port);
  int socket_desc;
  struct sockaddr_in server;

  // Create socket
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
    puts("connection error");
  }
  else
  {
    puts("Connected");
  }
  array[index] = socket_desc;
}

// struct arg_struct {
     int host_sd;
     int host_numid;
     int serverCosts[100];
     int updatedCosts = 0; // count how many costs have been updated
// };

void *cost_query(){
    //struct arg_struct *args = arguments;
    send(host_sd, "\\CostQuery", 10, 0);
    int cost;
    recv(host_sd, server_message, sizeof(server_message), 0);
    cost=atoi(server_message);
    printf("~ Cost from server %i: %i ~", host_numid, cost);
    serverCosts[host_numid] = cost;
    updatedCosts = updatedCosts + 1;
    printf("Updated costs = %i", updatedCosts);
    return NULL;
}

void send_to_server(int server, char *send_data)
{
    char *ack;
    char *msg;
    send(server, send_data, sizeof(send_data), 0);
    recv(server, server_message, sizeof(server_message), 0);
    ack = server_message;
    printf("~ Recieved ack from server: %s ~\n", ack);
    recv(server, server_message, sizeof(server_message), 0);
    msg = server_message;
    printf("~ Recieved from server: %s ~\n", msg);
    if (strcmp(msg, "\\FileTransfer") == 0) // recieve file time
    {
        char *filename;
        send(server, "ack_send_file", sizeof("ack_send_file"), 0);
        recv(server, server_message, sizeof(server_message), 0); // Recieve filename
        filename=server_message;
        printf("~ Recieved from server: %s\n", filename);
        send(server, "client_recieved_filename", sizeof("client_recieved_filename"), 0);
        FILE *file;
        file = fopen(filename, "wb");
        char *numsends;
        recv(server, server_message, sizeof(server_message), 0); // Recieve number of times to recieve
        numsends=server_message;
        send(server, "client_recieved_loopnum", sizeof("client_recieved_loopnum"), 0);
        printf("Start copying file...\n");
        for (int i=0; i < atoi(numsends)+1; i++)
        {
            char *filedata;
            recv(server, server_message, sizeof(server_message), 0); // Recieve content
            filedata=server_message; 
            fwrite(filedata, sizeof(filedata), 1, file);
        }
        send(server, "client_recieved_filedata", sizeof("client_recieved_filedata"), 0);
        fclose(file);
        recv(server, server_message, sizeof(server_message), 0); // Recieve final ack
    }
    //s_array[server].sendall(send_data.encode(FORMAT))
    //ack=s_array[server].recv(1024).decode(FORMAT)
    //print(f"~ Recieved ack from server: {ack} ~\n")
    //msg=s_array[server].recv(1024).decode(FORMAT)
    //print(f"~ Recieved from server: {msg} ~\n")
    //if (msg == "\FileTransfer"):    # RECIVE FILE
    //    s_array[server].sendall("ack_send_file".encode(FORMAT))
    //    filename=s_array[server].recv(1024).decode(FORMAT)                 # Recieve filename
    //    print(f"~ Recieved from server: {filename}\n")
    //    s_array[server].sendall("client_recieved_filename".encode(FORMAT))   # Send acknowledgement back
    //    file = open(f"{filename}", "wb")
    //    numsends=s_array[server].recv(1024).decode(FORMAT)
    //    s_array[server].sendall("client_recieved_loopnum".encode(FORMAT))   # Send acknowledgement back
    //    print("Start copying file...\n")
    //    for i in range(int(numsends)+1):
    //        filedata=s_array[server].recv(1024)                           # Recieve content
    //        file.write(filedata)
    //    s_array[server].sendall("client_recieved_filedata".encode(FORMAT))   # Send acknowledgement back
    //    file.close()
    //    s_array[server].recv(1024).decode(FORMAT)
}


int main(int argc, char *argv[])
{

  FILE *ptr;

  // Read rakefile
  ptr = fopen("Rakefile", "r");
  if (NULL == ptr)
  {
    printf("file can't be opened \n");
  }

  // Counting the number of lines in the file
  // Not including comments

  char *status1;
  char *status2;
  char input1[255], input2[255];
  int numOfLines = 0;

  do
  {
    status1 = fgets(input1, sizeof input1, ptr);
    // Check if comment
    if (strncmp(input1, "#", strlen("#")) != 0)
    {
      // Check if empty line
      if (strcmp(input1, "") != 10)
      {

        numOfLines += 1;
      }
    }

    // Checking if character is not EOF.
    // If it is EOF stop reading.
  } while (status1);

  numOfLines = numOfLines - 2;
  fclose(ptr);

  // Open rakefile for starting actions
  ptr = fopen("Rakefile", "r");
  char **all_lines[numOfLines];
  int line_lengths[numOfLines];
  int i = 0;
  int defaultPort;
  int numOfHosts = 0;
  char **hostline;
  // Interperet lines as host, port, comment/empty, or important
  do
  {
    status1 = fgets(input2, sizeof input2, ptr);
    // Check if comment and ignore
    if (starts_with(input2, "#"))
    {
      continue;
    }
    // Check if empty line and ignore
    else if (strcmp(input2, "") == 10)
    {
        continue;
    }
    // Check if port line and set default port
    else if (starts_with(input2, "PORT"))
    {
      int nwords;
      char **portline = strsplit(input2, &nwords);
      defaultPort = atoi(portline[2]);
    }
    // Check if host line and set host (double check this)
    else if (starts_with(input2, "HOSTS"))
    {
      int nwords;
      hostline = strsplit(input2, &nwords);
      int n;
      for (n = 0; n < nwords; n++)
      {
        if (isdigit(hostline[n][0]))
        {
          numOfHosts++;
        }
      }
    }
    // Otherwise add to all_lines
    else
    {
      int nwords;
      char **words = strsplit(input2, &nwords);
      line_lengths[i] = nwords;
      all_lines[i] = words;
      i++;
    }
    // Checking if character is not EOF.
    // If it is EOF stop reading.
  } while (status1);

  int arrayOfHosts[numOfHosts];
  // Loop through each word from host line (double check this)
  for (int i = 2; i < numOfHosts + 2; i++)
  {
    // if string contains : then split
    if (strchr(hostline[i], ':') != NULL)
    {
      char *s;
      s = strtok(hostline[i], ":");
      int j = 2;
      char *host;
      int port;
      while (j != 0)
      {
        if (j == 2)
          host = s;
        if (j == 1)
          port = atoi(s);
        s = strtok(NULL, " ,");
        j--;
      }
      connect_socket(arrayOfHosts, i - 2, host, port);
    }
    else
    {
      connect_socket(arrayOfHosts, i - 2, hostline[i], defaultPort);
    }
  }

  int n = 0;
  int increment = 1;
  while (n < numOfLines)
  {
    increment = 1;
    char **line = all_lines[n];
    printf("start of line: %s\n", line[0]);
    if (starts_with(line[0], "actionset"))
    {
        // something is wrong with cost calculations
        // Cost query
        /*
        printf("NumHosts = %i\n", numOfHosts);
        printf("--- cost query time ---\n");   // Request each server to start doing the cost query
        int serverCosts[numOfHosts];
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

        for (int i = 0; i < numOfHosts; i++) {
            printf("looped = %i\n", i);
            //pthread_t thread_id;
            host_sd = arrayOfHosts[i];
            host_numid = i;
            cost_query();
            // pthread_create(&thread_id, NULL, cost_query, NULL);
            // pthread_join(thread_id, NULL);
        }
            
        while (0 == 0) // waits for servers to finish cost query
        { 
            if (updatedCosts == numOfHosts)
            {
                break;
            } 
        }

        // Find minimum cost server
        int minCost;
        int optimalServer;
        minCost = serverCosts[0];
        for ( int c = 1 ; c < sizeof(serverCosts) ; c++ ) 
        {
            if ( serverCosts[c] < minCost ) 
            {
                minCost = serverCosts[c];
                optimalServer = c+1;
            }
        }
        printf("\nLowest cost = %i", minCost);
        printf("Lowest cost server = %i", optimalServer);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        */
    }
    else if (starts_with(line[0], "\tremote-"))
    {
      char pass_line[] = "";
      printf("n: %i\n", n);
      int num = n;
      convert_to_full_line(pass_line, line, line_lengths[n]);
      printf("next: %s\n", all_lines[num + 1][0]);

      if (num + 1 < numOfLines) // check that its not the end of the rakefile
      {
        if (starts_with(all_lines[num + 1][0], "\t\trequires"))
        {
          send(arrayOfHosts[0], "\\FileTransfer", 13, 0);
          char *ack;
          recv(arrayOfHosts[0], server_message, sizeof(server_message), 0);
          ack = server_message;
          int numoffiles = line_lengths[num + 1] - 1;
          char numoffiles_str[20];
          sprintf(numoffiles_str, "%d", numoffiles);
          send(arrayOfHosts[0], numoffiles_str, strlen(numoffiles_str), 0);
          recv(arrayOfHosts[0], server_message, sizeof(server_message), 0);
          printf("ack: %s\n", ack);

          char fname[20];
          FILE *fptr = NULL;

          int filenumber = 1;

          while (filenumber != numoffiles + 1)
          {
            char *filename = all_lines[num + 1][filenumber];

             if (filenumber == numoffiles)
             {
               filename[strlen(filename) - 1] = '\0';
             }
            printf("sending file: %s\n", filename);
             
            send(arrayOfHosts[0], filename, strlen(filename), 0);
            recv(arrayOfHosts[0], server_message, sizeof(server_message), 0);
            printf("ack received: %s\n", server_message);

            FILE *ptr;
            // Read rakefile
            ptr = fopen(filename, "r");
            fseek(ptr, 0, SEEK_END);
            // seek to end of file
            int filesize = ftell(ptr);
            // get current file pointer
            fseek(ptr, 0, SEEK_SET); // seek back to beginning of file
            fclose(ptr);
            int numsends = filesize / 1025;
            printf("Numsends : %i", numsends);

            //numsends += 1;

            char numsends_str[1000];
            sprintf(numsends_str, "%d", numsends);

            printf("%s\n", numsends_str);
            send(arrayOfHosts[0], numsends_str, strlen(numsends_str), 0);
            recv(arrayOfHosts[0], server_message, sizeof(server_message), 0);
            printf("ack received: %s\n", server_message);

            char line[RSIZ][LSIZ];
            char fname[20];
            FILE *fptr = NULL; 
            int i = 0;
            int tot = 0;
              
            fptr = fopen(filename, "rb");

            struct stat sb;
            if (stat(filename, &sb) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }
            
            char* file_contents = malloc(sb.st_size);
            fread(file_contents, sb.st_size, 1, fptr);

            send(arrayOfHosts[0], file_contents, sb.st_size, 0);

            recv(arrayOfHosts[0], server_message, sizeof(server_message), 0);
            printf("ack received: %s\n", server_message);
            fclose(ptr);
            filenumber += 1;
          }
          increment = 2;
        }
      }
      
      send_to_server(arrayOfHosts[0], pass_line);

    }
    else
    {
        // subproccess
    }

    n = n + increment;
  }
}