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
#include"reads.h"
#include"debug.h"


Input_Commands_Ptr return_struct(char**parse_char, int parse_char_nums);
void print_commands(Input_Commands_Ptr A);
    
Input_Commands_Ptr first_para_check(char* tmpc)
{
	Input_Commands_Ptr Result = (Input_Commands_Ptr) malloc(sizeof(Input_Commands));
	Result->commands_size = 0;
	if(!strcmp(tmpc,"unset"))
	{	
		//env_handling(2);
		Result->num = 1;
		return (Result);
	}
	else if(!strcmp(tmpc, "export"))
	{
		//env_handling(1);
		Result->num = 1;
		return (Result);
	}
	else if(!strcmp(tmpc, "DEBUG_ON"))
	{
		//debug_handling(1);
		Result->num = 1;
		return (Result);
	}
	else if(!strcmp(tmpc, "DEBUG_OFF"))
	{
		//debug_handling(0);
		Result->num = 1;
		return (Result);
	}
	else if(!strcmp(tmpc, "exit"))
	{
		Result->num = -255;
		return (Result);
	}
	else if(!strcmp(tmpc, "jobs"))
	{
		Result->num = 1;
		return (Result);
		//jobs_handling(0);
	}
	else if(!strcmp(tmpc, "fg"))
	{
		//jobs_handling(1);
		Result->num = 1;
		return (Result);
	}
	else if(!strcmp(tmpc, "bg"))
	{
		//jobs_handling(-1);
		Result->num = 1;
		return (Result);
	}
	Result->num = 0;
	return (Result);
	
}


Input_Commands_Ptr reads()
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
                    Input_Commands_Ptr A = first_para_check(tmpc);		
                    if(A->num != 0)
                    {
                        free(tmpc);
                        free(parse_char);
                        return A;
                    }	
                    free(A);
                }
	
                readin++;
		if(readin>=size)
		{
			parse_size += size;
			parse_char = (char**) realloc(parse_char, parse_size*sizeof(char*));
		}
		

                switch(tmpc[0])
                {
/*
                        case '\'':
                        case '\"':
                        case '(':
                        case '[':
                        case '{':   if(str_handling(tmpc, readin-1) == -1){ return -1;}; break;
*/
                        default :   parse_char[readin-1] = (char*) malloc((strlen(tmpc)+1)*sizeof(char));
			
                                    strcpy(parse_char[readin-1], tmpc); break;
                }

                if(getchar() == '\n'){break;}
        }
        free(tmpc);

        parse_char[readin] = 0;
        parse_char_nums = readin;

        if(parse_char[parse_char_nums-1][0] == '&')
        {
            free(parse_char[parse_char_nums - 1]);
            parse_char_nums -= 1;
            background = 1;
        }

        return return_struct(parse_char, parse_char_nums);
}

Input_Commands_Ptr return_struct(char**parse_char, int parse_char_nums)
{
/*
    //no input
	if(parse_char_nums == 0)
	{
		Input_Commands_Ptr Result = (Input_Commands_Ptr) malloc(sizeof(Input_Commands));
		Result->commands_size = 0;
		Result->num = 1;
		return (Result);
	}
*/
	int cur_pos = 0;
	Input_Commands_Ptr A =  (Input_Commands_Ptr) malloc(sizeof(Input_Commands));
	Input_Commands_Ptr  Result = A; 
    A->num = 0;
	for(int i = 0; i < parse_char_nums ; i++)
	{
		if( !strcmp( parse_char[i], "|" ) )
		{
			A->commands = (char**) malloc( sizeof(char*)*(i-cur_pos+1) );
			A->commands_size = i - cur_pos + 1;
            A->commands[A->commands_size-1] = NULL;

			for(int j = 0; j < i - cur_pos; j++)
			{
				(A->commands)[j] = (char*) malloc( (strlen(parse_char[j + cur_pos])+1)*sizeof(char));
				strcpy( (A->commands)[j], parse_char[j + cur_pos] ); 
                free(parse_char[j + cur_pos]);
			}
            free(parse_char[i]);

			A->next = (Input_Commands_Ptr) malloc(sizeof(Input_Commands));
			A->special_command = (char*) malloc(sizeof(char)*1);
			A->special_command[0] = '|';

			A = A->next;
			cur_pos = i + 1;
		}
	}
	if(cur_pos < parse_char_nums)
	{
			//A = (Input_Commands_Ptr) malloc(sizeof(Input_Commands));
			A->commands = (char**) malloc(sizeof(char*)*(parse_char_nums - cur_pos + 1));
			A->commands_size = parse_char_nums - cur_pos + 1;
            A->commands[A->commands_size-1] = NULL;

			for(int j = 0; j < parse_char_nums - cur_pos; j++)
			{
				(A->commands)[j] = (char*) malloc( (strlen(parse_char[j+cur_pos])+1)*sizeof(char));
				strcpy( (A->commands)[j], parse_char[j + cur_pos]); 
                free(parse_char[j + cur_pos]);
			}
			A->next = NULL;
			A->special_command = (char*) malloc(sizeof(char)*1);
			A->special_command[0] = '\0';
	}
    free(parse_char);
    
if(DEBUG_ON)
{
    A = Result;
    print_commands( A);
}
    return Result;
}

void print_commands(Input_Commands_Ptr A)
{
    do
    {
        printf("A->commands = ");
        for(int i = 0; i < A->commands_size; i++)
        {
            printf("%s ", A->commands[i]);
        }
        puts("");
        A = A->next;
    }while(A);
}
