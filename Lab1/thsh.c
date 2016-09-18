/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

char** get_args(char* line) {
    if (line[0] == '\n') {
        return NULL;
    }
    char** args = malloc(MAX_INPUT / 2);
    char* cmd = strndup(line, (int)(strchr(line, ' ') - line));
    cmd = strtok(line, " ");
    int i;
    
    for (i = 0; cmd != NULL; i++) {
        args[i] = cmd;
        cmd = strtok(NULL, " ");
    }

    args[i] = NULL;
	
    return args;
}

int runcommand(char* file,char** args){
	int pid=fork();
	int child_Status;
	if(pid<0){//error forking child
		write(1,"ERROR FORKING CHILD PROCESS",strlen("ERROR FORKING CHILD PROCESS"));
		exit(EXIT_FAILURE);
	}
	else if(pid==0){//child runs execvp...dont forget to check if this fails this...
		int exec_status = execvp(file,args);
		printf("%d",exec_status);
		write(1,"Could not find file specified, or invalid args.",strlen("Could not find file specified, or invalid args."));
	}
	else{//parent waiting....
		child_Status=wait(pid);
		write(1,"successfull execvp",strlen("successfull execvp"));
	}
}


int
main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "thsh> ";
  char cmd[MAX_INPUT];


  while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;


    // Print the prompt
    rv = write(1, prompt, strlen(prompt));
    if (!rv) {
      finished = 1;
      break;
    }

    // read and parse the input
    for(rv = 1, count = 0,
	  cursor = cmd, last_char = 1;
	rv
	  && (++count < (MAX_INPUT-1))
	  && (last_char != '\n');
	cursor++) {

      rv = read(0, cursor, 1);
      last_char = *cursor;
    }
    *cursor = '\0';

    if (!rv) {
      finished = 1;
      break;
    }

	char** args=get_args(cmd);
    // Execute the command, handling built-in commands separately
    // Just echo the command line for now
	if(((strncmp(args[0],"exit\n",5)==0)&&(strlen(args[0])==5))||
	((strncmp(args[0],"exit",4)==0)&&(strlen(args[0])==4))){ //if cmd is exit, we successfully exit
		exit(EXIT_SUCCESS);
	}
    else{ //runcommand
		runcommand(args[0],args+1);
	}
	
  }

  return 0;
}


