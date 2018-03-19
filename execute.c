
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<regex.h>
#include"job.h"
#include"init.h"	
#include"debug.h"
#include"reads.h"
#include"execute.h"

void get_message(int fd);
void execute_commands(int* final_pipefd, Input_Commands_Ptr Com);
void execute_command(char** execute_command, int len);
void last_command(char** execute_command, int len);
void complete_return(int* final_pipefd);
int check_redirect(char** parse_strs, int length);


int execute(Input_Commands_Ptr Com)
{
	int final_pipefd[2];	
	pipe(final_pipefd);

    int status;
	
	pid_t pid = fork();
	if(pid == 0)
	{
                setpgid(0, 0);
                int pgid = getpid();

		pid_t pid2 = fork();
		if(pid2 == 0)
		{	
			execute_commands(final_pipefd, Com);
			
			exit(0);
		}

        if(!background)
        {
//int opgrp = tcgetpgrp(STDOUT_FILENO);
//signal(SIGTTOU,SIG_IGN);
//tcsetpgrp(STDOUT_FILENO,pid2);
//tcsetpgrp(STDIN_FILENO,pid2);
            wait(NULL);
//tcsetpgrp(STDOUT_FILENO,opgrp);
//tcsetpgrp(STDIN_FILENO,opgrp);
        }
        //int wpid = waitpid(pid2, &status, WUNTRACED);
		exit(0);
	}
int opgrp = tcgetpgrp(STDOUT_FILENO);
signal(SIGTTOU,SIG_IGN);
tcsetpgrp(STDOUT_FILENO,pid);
tcsetpgrp(STDIN_FILENO,pid);

	close(final_pipefd[1]);
	//dup2(final_pipefd[0], 0);
	wait(&status);

tcsetpgrp(STDOUT_FILENO,opgrp);
tcsetpgrp(STDIN_FILENO,opgrp);
    //int wpid = waitpid(pid, &status, WUNTRACED);
	printf("\n");
    
    if(!background)
    {
        get_message(final_pipefd[0]);
    }

puts("4.1:here to free");
while(Com){
    Input_Commands_Ptr T;
    for(int i = 0; i < Com->commands_size-1; i++)
    {
printf("%s \n",Com->commands[i]);
        free(Com->commands[i]);
    }
    free(Com->commands);
    free(Com->special_command); 
    T = Com->next;
    free(Com); 
    Com =  T;
}
puts("4.2:end free");

        return 1;
}

void get_message(int fd)
{
	FILE* fp = fdopen(fd,"r");
	char buffer[256];
	while(fgets(buffer, 256, fp) != NULL)
	{
			printf("%s",buffer);
	}	

}

void execute_commands(int* final_pipefd, Input_Commands_Ptr Com)
{
	//char* command1[3] = {"ls", "-al", NULL};
	//char* command2[2] = {"cat",  NULL};
    while(Com->next)
    {
	    execute_command(Com->commands, Com->commands_size);
        Com = Com->next;
    }
	
	last_command(Com->commands, Com->commands_size);
    //complete_return(final_pipefd);
}

void execute_command(char** execute_command, int len)
{
	int pipefd[2];
	pipe(pipefd);

	pid_t pid2 = fork();
	if(pid2 == 0)
	{	

		close(pipefd[0]);
		dup2(pipefd[1], 1);
check_redirect(execute_command, len -1);


		execvp(execute_command[0], execute_command);

		exit(0);
	}

	close(pipefd[1]);
	dup2(pipefd[0], 0);

}

void last_command(char** execute_command, int len)
{
	    dup2(STDOUT_FILENO, 1);
        check_redirect(execute_command, len -1);

		execvp(execute_command[0], execute_command);

		exit(0);
}

void complete_return(int* final_pipefd)
{


/*
	char buffer[256];
	FILE* fp = fdopen(STDIN_FILENO,"r");
	FILE* fpout = fdopen(final_pipefd[1],"w");
    FILE* fprout = fdopen(STDOUT_FILENO,"w");

	while( fgets(buffer, 256, fp) != NULL)
	{
        if(background)
        {
            fprintf(fprout, "%s",buffer);
        }
        else
        {
            //fprintf(fpout, "%s",buffer);
fprintf(fprout, "%s",buffer);
        }
	}
	close(final_pipefd[0]);
*/
}

int check_redirect(char** parse_strs, int length)
{
        int i;
        for(i = 0; i < length; i++)
        {
                if(!strcmp(parse_strs[i], "<"))
                {
                        int fd = open(parse_strs[i+1], O_RDONLY);

                        length-=2;
                        int j;
                        for(j = i;j<=length;j++)
                        {
                                parse_strs[j] = parse_strs[j+2];
                        }
                        i--;
                        parse_strs[length+1] = NULL;
                        parse_strs[length+2] = NULL;
                        dup2(fd, 0);

                        close(fd);


                }
                else if(!strcmp(parse_strs[i], ">"))
                {
                        int fd = open(parse_strs[i+1], O_RDWR| O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                        length-=2;
                        int j;
                        for(j = i;j<=length;j++)
                        {
                                parse_strs[j] = parse_strs[j+2];
                        }
                        i--;
                        parse_strs[length+1] = NULL;
                        parse_strs[length+2] = NULL;
                        dup2(fd, 1);

                        close(fd);
                }
        }

        return 0;
}
