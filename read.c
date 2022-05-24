#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strsplit.h"
#define LSIZ 128 
#define RSIZ 10 

int main(void) 
{
    char line[RSIZ][LSIZ];
	char fname[20];
    FILE *fptr = NULL; 
    int i = 0;
    int tot = 0;
    printf("\n\n Read the file and store the lines into an array :\n");
	printf("------------------------------------------------------\n"); 
	printf(" Input the filename to be opened : ");

        void getarray(int n){
        int init_size = strlen(line[n]);
        char delim[] = " ";

        char *ptr = strtok(line[n], delim);
        char destination[] = "[ ";
        char end[] = "]";
        char apo[] = "\"";
        char comma[] = ",";
        char source[] = " , ";
        while(ptr != NULL)
        {
            printf("'%s'\n", ptr);
            strcat(destination,apo);

            strcat(destination,ptr);
            strcat(destination,apo);
            strcat(destination,comma);
            
            
            //strcat(destination,source);
            ptr = strtok(NULL, delim);
            
        }
        destination[strlen(destination)-1] = '\0';
        strcat(destination,end);
        printf(" %s\n", destination);
    }	
    
    fptr = fopen("Rakefile", "r");
    while(fgets(line[i], LSIZ, fptr)) 
	{
        line[i][strlen(line[i]) - 1] = '\0';
        i++;
    }
    tot = i;
    getarray(4);
    getarray(5);
	printf("\n The content of the file %s  are : \n",fname);    
    for(i = 0; i < tot; ++i)
    {
        char *ret;
        const char *tmp = line[i];
        ret = strstr(tmp, "\t");
        if (ret)
            getarray(i);
            
        
        
    }

    
}

