
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
#include"execute.h"


static void signal_handler(int signo)
{
        FILE* fp = fopen("/dev/tty", "w");
        setbuf(fp, NULL);
        switch(signo)
        {
                case SIGINT:     /*fprintf(fp, "receive ^C \n"); break;*/
                case SIGQUIT:   /* fprintf(fp, "receive ^Backspace \n"); break;*/
                                //fprintf(fp, "  \nshell_script$ ");
                                fprintf(fp, "  \n> ");
                                break;
        }
        setbuf (fp, buffer ); // setbuf(file, NULL) 會取消緩衝區，每次都直接輸出入。
        fclose (fp);
}

int signal_handling()
{
        signal(SIGINT, signal_handler);
        signal(SIGQUIT, signal_handler);
        return 0;
}

int init()
{
	int i;

	size = 10;
	jobs_size = size;

	jobs = (Job**) malloc(jobs_size * sizeof(Job*));
	jobs_occupied_index = (int*) calloc(jobs_size, sizeof(int));
	jobs[0] = NULL;
}

int para_init()
{
	background = 0;
	DEBUG_ON = 0;
	regular_expression_found = 0;
}

extern char** environ;
int env_handling(int choose)
{

}
