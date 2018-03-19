#ifndef __READS
#define __READS

char* buf;
char ** parse_char;
int parse_char_nums;
int readin_length;


typedef struct Input_commands Input_Commands;
typedef Input_Commands* Input_Commands_Ptr;
typedef Input_Commands** Input_Commands_PPtr;
typedef struct Input_commands
{
	int num;
	char** commands;
	int commands_size;
	Input_Commands_Ptr next;
	char* special_command;
}Input_Commands;

Input_Commands_Ptr reads();
#endif
