#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MAX_FILE_NAME_LEN 256

pid_t current_proc = 0;
char *path = NULL;

void handle_int(int sig) {
	printf("Cleaning up...");
	unlink(path);
	exit(1);
}


void spawn(char *cmd) {
	if(current_proc) {
		waitpid(current_proc, NULL, 0);
	};
	current_proc = fork();
	if(!current_proc) {
		execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
	}
}

void die() {
	kill(current_proc, SIGTERM);
	printf("Encountered error %d: %s\n", errno, strerror(errno));
	exit(errno);
}

void kill_current() {
	if(!current_proc) return;
	printf("Trying to kill pid %d\n", current_proc);
	kill(current_proc, SIGTERM);
	kill(current_proc, SIGKILL);
	waitpid(current_proc, NULL, 0);
	current_proc = 0;
}

int rm_nl(char *str) { //Strip trailing newline;
	if(*str == '\0') return 1;
	char *c;
	for(c=str;*(c+1) != '\0';c++) ;;
	if(*c == '\n')
		*c = '\0';	
	return 0;
}

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s <filaname>\n", argv[0]);
		return 1;
	}

	signal(SIGINT, &handle_int);

	path = argv[1];
	unlink(path);

	if(mkfifo(path, S_IRUSR | S_IWUSR)) die();
	FILE * fp = fopen(path, "r");
	if(!fp) die();

	char *buf = malloc(sizeof(char)*MAX_FILE_NAME_LEN);
	char *line = NULL;

	while(1) {
		line=fgets(buf, MAX_FILE_NAME_LEN, fp);
		if(!line) {
			if (!errno)
				continue;
			else
				die();
		}

		rm_nl(line);

		//Never kill clear prematurely 
		if(!strcmp(line, "clear")) { 
			system("clear");
			continue;
		}

		kill_current();
		spawn(line);

		fclose(fp);
		fp = fopen(path, "r");
		if(!fp) die();
	}

	free(buf);
	return 0;
}
