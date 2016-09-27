/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int debugging = 0;
char cwd[4096];//current working directory,
char lastPath[4096];

char** parsecommand(char* line) {
    char** argv = malloc(MAX_INPUT / 2);
    char* cmd = strtok(line, " \n\t()<>|&;");
    int i;
    for (i = 0; cmd != NULL; i++) {
        if (!strncmp(cmd, "$", 1)) {
            argv[i] = getenv(++cmd);
        } else {
            argv[i] = cmd;
        }
        cmd = strtok(NULL, " \n\t()<>|&;");
    }

    argv[i] = NULL;
    return argv;
}

int exitinternal(char** argv) {
    if((strncmp(argv[0],"exit",4)==0)&&(strlen(argv[0])==4)){ 
        exit(EXIT_SUCCESS);
    }
    return 1;
}

int setinternal(char** argv) {
    if (!strncmp(argv[0], "set", 3) && strlen(argv[0]) == 3) {
        char* envname = strtok(argv[1], "=");
        char* envval = strtok(NULL, "=");
        setenv(envname, envval, 1);
        return 0;
    }
    return 1;
}

int cdinternal(char** argv) {
    if((strncmp(argv[0],"cd",2)==0)&&(strlen(argv[0])==2)){
        int cdStat;
        if(argv[2]!=NULL){ //too many args
            write(1,"cd: Too many arguments.\n",strlen("cd: Too many arguments.\n"));
            return 0;//no more action needed...
        } else if((argv[1]==NULL||(strncmp(argv[1],"~",1))==0)&&strlen(argv[1])==1){ //cd or cd ~
            cdStat=chdir(getenv("HOME"));
            if(cdStat<0){
                write(1,"cd: failed to change to home directory.\n",strlen("cd: failed to change to home directory.\n"));
                free(argv);
                return 0;
            }
            strncpy(lastPath,cwd,sizeof(cwd)); //save off previous cwd...
        } else if((strncmp(argv[1],"~",1)==0)){ //cd ~X
            char p[4096]; 
            strcat(p,getenv("HOME"));
            strcat(p,argv[1]+sizeof(char));//append all but ~ to home path...
            cdStat=chdir(p);
            memset(p,0,sizeof p); // because stupid strcat....
            if(cdStat<0){
                write(1,"cd: not a valid directory.\n",strlen("cd: not a valid directory.\n"));
                free(argv);
                return 0;
            }
            strncpy(lastPath,cwd,sizeof(cwd)); //save off previous cwd...
        } else if((strncmp(argv[1],"-",1)==0)&&(strlen(argv[1])==1)){ //cd -
            if(chdir(lastPath)<0)write(1,"cd: cd - failed.\n",strlen("cd: cd - failed.\n"));
            strncpy(lastPath,cwd,sizeof(cwd)); //save off previous cwd...
        } else {
            cdStat=chdir(argv[1]); //normal cd
            if(cdStat<0){
                write(1,"cd: not a valid directory.\n",strlen("cd: not a valid directory.\n"));
                return 0;
            }
        }

        getcwd(cwd,4096); //update cwd...
        return 0;
    }
    return 1;
}

int runinternal(char** argv) {
    if (!exitinternal(argv)) return 0;
    if (!setinternal(argv)) return 0;
    if (!cdinternal(argv)) return 0;
    return 1;
}

int runbinary(char** argv){
    if (!runinternal(argv)) return 0;

    int pid=fork();
    int child_status;
    if(pid<0){//error forking child
        write(1,"ERROR FORKING CHILD PROCESS\n",strlen("ERROR FORKING CHILD PROCESS\n"));
        exit(EXIT_FAILURE);
    }
    else if(pid==0){//child runs execvp...dont forget to check if this fails this...
        if (debugging) {
            char running[MAX_INPUT + sizeof("RUNNING: \n")];
            sprintf(running, "RUNNING: %s\n", argv[0]);
            write(1, running, strlen(running));
        }
        if (execvp(argv[0],argv) == -1) {
            write(1,"Could not find file specified, or invalid args.\n",
                    strlen("Could not find file specified, or invalid args.\n"));
            exit(0);
        }
    }
    else{//parent waiting....
        waitpid(pid, &child_status, 0);
        char tmp[30];
        sprintf(tmp, "%d", child_status);
        setenv("?", tmp, 1);
        if (debugging) {
            char ended[MAX_INPUT + sizeof("ENDED:  (ret=)\n")];
            sprintf(ended, "ENDED: %s (ret=%d)\n", argv[0], child_status);
            write(1, ended, strlen(ended));
        }
    }
    //set $? environment variable
    free(argv);
    return 0;
}


int main (int argc, char ** argv, char **envp) {

    int finished = 0;
    char *prompt = "thsh> ";
    char cmd[MAX_INPUT];

    getcwd(cwd,4096); //apparently pathMax in linux

    int i;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debugging = 1;
        }
    }

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
        int status = runbinary(parsecommand(cmd)); //read status of runcommand....we can handle special commands in main space too now

    }

    return 0;
}

