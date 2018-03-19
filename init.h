#ifndef __INIT
#define __INT
int init();
int para_init();
int signal_handling();
static void signal_handler(int signo);
int env_handling(int);
char buffer[BUFSIZ];
int background;
#endif
