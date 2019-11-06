#include <stdio.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <stdlib.h> 
#include <string.h>

typedef struct
{
	int PID;
	int PPID;
	char Command [20];
} stat;

int max_x = 0;
int max_y = 0;

void max(int x,int y)
{
	if(max_x < x)
	{
		max_x = x;
	}
	if(max_y < y)
	{
		max_y = y;
	}
}

int size = 1000;
stat tree[1000][1000];

void make_tree(stat tmp) 
{ 
    for(int i = 0; i < size - 3; i++)
	{
		for(int j = 1; j < size-1; j++)
		{
			if(tree[i][j - 1].PID == tmp.PPID)
			{
				if(tree[i][j].PID != -1)
				{
					while(tree[i][j].PID != -1)
					{
						i++;
					}
					for (int k = 0; k <	size; k++)
					{		
							tree[i + 1][k] = tree[i][k];
							tree[i][k].PID = -1;
					}
					tree[i][j] = tmp;
					max(i+1,j);
					return;
				}
				else
				{
					tree[i][j] = tmp;
					max(i,j);
					return;
				}
			}
		}
	}
} 

int main()
{
	int pid; 
	struct dirent *ep;   
	DIR* dp; 
	dp = opendir ("/proc");
	
	char directory[25];
	char buf[25];
	stat stat_array[100];
	
	strcpy (stat_array[0].Command,"(dummy)");
	stat_array[0].PID = 0;
	stat_array[0].PID = 0;
	
	int stat_amount = 1;
	
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
				int i;
				
				fscanf(fp, "%s", buf);
				sscanf(buf, "%d", &i);
				stat_array[stat_amount].PID = i;
				
				fscanf(fp, "%s", stat_array[stat_amount].Command);
				fscanf(fp, "%s", buf);
				
				fscanf(fp, "%s", buf);
				sscanf(buf, "%d", &i);
				stat_array[stat_amount].PPID = i;
				
				fclose(fp);
	
				stat_amount++;

			}
		}       
		closedir(dp);
	}
	else     
	{
		perror ("Couldn't open the directory");
	}
	
	for(int i = 0; i < size; i++)
		for(int j = 0; j < size; j++)
		{
			tree[i][j].PID = -1;
		}
			
		tree[0][0] = stat_array[0];
		
	
	for(int i = 1; i < stat_amount; i++)
		{
			make_tree(stat_array[i]);
		}
		
	for(int i = 0; i < max_x + 1; i++)
	{
		for(int j = 0; j < max_y; j++)
		{
			if(tree[i][j].PID != -1)
			{
				if(i > 0 && ( tree[i - 1][j].PPID == tree[i][j].PPID))
				{
					printf("|-->%5d ", tree[i][j].PID);
					printf("%-10s ", &tree[i][j].Command);
				}
				else
				{
					printf("--->%5d ", tree[i][j].PID);
					printf("%-10s ", &tree[i][j].Command);
				}
			}
			else
			{
				printf("                     ");
			}
		}	
		printf("\n");
	}
}