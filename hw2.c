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
int jobs_handling(int choose);
void background_check();
int str_handling(char*,int);
int debug_handling(int);
//int scan();
//int parse();
int regular_expression_determine_init(char** execute_command, int start_pos);
int regular_expression_handling(int* pipefd, int fdout);
int post_request_handling();
int add_joblist(int pid);
int check_redirect(char**,int);
//*****************************


int prompt()
{
    //printf("shell-script$ ");
    printf("> ");
	fflush(stdout);
}

int main()
{
	init();
	int t;
    signal_handling();

    prompt();   
    while(1)
    {
		para_init();

        Input_Commands_Ptr r;
		
		r = reads();
		t = r->num;
		switch(t)
		{
			case -1: puts("error");
			case 1:        free(r);         continue;  break;
			case -255: free(r); return 0;  break;
		}
        execute(r);
        prompt();   
                //sleep(3);
    }
}


