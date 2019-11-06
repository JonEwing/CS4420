//////////////////////////////////////////////////////////////////////////////
//
//Jonathan Feige
//CS4420 Project 1.1
//09/25/19
//
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <stdlib.h> 
#include <string.h>


int main()
{
	int pid; 
	struct dirent *ep;   
	DIR* dp; 
	dp = opendir ("/proc");
	
	char directory[25];
	char PID [10];
	char Command [20];
	char PPID [20];
	char State [5];
	char VMSize [20];
	
	printf("PID      Command               PPID   State    VM Size (bytes)\n");
	printf("--------------------------------------------------------------\n");
	
	if (dp != NULL)     
	{       
		while (ep = readdir (dp))       
		{         
			pid = strtol(ep->d_name, NULL, 10);         
			if( ( ep->d_type == DT_DIR ) && ( pid > 0) )         
			{           
				strcpy(directory,"/proc/");
				strcat(directory, ep->d_name);
				strcat(directory, "/stat");
				
				FILE *fp = fopen(directory,"r");
				char buf[50];
				
				fscanf(fp, "%s", PID);
				fscanf(fp, "%s", Command);
				fscanf(fp, "%s", State);
				fscanf(fp, "%s", PPID);
				for(int i = 0; i < 19; i++)
				{
					fscanf(fp, "%s", buf);
				}
				fscanf(fp, "%s", VMSize);
				
				printf("%s	", &PID);
				
				char* token = strtok(Command, "(");
				token = strtok(token, ")");
				printf("%-24s", token); 

				
				printf("%-7s", &PPID);
				printf("%-9s", &State);
				printf("%s\n", &VMSize);
				
				fclose(fp);
			}
		}       
		closedir(dp);
	}
	else     
	{
		perror ("Couldn't open the directory");
	}
	
}