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

int init();
int signal_handling();
int para_init();
int env_handling(int);
int jobs_handling(int choose);
int reads();
void background_check();
int str_handling(char*,int);
int debug_handling(int);
//int scan();
//int parse();
int execute();
int regular_expression_determine_init(char** execute_command, int start_pos);
int regular_expression_handling(int* pipefd, int fdout);
int post_request_handling();
int add_joblist(int pid);
int check_redirect(char**,int);
//*****************************

int DEBUG_ON = 0;

const int size = 256;

typedef struct JOB
{
	int pid;
	char** commands; 
}Job;
int jobs_num = 0;
int jobs_size;
Job** jobs;
int* jobs_occupied_index;
//****************************
int init()
{
	int i;
	jobs_size = size;
	jobs = (Job**) malloc(jobs_size * sizeof(Job*));
	jobs_occupied_index = (int*) calloc(jobs_size, sizeof(int));
	jobs[0] = NULL;
}

int main()
{
	init();
        signal_handling();
        while(1)
        {
		para_init();

                char* s;
                int r;
                printf("shell-script$ ");
        //      scanf("%s", s);

                r = reads();
                switch(r)
                {
                        case -1: puts("error");
                        case 1:                 continue; break;
			case -255: return 0; break;
                }
//              scan();
//              parse();
              execute();
                //sleep(3);
        }
}

char buffer[BUFSIZ];
static void signal_handler(int signo)
{
        FILE* fp = fopen("/dev/tty", "w");
        setbuf(fp, NULL);
        switch(signo)
        {
                case SIGINT:     /*fprintf(fp, "receive ^C \n"); break;*/
                case SIGQUIT:   /* fprintf(fp, "receive ^Backspace \n"); break;*/
                                fprintf(fp, "  \nshell_script$ ");
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

int background = 1;
int para_init()
{
	background = 0;
}

extern char** environ;
int env_handling(int choose)
{
	int i,j;
	char c;
	if(choose == 1)
	{
		//puts("env0");
		     if(getchar() == '\n'){
		//puts("env1");
				for( i = 0; environ[i] != NULL; i++)
				{
		//puts("env2");
					printf("declare -x ", environ[i]);
					for(int j = 0; environ[i][j] != '\0'; j++)
					{
		//puts("env3");
						c = environ[i][j];
						printf("%c", c);
						if(c == '='){ putchar('\"');}
					}
		//puts("env4");
					puts("\"");
				}
				return 1;
		     }
		
//puts("env5");
	     int size_c = size, readin_c = 0;
	     char *temp_c = (char*) malloc(size_c * sizeof(char));
	     int equal_sign_found = 0, has_double_quote = 0, alpha_start = 0;
	     while((c = getchar()) != '\n')
	     {
		if(isblank(c)  && (alpha_start == 0)){/*printf("dis_c: %c\n", c);*/continue;}
		else{alpha_start = 1;}
//puts("env6");
		readin_c++;
		if(isblank(c) && (alpha_start == 1)){ fflush(stdin); readin_c--; while((c = getchar())!= '\n'){} break;}
		if(readin_c+2 >= size_c)
		{
			size_c += (size+2);
			temp_c = (char*) realloc(temp_c, size_c * sizeof(char));
		}

		if(equal_sign_found && (c == '\"'))
		{
			readin_c--;
			has_double_quote = 1;
			continue;
		}
		else
		{
			temp_c[readin_c-1] = c;
//printf("readin_c %d, c = %c\n",readin_c, c);
		}
		if(c == '='){ equal_sign_found = 1;}
		else{ equal_sign_found = 0;}
	     }

	     if(has_double_quote)
		{
			temp_c[readin_c-1] = '\0';
		}
	     else
	     {
			temp_c[readin_c] = '\0';
	     }
//printf("readin_c: %d, temp_c:1 %s\n",readin_c, temp_c );
	     putenv(temp_c);
		return 0;
     }
     else if(choose  == 2)
     {
	     int size_c = size, readin_c = 0;
	     char *temp_c = (char*) malloc(size_c * sizeof(char));
	     int isblank = 1;
//puts("unset1");
	     while((c = getchar()) != '\n')
	     {
			if(isblank(c) && (isblank==0))
			{
				isblank = 1;
				temp_c[readin_c] = '\0';
//printf("readin_c: %d, temp_c:1 %s\n",readin_c, temp_c );
			        putenv(temp_c);
				readin_c = 0;
				continue;	
			}
			else if(isblank(c))
			{
				continue;
			}
			readin_c++;
			if(readin_c+2 >= size)
			{
				size_c += size+2;
				temp_c = (char*) realloc(temp_c, size_c * sizeof(char));
			}	     
			
			temp_c[readin_c-1] = c;
			isblank = 0;
	     }
	if(isblank==0)
	{
		temp_c[readin_c] = '\0';
//printf("readin_c: %d, temp_c:1 %s\n",readin_c, temp_c );
		putenv(temp_c);
	}
	return 2;
     } 
     
}

int print_job_status(int job_order, Job* J, int status)
{
	int j;

        pid_t wpid = waitpid(J->pid, &status, WNOHANG|WUNTRACED);
	printf("[%d] ", job_order);
	if(wpid < 0)
	{
		printf(" waitpid error. ");
	}
	else if(wpid == 0)
	{
		printf(" running ");
	}
	else if(wpid == J->pid)
	{
		if (WIFEXITED(status))
		{
			printf(" ended normally ");
			jobs_occupied_index[job_order] = 0;
			free(J);
		}
		else if (WIFSIGNALED(status))
		{
			printf(" ended because of an uncaught signal ");
			jobs_occupied_index[job_order] = 0;
			free(J);
		}
		else if (WIFSTOPPED(status))
		{
			printf(" stopped ");
		}
		//exit(EXIT_SUCCESS);
	}
	printf(" pid = %d  \t",J->pid);
	for(j = 0; (J->commands)[j] != NULL; j++)
	{
		printf("%s ", (J->commands)[j]);
	}
	puts("");
	return 0;
}

int jobs_handling(int choose)
{
	int i, j, job_order, status, wpid;
	Job* J;
	if(!choose)
	{
		for(i = 0; i < jobs_num; i++)
		{
			if(jobs_occupied_index[i])
			{
				J = jobs[i];
				print_job_status(i, J, status);
			}
		}	
	}
	else if(choose == 1)
	{
		scanf("%d", &job_order);
		if(jobs_occupied_index[job_order] == 0)
		{
			printf("non exist.\n");
			return -1;
		}
			J = jobs[job_order];
			print_job_status(job_order, J, status);
                	wpid = waitpid(J->pid, &status, WUNTRACED);
		
	}
	else if(choose == -1)
	{
		char buffer_j[1024], c_con[128];
		char* pEnd = buffer_j;
		int k = 0, index = 0;;
		long l;
		fgets(buffer_j,256,stdin);
if(DEBUG_ON)
{
printf("buffer_j: %s\n", buffer_j);
}
		while(1)
		{
			while(pEnd[k] != '\0')
			{
				if(isdigit(pEnd[k])){break;}
				k++;
			}
			if(pEnd[k] == '\0')
			{
				break;
			}
			while(isdigit(pEnd[k]))
			{
				c_con[index] = pEnd[k];
				index++;
				k++;
			}
			c_con[index] = '\0';
			index = 0;
			//scanf("%d", &job_order);
			job_order = (int) atoi(c_con);
if(DEBUG_ON)
{
printf("job_order= %d \n",job_order);
}

			if(jobs_occupied_index[job_order] == 0)
			{
				printf("non exist.\n");
				return -1;
			}

			J = jobs[job_order];
			kill(J->pid, SIGCONT);		

		}
	}
	return 0;
}

char* buf;
char ** parse_char;
int parse_char_nums;
int readin_length = 0;


int reads()
{
        char *tmpc = (char*) malloc(sizeof(char)*256);
        int size = 256;
        int parse_size = size;
        parse_char = (char**) malloc(sizeof(char*) *size);
        int readin = 0;
//puts("1");
        while(scanf("%s", tmpc) == 1)
        {
                if((readin == 0) )
                {
			if(!strcmp(tmpc,"unset"))
			{	
                        	env_handling(2);
                        	return 1;
			}
			else if(!strcmp(tmpc, "export"))
			{
                        	env_handling(1);
                        	return 1;
			}
			else if(!strcmp(tmpc, "DEBUG_ON"))
			{
				debug_handling(1);
                        	return 1;
			}
			else if(!strcmp(tmpc, "DEBUG_OFF"))
			{
				debug_handling(0);
                        	return 1;
			}
			else if(!strcmp(tmpc, "exit"))
			{
                        	return -255;
			}
			else if(!strcmp(tmpc, "jobs"))
			{
				jobs_handling(0);
                        	return 1;
			}
			else if(!strcmp(tmpc, "fg"))
			{
				jobs_handling(1);
                        	return 1;
			}
			else if(!strcmp(tmpc, "bg"))
			{
				jobs_handling(-1);
                        	return 1;
			}
			
                }
	
//puts("1.1");
                readin++;
                if(readin>=size)
                {
                        parse_size += size;
                        parse_char = (char**) realloc(parse_char, parse_size*sizeof(char*));
                }

                switch(tmpc[0])
                {
                        case '\'':
                        case '\"':
                        case '(':
                        case '[':
                        case '{':   if(str_handling(tmpc, readin-1) == -1){ return -1;}; break;
                        default :   parse_char[readin-1] = (char*) malloc(sizeof(tmpc));
                                    strcpy(parse_char[readin-1], tmpc); break;
                }

//puts("1.2");
/*
                parse_char[readin-2] = (char*) malloc(sizeof(tmpc));
                strcpy(parse_char[readin-2], tmpc);
*/
//printf("readin-2 = %d, %s\n", readin-1, parse_char[readin-1]);
//puts("1.3");
                if(getchar() == '\n'){break;}
        }
        parse_char[readin] = 0;
        parse_char_nums = readin;
//printf("pre_parse_char_nums: %d \n",parse_char_nums);
	background_check();
        return 0;
}

void background_check()
{
	if(!strcmp(parse_char[parse_char_nums-1],  "&"))
	{
		background = 1;
		parse_char_nums--;
		parse_char[parse_char_nums] = 0;	
	}
}

int str_handling(char* str, int index)
{
                int special_char_num = 5;
                int pre_backslash = 0 , quote[special_char_num], quote_choose = -1;
                int i;
                for(i = 0; i < special_char_num; i++)
                {
                        quote[i] = 0;
                }
                int size_char = strlen(str), malloc_size = size_char;
                if(size > malloc_size)
                {
                        malloc_size = size;
                }
                char* newchar = (char*) malloc(2*malloc_size*sizeof(char));
                strcpy(newchar, str);

                char cplus, cminus;
                switch(str[0])
                {
                                case '\'': cplus = '\''; cminus = '\''; quote_choose = 0; break;
                                case '\"': cplus = '\"'; cminus = '\"'; quote_choose = 1; break;
                                case '(': cplus = '('; cminus = ')'; quote_choose = 2; break;
                                case '[': cplus = '['; cminus = ']'; quote_choose = 3; break;
                                case '{': cplus = '{'; cminus = '}'; quote_choose = 4; break;
                }

                for(i = 0; i <  size_char; i++)
                {
                        if(str[i] == cplus)
                        {
                                quote[quote_choose] += 1;
                        }
                        else if(str[i] == cminus)
                        {
                                quote[quote_choose] -= 1;
                        }
                }

                char c;
                while(quote[quote_choose] != 0)
                {
                        if((c = getchar()) == '\n')
                        {
                                return -1;
                        }


                        if(c == cplus)
                        {
                                quote[quote_choose] += 1;
                        }
                        else if(c == cminus)
                        {
                                quote[quote_choose] -= 1;
                        }

                        newchar[size_char] = c;
                        size_char++;
                }
                parse_char[index] = newchar;
        return 0;
}

int debug_handling(int choose)
{
	DEBUG_ON = choose;
	return 0;
}

//char*AA[]={"ls","-al", 0};
int regular_expression_found = 0;
char* re_temp_c;
int execute()
{
        pid_t pid, wpid;
        int status;
        int pipeline_on = 0;

	int origin_pipefd[2], origin_pipe_on = 0;
	if(origin_pipe_on)
	{
		pipe(origin_pipefd);
	}
//setpgid(0, 0);
        pid = fork();
        if(pid == 0)
        {
                setpgid(0, 0);
                int pgid = getpid(), be_first_leader = 0;

//execvp("ls", AA);
                int start_pos = 0;
                char ** execute_command =  (char**) malloc(sizeof(char*) *size);
        /*
                if(readin>=size)
                {
                        parse_size += size;
                        parse_char = (char**) realloc(parse_char, parse_size*sizeof(char*));
                }
*/
//puts("4.0");
                //in = 0;
                int i, j;
                for(i = 0; i < parse_char_nums; i ++)
                {
//puts("4.01");

                        if( (i + 1 +1 - start_pos) >= size)
                        {
                                execute_command = (char**) realloc(execute_command, 2*(i+1-start_pos)*sizeof(char*));
                        //      execute_command[i - start_pos] = NULL;
                        }
//puts("4.02");
printf("parse_char_nums = %d, parse_char[i] = %s\n",parse_char_nums, parse_char[i]);

                        if(!strcmp(parse_char[i], "|"))
                        {
puts("4.03");
                                execute_command[i - start_pos] = NULL;
                                /*
                                if(!pipeline_on)
                                {
                                        execute_command[i - start_pos] = NULL;
                                }
                                else
                                {
                                        execute_command[i - start_pos] = pipefd[0];
                                        execute_command[i - start_pos] = NULL;
                                }
                                */
                                /*
                                for(j = 0; j < (i - start_pos); j++)
                                {

                                }
                                */

if(DEBUG_ON)
{
for(j = 0; j <= i - start_pos; j++)
{
        printf("%s  @@ ", execute_command[j]);
}
puts("");
}
				int len = i - start_pos;
				regular_expression_determine_init(execute_command, len);

                                int pipefd[2], temp_new_pipefd[2],  rpid, group_leader;

                                pipe(pipefd);
                                pipe(temp_new_pipefd);
                                if(be_first_leader)
                                {
                                        group_leader = 1;
                                        be_first_leader = 0;
                                }
                                else
                                {
                                        group_leader = 0;
                                }
                                if((rpid = fork()) == 0)
                                {
                                        if(group_leader)
                                        {
                                                //pgid = getpid();
                                                setpgid(0, 0);
                                                /*
                                                if(fork() == 0)
                                                {
                                                        //execlp("ls", "ls","-al", "/etc/passwd", 0);
                                                        sleep(3);
                                                }
                                                else{
                                                        exit(0);
                                                }
                                                */
                                        }
                                        else
                                        {
if(DEBUG_ON)
{
printf("rpid = %d, pgid = %d\n",getpid(),  pgid);
}
                                                setpgid(rpid, pgid);
                                        }

                                	//close(pipefd[0]);
						//close(pipefd[1]);
						close(temp_new_pipefd[0]);
                                        dup2(temp_new_pipefd[1], 1);
                                        //dup2(pipefd[1], 1);
					if(regular_expression_found)
					{
						check_redirect(execute_command, i - start_pos -1);
						//check_redirect(execute_command, i - start_pos);
					}
					else
					{
						check_redirect(execute_command, i - start_pos);
					}
                                        //execvp(parse_char[0], parse_char);
                                        execvp(execute_command[0], execute_command);
					exit(EXIT_FAILURE);
                                }
                                else if (rpid > 0)
                                {
                                        if(group_leader)
                                        {
                                                pgid = rpid;
                                                setpgid(0,pgid);
                                                //group_leader = 0;
                                        }
                                }
                                //close(pipefd[1]);
                dup2(temp_new_pipefd[0], 0);
					close(temp_new_pipefd[1]);
        			wpid = waitpid(rpid, &status, WUNTRACED);
if(DEBUG_ON)
{
		if (WIFEXITED(status))
		{
			printf(" ended normally \n");
		}
		else if (WIFSIGNALED(status))
		{
			printf(" ended because of an uncaught signal \n");
		}
		else if (WIFSTOPPED(status))
		{
			printf(" stopped \n");
		}
}
                                
		regular_expression_handling(temp_new_pipefd, pipefd[1]);
               dup2(pipefd[0], 0);
				//regular_expression_handling(temp_new_pipefd, STDOUT_FILENO);
				//in = pipefd[0];
                                //wpid = waitpid(rpid, &status, WUNTRACED);

                                pipeline_on = 1;
                                start_pos = i +1;
                        }
                        else
                        {
//puts("4.1");
	
                                execute_command[i - start_pos] = parse_char[i];
                        }
                }
                execute_command[parse_char_nums - start_pos] = NULL;


if(DEBUG_ON)
{
printf("start_pos = %d\n", start_pos);
for(j = 0; j <= i - start_pos; j++)
{
        printf("%s  @!@ ", execute_command[j]);
}
puts("");
}
		int len = parse_char_nums - start_pos;
		regular_expression_determine_init(execute_command, len);
/*
    if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
        printf("Could not set PGID.n");
        return(EXIT_FAILURE);
    }
*/	

        int lpid, pipefd[2];
	pipe(pipefd);
                if((lpid = fork()) == 0)
                //wpid = waitpid(pid, &status, WUNTRACED);
                {

                        if(be_first_leader)
                        {
                                pgid = getpid();
                                setpgid(0, 0);
                        }
                        else
                        {
                                setpgid(lpid, pgid);
                        }
		
	close(pipefd[0]);
	dup2(pipefd[1], 1);
	/*
                int fore_fd = open("/dev/tty", O_RDWR);
                //int fore_fd = open("/dev/tty", O_WRONLY);
               // dup2(fore_fd, STDIN_FILENO);
                 dup2(fore_fd, STDOUT_FILENO);
               // dup2(fore_fd, STDERR_FILENO);
                close(fore_fd);
	*/		
			if(regular_expression_found)
			{
                        	check_redirect(execute_command, parse_char_nums - start_pos -1);
			}
			else
			{
                        	check_redirect(execute_command, parse_char_nums - start_pos);
			}
                        //execvp(parse_char[0], parse_char);
                        execvp(execute_command[0], execute_command);
			exit(EXIT_FAILURE);

         }

	close(pipefd[1]);
	dup2(pipefd[0], 0);

        wpid = waitpid(lpid, &status, WUNTRACED);
if(DEBUG_ON)
{
		if (WIFEXITED(status))
		{
			printf(" ended normally \n");
		}
		else if (WIFSIGNALED(status))
		{
			printf(" ended because of an uncaught signal \n");
		}
		else if (WIFSTOPPED(status))
		{
			printf(" stopped \n");
		}
}
	if(origin_pipe_on)
	{
		close(origin_pipefd[0]);
		dup2(origin_pipefd[1], 1);
		regular_expression_handling(pipefd, origin_pipefd[1]);
	}
	else
	{
		regular_expression_handling(pipefd, STDOUT_FILENO);
	}

	regular_expression_found = 0;



	exit(0);
        }
        else if(pid > 0)
        {
		if(background == 0)
		{
				if(origin_pipe_on)
				{
						close(origin_pipefd[1]);
						dup2(origin_pipefd[0], 0);

				}

                	wpid = waitpid(pid, &status, WUNTRACED);
			//execlp("less","less","/etc/passwd",0);
			//
				if(origin_pipe_on)
				{
					int buffer_size = 65535;
					char buffer[buffer_size], parsed_buffer[buffer_size];
					FILE* fp = fdopen(origin_pipefd[0],"r");
					//while(read(pipefd[0], buffer, buffer_size-1))
					while(fgets(buffer, buffer_size, fp) != NULL)
					{
							buffer[buffer_size-1] = '\0';
							printf("%s",buffer);
					}	
				}

		}
		else if(background ==1)
		{
			add_joblist(pid);//++++++++++++++++++origin_pipefd[0]
		}
        }
        return 1;
}

int regular_expression_determine_init(char** execute_command, int len)
{
	regular_expression_found  = 0;
	re_temp_c = execute_command[len -1];
if(DEBUG_ON & 0)
{
printf("regu2, re_temp_c = %s, len = %d\n",re_temp_c, len);
int j;
for(j = 0; j <= len; j++)
{
        printf("%s  @@ ", execute_command[j]);
}
puts("");
}
	for(int i =0; re_temp_c[i] != '\0' ; i++)
	{
		if( (re_temp_c[i] == '*') || (re_temp_c[i] == '?') ) 
		{
			regular_expression_found = 1;
			re_temp_c = malloc(strlen(re_temp_c)*sizeof(char));
			strcpy(re_temp_c, execute_command[len -1]);
			execute_command[len -1] = 0;
			break;
		}
	}
if(DEBUG_ON & 0)
{
	printf("\n@regular parse_char_nums = %d, len = %d \n", parse_char_nums, len);
}
	return 0;
}

int regular_expression_handling(int* pipefd, int fdout)
{
	regex_t regex;
	int reti;
	//char msgbuf[100];
	char* re_post_char;
	/* Compile regular expression */
	if(regular_expression_found)
	{
	if(DEBUG_ON)
	{
	printf("re_temp_c = %s\n", re_temp_c);
	}
	int re_post_size = size;
	int re_post_index = 2;
	if(re_post_size < 7){re_post_size = 7;}
	re_post_char = (char*) malloc(re_post_size * sizeof(char));
	re_post_char[0] = '\\';
	re_post_char[1] = 'b';
	for(int i = 0; re_temp_c[i] != '\0'; i++)
	{	
		re_post_index++;
		if(re_post_index +4 >= re_post_size)
		{
			re_post_size += (4+size);
			re_post_char = (char*) realloc(re_post_char,re_post_size * sizeof(char));
		}	

		switch(re_temp_c[i])
		{
			case '*':
			case '?':   re_post_char[re_post_index-1]='.';	re_post_index++; break;
			case '.':   re_post_char[re_post_index-1]='\\';	re_post_index++; break;
		}

		re_post_char[re_post_index-1] = re_temp_c[i];
	}
	re_post_char[re_post_index++] = '\\';
	re_post_char[re_post_index++] = 'b';
	re_post_char[re_post_index++] = '\0';
	free(re_temp_c);
	if(DEBUG_ON)
	{
	printf("re_post_char = %s\n", re_post_char);
	}
		reti = regcomp(&regex, re_post_char , REG_EXTENDED);
		if (reti) {
		    fprintf(stderr, "Could not compile regex\n");
		    exit(1);
		}
	}

	int buffer_size = 65535;
	char buffer[buffer_size], parsed_buffer[buffer_size];
	FILE* fp = fdopen(pipefd[0],"r");
if(fp ==NULL){printf("fp is NULL\n");};
	FILE* fpout = fdopen(fdout,"w");
 FILE* fpstdout = fopen("/dev/tty", "w");
 	char* info;
//puts("re 45");
	//while(read(pipefd[0], buffer, buffer_size-1))
	while( (info=fgets(buffer, buffer_size, fp)) != NULL)
	{
//puts("re 46");
		if(regular_expression_found)
		{
			reti = regexec(&regex,buffer, 0, NULL, 0);
			if(!reti)
			{
				if(DEBUG_ON)
				{
					buffer[buffer_size-1] = '\0';
					fprintf(fpout,"reti = %d, buffer= %s",reti,buffer);
					//fprintf(fpstdout,"$$#reti = %d, buffer= %s",reti,buffer);
				}
				else
				{
					buffer[buffer_size-1] = '\0';
					fprintf(fpout, "%s",buffer);
				}
			}		
		}
		else
		{
			buffer[buffer_size-1] = '\0';
			fprintf(fpout, "%s",buffer);
			//fprintf(fpstdout,"$$#reti = %d, buffer= %s",reti,buffer);
		}
	}	
	fclose(fp);
	fclose(fpout);
	fclose(fpstdout);
//puts("re 49");
	return 0;
}

int post_request_handling()
{

}

int add_joblist(int pid)
{

	int i;
	if( jobs_num+1 >= jobs_size)
	{
		int pre_size = jobs_size;
		jobs_size += size;
		jobs = (Job**) realloc(jobs, jobs_size * sizeof(Job*) );
		jobs_occupied_index = (int*) realloc(jobs_occupied_index, jobs_size * sizeof(int));
		for(i = pre_size; i < jobs_size; i++)
		{
			jobs_occupied_index[i] = 0;
		}
	}

	Job* j;
	int jobs_pick_index; 
        for(int i = 0; i <= jobs_num; i++)
	{
		if(!jobs_occupied_index[i])
		{
			jobs_pick_index = i;
			jobs_occupied_index[i] = 1;	
			break;
		}
	}	

	jobs[jobs_pick_index] = (Job*) malloc( sizeof(Job) );
	j = jobs[jobs_pick_index];
	j->pid = pid;
	j->commands = (char**) malloc(parse_char_nums* sizeof(char) );
	for(i = 0; i < parse_char_nums; i++)
	{
		(j->commands)[i]  = (char*) malloc(strlen(parse_char[i]) * sizeof(char)+1);
        	strcpy( (j->commands)[i], parse_char[i]);
if(DEBUG_ON)
{
printf("parse_char[i] = %s",parse_char[i]);	
}
	}
if(DEBUG_ON)
{
puts("");
}
	(j->commands)[i] = NULL;
	if(jobs_pick_index == jobs_num)
	{
		jobs_num++;
	}
if(DEBUG_ON)
{
for(int i = 0; i < parse_char_nums; i++)
{
	printf("%s @j ", (j->commands)[i]);
}
puts("");
}
	return 0;
}

int check_redirect(char** parse_strs, int length)
{
        int i;
        for(i = 0; i < length; i++)
        {
//printf("i=%d\n", i);
                if(!strcmp(parse_strs[i], "<"))
                {
                        int fd = open(parse_strs[i+1], O_RDONLY);

                        //free(parse_strs[i]);
                        //free(parse_strs[i+1]);
                        length-=2;
                        for(;i<=length;i++)
                        {
                                parse_strs[i] = parse_strs[i+2];
                        }
                        parse_strs[length+1] = NULL;
                        parse_strs[length+2] = NULL;
//printf("fd = %d\n", fd);
                        dup2(fd, 0);

                        close(fd);


                }
                else if(!strcmp(parse_strs[i], ">"))
                {
                        int fd = open(parse_strs[i+1], O_RDWR| O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//printf("parse_strs[i] = %s, parse_strs[i+1] = %s\n", parse_strs[i], parse_strs[i+1]);
                        //free(parse_strs[i]);
                        // free(parse_strs[i+1]);
                        length-=2;
                        for(;i<=length;i++)
                        {
                                parse_strs[i] = parse_strs[i+2];
                        }
                        parse_strs[length+1] = NULL;
                        parse_strs[length+2] = NULL;
//printf("fd = %d\n", fd);
                        dup2(fd, 1);

                        close(fd);
//puts("3.6");
                }
//puts("3.7");
//printf("%s@@ ", parse_strs[i]);
        }

//puts("3.8");

//printf("post_length: %s\n", length);
//puts("3.9");
/*
for(i = 0; i <= length; i++)
{
        printf( "%s@@ \n", parse_strs[i]);
}
*/
        return 0;
}

