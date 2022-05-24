#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#if defined(__linux__)
extern char *strdup(const char *str);
#endif

#define SQUOTE '\''
#define DQUOTE '"'

typedef unsigned char BYTE;

void convert_to_full_line(char *pass_line, char **line_array, int line_len)
{

  // strcpy(pass_line, "[");
  strcat(pass_line, "[");
  strcat(pass_line, "'");
  strcat(pass_line, line_array[0]);
  strcat(pass_line, "'");
  int i = 1;

  while (i != line_len - 1)
  {
    strcat(pass_line, ",");
    strcat(pass_line, "'");
    strcat(pass_line, line_array[i]);
    strcat(pass_line, "'");
    i++;
  }
  strcat(pass_line, ",");
  char word2[] = "";
  strcat(pass_line, line_array[i]);
  pass_line[strlen(pass_line) - 1] = '\0';
  strcat(pass_line, "'");
  strcat(pass_line, "]");
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
      while (*copy == ' ' || *copy == '\t')
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
        while (*copy && (*copy != ' ' && *copy != '\t'))
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

void free_words(char **words)
{
  if (words != NULL)
  {
    char **t = words;

    while (*t)
    {
      free(*t++);
    }
    free(words);
  }
}

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

int main(int argc, char *argv[])
{
  // char *word = "hello hello hello";
  // int nwords;
  // char **line = strsplit(word, &nwords);

  FILE *ptr;

  // Read rakefile
  ptr = fopen("Rakefile", "r");
  if (NULL == ptr)
  {
    printf("file can't be opened \n");
  }
  printf("content of this file are: \n");

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
  ptr = fopen("Rakefile", "r");
  char **all_lines[numOfLines];
  int line_lengths[numOfLines];
  int i = 0;
  int defaultPort;
  int numOfHosts = 0;
  char **hostline;
  do
  {
    status1 = fgets(input2, sizeof input2, ptr);

    // Check if comment
    if (starts_with(input2, "#"))
      continue;

    // Check if empty line
    else if (strcmp(input2, "") == 10)
      continue;

    // Check if port line
    else if (starts_with(input2, "PORT"))
    {
      int nwords;
      char **portline = strsplit(input2, &nwords);
      defaultPort = atoi(portline[2]);
    }

    // Check if host line
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
  // Loop through each word from host line
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

  char server_message[2000];
  int n = 0;
  while (n < numOfLines)
  {

    char **line = all_lines[n];
    printf("start of line: %s\n", line[0]);
    if (starts_with(line[0], "actionset"))
    {
    }
    else if (starts_with(line[0], "remote-"))
    {
      printf("%s\n", line[0]);

      int num = line_lengths[n];

      char pass_line[] = "";

      printf("n: %i\n", n);
      convert_to_full_line(pass_line, line, num);
      printf("n: %i\n", n);

      if (n + 1 < numOfLines)
      {
        printf("yes\n");
        if (starts_with(all_lines[n + 1][0], "requires"))
        {
          printf("requires");
        }
      }
      send(arrayOfHosts[0], pass_line, strlen(pass_line), 0);
      recv(arrayOfHosts[0], server_message, sizeof(server_message), 0);

      // if (n + 1 < numOfLines)
      // {
      //   if (starts_with(all_lines[n + 1][0], "requires"))
      //   {
      //     printf("%s\n", all_lines[n + 1][0]);
      //   }
      // }
    }
    else
    {
    }

    n++;
  }
  // int size = sizeof lines / sizeof lines[0];
  // //printf("%i\n", numOfLines);
  // while (n < size)
  // {
  //   printf("blah ");
  //   char actionCheck[9];
  //   char tabCheck[1];
  //   char doubleTabCheck[2];
  //   char remoteCheck[7];

  //   // strncpy(actionCheck, lines[n], 9);
  // strncpy(remoteCheck, lines[n], 7);

  // printf("sending line - %s\n", lines[n]);

  // if (strcmp(actionCheck, "actionset") == 0)
  // {
  //   printf("sending action set\n");
  // }
  // else if (lines[n][0] == '\t')
  // {
  //   if (lines[n + 1][1] == '\t')
  //   {
  //     printf("DOUBLETAB\n");
  //   }
  //   else if (lines[n][1] == 'r' && lines[n][7] == '-')
  //   {
  //     printf("REMOTE\n");

  //     send(hosts_array[0], lines[n], strlen(lines[n]), 0);
  //     recv(hosts_array[0], server_message, sizeof(server_message), 0);
  //     printf("send lol - %s", lines[n]);

  //     char line[256];
  //     strcpy(line, lines[n]);
  //     char *lineString = strtok(line, "-");
  //     ;
  //     while (lineString != NULL)
  //     {
  //       // if (lineString != "remote")
  //       // {
  //       printf("%s\n", lineString);

  //   //       lineString = strtok(NULL, " ,");
  //   //     }
  //   //   }
  //   n += 1;
  // }

  // //   n += 1;
  // }
  // {
  //   printf("line: %i - %s\n", n, lines[n]);

  //   else if ()
  //   {

  //   }
  //   else
  //   {

  //   }
  //   printf("%s\n", server_message);

  // }
}