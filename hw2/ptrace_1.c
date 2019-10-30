/* The follwing program is based on an example 
   from the Linux Journal Article "Playing with ptrace, Part 1
   It was modified to run on a modern x86_64 processor (like pu1, pu2, pu3, etc.) 
*/

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <x86_64-linux-gnu/asm/unistd_64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
__NR_write is the number of the write() system call
  see x86_64-linux-gnu/asm/unistd_64.h
*/

int main(int argc, char **argv)
{   
	FILE *fptr;
	fptr = fopen(argv[1],"w");
	
	char * exe[100];
	strcpy(exe, "/bin/");
	strcat(exe, argv[2]);
	
	char command[100];
	for (int i = 2; i < argc; ++i)
	{
		strcat(command, argv[i]);
		strcat(command, " ");		
	}

	pid_t child;
    long orig_rax, rax;
    unsigned long params[3];
    int status;
    int insyscall = 0;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls -l", command, NULL);
    }
    else {
       while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_rax = ptrace(PTRACE_PEEKUSER,
                     child, 8 * ORIG_RAX, NULL);

          if(orig_rax == __NR_write) {
             if(insyscall == 0) {
                /* Syscall entry */
                insyscall = 1;
                params[0] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * RDI,
                                   NULL);
                params[1] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * RSI,
                                   NULL);
                params[2] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * RDX,
                                   NULL);
				fprintf(fptr,"Write called with %lx, %lx , %lx\n",params[0], params[1],params[2]);
          }
          else { /* Syscall exit */
                rax = ptrace(PTRACE_PEEKUSER,
                             child, 8 * ORIG_RAX, NULL);
                    fprintf(fptr,"Write returned with %ld\n", rax);
                    insyscall = 0;
                }
            }
            ptrace(PTRACE_SYSCALL,
                   child, NULL, NULL);
        }
    }
	
	fclose(fptr);
    return 0;
}
