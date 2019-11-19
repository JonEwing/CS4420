#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <x86_64-linux-gnu/asm/unistd_64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int command_index=0;
char * command[100];

int  main()
{
    int i = 0;
    char line[1000];
	char str[1000];

    while ((fgets(line, sizeof line, stdin) != NULL) && (line[0] != '\n'))
    {
		if(line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 2] = '\0';
		}
		if(line[strlen(line) - 2] == '\n' || line[strlen(line) - 2] == '\r')
		{
			line[strlen(line) - 2] = '\0';
		}
		
		char* token = strtok(line, " "); 
	
		while (token != NULL) 
		{ 
			command[i] = token;
			token = strtok(NULL, " "); 
			i++;
		} 
  
		
		for(int j = 0; j < i; j++)
		{
			printf("%s ", command[j]); 
		}
		printf("\n");
		i = 0;
    }

    return 0;
}