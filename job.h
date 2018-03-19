#ifndef __JOB
#define __JOB
typedef struct JOB Job;
typedef struct JOB
{
	int pid;
	char** commands; 
}Job;


int size;
int jobs_size;

int jobs_num;

Job** jobs;
int* jobs_occupied_index;//???
#endif
