// Jonathan Feige
// CS4420_Ptrace_1
// 11/04/19

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <x86_64-linux-gnu/asm/unistd_64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int long_size = sizeof(long);
void getdata(pid_t child, long addr, char *str, int len)
{
	char *laddr;
	int i, j;
  	union u {
    	long val;
    	char chars[long_size];
  	} data;
  	i = 0;
  	j = len / long_size;
  	laddr = str;
  	while(i < j) {
    	data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
    	memcpy(laddr, data.chars, long_size);
    	++i;
    	laddr += long_size;
  	}
  	j = len % long_size;
  	if (j != 0) {
    	data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
    	memcpy(laddr, data.chars, j);
  	}
  	str[len] = '\0';
}

struct fd {
   int fd_read;
   int fd_write;
};

struct fd fds[64000];

int read_write(int fd, int read, int write)
{
	fds[fd].fd_read += read;
	fds[fd].fd_write += write;
}


int command_index=0;
char * command[100];

int main()
{
	for(int i = 0; i < 64000; i++)
	{
		fds[i].fd_read = 0;
		fds[i].fd_write = 0;
	}
	
	int i = 0;
    char line[1000];
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
	
		pid_t child;
    	long orig_rax, rax;
    	unsigned long params[3];
    	int status;
    	int insyscall = 0;
    	child = fork();

		FILE *fp_stdout;
		FILE *fp_stderr;
		FILE *fp_strace;
   	 	if(child == 0) {
        	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        	execvp(command[0], command);
    	}
    	else {


			char snum[10000];

			sprintf(snum, "%d", child);
			strcat(snum,"_stdout.txt");
			fp_stdout = fopen(snum, "w");

			sprintf(snum, "%d", child);
			strcat(snum,"_stderr.txt");
			fp_stderr = fopen(snum, "w");

			sprintf(snum, "%d", child);
			strcat(snum,"_strace.txt");
			fp_strace = fopen(snum, "w");

			while(1) {
				wait(&status);
				if(WIFEXITED(status))
					break;
				orig_rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);

				/*Write*/
				if(orig_rax == __NR_write) {
					if(insyscall == 0) {
						insyscall = 1;
						params[0] = ptrace(PTRACE_PEEKUSER,child, 8 * RDI,NULL);
						params[1] = ptrace(PTRACE_PEEKUSER,child, 8 * RSI,NULL);
						params[2] = ptrace(PTRACE_PEEKUSER,child, 8 * RDX,NULL);
					
						char str[20];
						getdata(child, params[1], str, 20);
						fprintf(fp_strace,"Write (%ld , %s , %ld) --> ",params[0], str,params[2]);
						if(params[0] == 1)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stdout,"WRITE: %s\n", str);
						}
						if(params[0] == 2)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stderr,"WRITE: %s\n", str);
						}
						read_write(params[0],0,params[2]);
					}
					else {
						rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);
                    	fprintf(fp_strace,"%ld\n\n", rax);
                    	insyscall = 0;
                	}
            	}
			
				/*Read*/
				else if(orig_rax == __NR_read){
					if(insyscall == 0) {
						insyscall = 1;
						params[0] = ptrace(PTRACE_PEEKUSER,child, 8 * RDI,NULL);
						params[1] = ptrace(PTRACE_PEEKUSER,child, 8 * RSI,NULL);
						params[2] = ptrace(PTRACE_PEEKUSER,child, 8 * RDX,NULL);
						
						char str[20];
						getdata(child, params[1], str, 20);
						fprintf(fp_strace,"Read (%ld , %s , %ld) --> ",params[0], str,params[2]);
						if(params[0] == 1)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stdout,"READ: %s\n", str);
						}
						if(params[0] == 2)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stderr,"READ: %s\n", str);
						}
						read_write(params[0],params[2],0);
					}
					else {
						rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);
                  	  	fprintf(fp_strace,"%ld\n\n", rax);
                   	 	insyscall = 0;
                	}
				}
			
				/*Open*/
				else if(orig_rax == __NR_open)
					if(insyscall == 0) {
						insyscall = 1;
						params[0] = ptrace(PTRACE_PEEKUSER,child, 8 * RDI,NULL);
						params[1] = ptrace(PTRACE_PEEKUSER,child, 8 * RSI,NULL);
					
						char str[20];
						getdata(child, params[0], str, 20);					
						fprintf(fp_strace,"Open (%s , %ld) --> ",str, params[1]);
						if(params[0] == 1)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stdout,"OPEN: %s\n", str);
						}
						if(params[0] == 2)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stderr,"OPEN: %s\n", str);
						}
					}
					else {
						rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);
                    	fprintf(fp_strace,"%ld\n\n", rax);
                   	 	insyscall = 0;
                	}
			
				/*Openat*/
				else if(orig_rax == __NR_openat)
					if(insyscall == 0) {
						insyscall = 1;
						params[0] = ptrace(PTRACE_PEEKUSER,child, 8 * RDI,NULL);
						params[1] = ptrace(PTRACE_PEEKUSER,child, 8 * RSI,NULL);
						params[2] = ptrace(PTRACE_PEEKUSER,child, 8 * RSI,NULL);
					
						char str[20];
						getdata(child, params[1], str, 20);					
						fprintf(fp_strace,"Openat (%ld, %s , %ld) --> ",params[0], str, params[2]);
						if(params[0] == 1)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stdout,"OPENAT: %s\n", str);
						}
						if(params[0] == 2)
						{
							char str[1000];
							getdata(child, params[1], str, 100);
							fprintf(fp_stderr,"OPENAT: %s\n", str);
						}
					}
					else {
						rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);
                   	 	fprintf(fp_strace,"%ld\n\n", rax);
                    	insyscall = 0;
                	}
				
				/*Close*/
				else if(orig_rax == __NR_close)
					if(insyscall == 0) {
						insyscall = 1;
						params[0] = ptrace(PTRACE_PEEKUSER,child, 8 * RDI,NULL);
								   
						fprintf(fp_strace,"Close (%ld) --> ",params[0]);
						if(params[0] == 1)
						{
							fprintf(fp_stdout,"CLOSE: %ld\n", params[0]);
						}
						if(params[0] == 2)
						{
							fprintf(fp_stderr,"CLOSE: %ld\n", params[0]);
						}
					}
					else {
						rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);
                    	fprintf(fp_strace,"%ld\n\n", rax);
                    	insyscall = 0;
                	}
			
				/*Exit*/			
				else if(orig_rax == __NR_exit || orig_rax == __NR_exit_group)
					if(insyscall == 0) {
						insyscall = 1;
						params[0] = ptrace(PTRACE_PEEKUSER,child, 8 * RDI,NULL);
								   
						fprintf(fp_strace,"Exit (%ld)\n\n",params[0]);
						if(params[0] == 1)
						{
							fprintf(fp_stdout,"EXIT: %ld\n", params[0]);
						}
						if(params[0] == 2)
						{
							fprintf(fp_stderr,"EXIT: %ld\n", params[0]);
						}
					}
					else {
                    	insyscall = 0;
                	}
				
            	ptrace(PTRACE_SYSCALL,child, NULL, NULL);
        	}	
    	}
	
		fprintf(fp_strace,"\n");
		for(int i =0; i < 64000; i++)
		{
			if((fds[i].fd_read != 0) || (fds[i].fd_write != 0))
			{
				fprintf(fp_strace,"file descriptor: %-5d, read volume: %-10d bytes,	write volume: %-10d bytes\n",i,fds[i].fd_read,fds[i].fd_write);
			}
		}
		fclose(fp_strace);
		fclose(fp_stdout);
		fclose(fp_stderr);

		printf("\n\n");
	}

    return 0;
}