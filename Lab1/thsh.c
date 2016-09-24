/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int runcommand(char* line){
    char** argv = malloc(MAX_INPUT / 2);
    char* cmd = strtok(line, " \n\t()<>|&;");
    int i;
    for (i = 0; cmd != NULL; i++) {
        argv[i] = cmd;
        cmd = strtok(NULL, " \n\t()<>|&;");
    }

    argv[i] = NULL;
    char* file = argv[0];

    if((strncmp(file,"exit",4)==0)&&(strlen(file)==4)){ 
        //if cmd is exit, we successfully exit
        exit(EXIT_SUCCESS);
    }
	
	//begin cd block
	if((strncmp(file,"cd",2)==0)&&(strlen(file)==2)){
		if(argv[2]!=NULL){ //too many args
			write(1,"cd: Too many arguments.\n",strlen("cd: Too many arguments.\n")); free(argv); return 0;//no more action needed...
		}
		int cdStat;
		if(argv[1]==NULL||(strncmp(argv[1],"~",1)==0)&&strlen(argv[1])==1){ //cd or cd ~
			cdStat=chdir(getenv("HOME"));
			if(cdStat<0){
				write(1,"cd: failed to change to home directory.\n",strlen("cd: failed to change to home directory.\n"));
				free(argv);
				return 0;
			}
			free(argv);
			return 1;
		}
		if((strncmp(argv[1],"-",1)==0)&&(strlen(argv[1])==1)){ //cd -
			free(argv);
			return 2; //special status
		}
		cdStat=chdir(argv[1]); //normal cd
		if(cdStat<0){
			write(1,"cd: not a valid directory.\n",strlen("cd: not a valid directory.\n"));
			free(argv);
			return 0;
		}
		free(argv);
		return 1;//success
	}
	//end cd block
	
    int pid=fork();
    int child_Status;
    if(pid<0){//error forking child
        write(1,"ERROR FORKING CHILD PROCESS",strlen("ERROR FORKING CHILD PROCESS"));
        exit(EXIT_FAILURE);
    }
    else if(pid==0){//child runs execvp...dont forget to check if this fails this...
        if (execvp(file,argv) == -1) {
            write(1,"Could not find file specified, or invalid args.\n",
            strlen("Could not find file specified, or invalid args.\n"));
			exit(0);
  }
    }
    else{//parent waiting....
        child_Status= wait(&pid);
    }
    free(argv);
    return 0;
}


int
main (int argc, char ** argv, char **envp) {

    int finished = 0;
    char *prompt = "thsh> ";
    char cmd[MAX_INPUT];
	char cwd[4096];//current working directory,
	getcwd(cwd,4096); //apparently pathMax in linux
	char lastPath[4096];//represents last path..
	
    while (!finished) {
        char *cursor;
        char last_char;
        int rv;
        int count;


        // Print the prompt, but cwd first...
		write(1,"[",1);
		write(1,cwd,strlen(cwd));
		write(1,"] ",2);
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

        // Execute the command, handling built-in commands separately
        // Just echo the command line for now
		if(strncmp(cmd,"\n",1)==0)continue;	//if they just type in enter
        int status = runcommand(cmd); //read status of runcommand....we can handle special commands in main space too now
		
		if(status==1){//cd (already changed dir) so now update cwd;
			strncpy(lastPath,cwd,sizeof(cwd)); //save off previous cwd...
			getcwd(cwd,4096); //update cwd...
		}
		
		if(status==2){//cd - special case....
			if(chdir(lastPath)<0)write(1,"cd: cd - failed.\n",strlen("cd: cd - failed.\n"));
			strncpy(lastPath,cwd,sizeof(cwd)); //save off previous cwd...
			getcwd(cwd,4096); //update cwd...
		}
		

    }

    return 0;
}


