/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int debugging = 0;
char cwd[4096]; //current working directory,
char lastPath[4096]; //previous working directory

int strWhiteSpace(const char *s){
    while(*s!='\0'){
        if(!isspace(*s))return 0;
        s++;
    }
    return 1;
}

char** parsecommand(char* line) { //parses a single command
    char** argv = malloc(MAX_INPUT / 2);
    char* cmd = strtok(line, " \n\t");
    int i = 0;
    for (; cmd != NULL; i++) {
        if (!strncmp(cmd, "$", 1)) { //check if variable, replace
            argv[i] = getenv(++cmd);
        } else if (cmd[0] == '~') {
            argv[i] = strcat(strdup(getenv("HOME")), &cmd[1]);
        } else argv[i] = cmd;
        cmd = strtok(NULL, " \n\t");
    }
    argv[i] = NULL;
    return argv;
}

char*** parsepipes(char* line) { //parses piped commands
    char** lines = malloc(MAX_INPUT / 2);
    char*** commands = malloc(MAX_INPUT / 2);
    char* cmd = strtok(line, "|");
    int i;
    for (i = 0; cmd != NULL; i++) {
        lines[i] = cmd;
        cmd = strtok(NULL, "|");
    }
    lines[i] = NULL;
    for (i = 0; lines[i] != NULL; i++) //parse individual commands
        commands[i] = parsecommand(lines[i]);
    commands[i] = NULL;
    return commands;
}

int exitinternal(char** argv) { //internal exit command
    if (!strncmp(argv[0], "exit", 4) && (strlen(argv[0]) == 4))
        exit(EXIT_SUCCESS);
    return 1;
}

int setinternal(char** argv) { //internal set command
    if (!strncmp(argv[0], "set", 3) && strlen(argv[0]) == 3) {
        char* envname = strtok(argv[1], "=");
        char* envval = strtok(NULL, "=");
        setenv(envname, envval, 1);
        return 0;
    }
    return 1;
}

int cdinternal(char** argv) { //internal cd command
    if ((strncmp(argv[0],"cd",2) == 0) && (strlen(argv[0]) == 2)) {
        int cdStat;
        if (argv[2] != NULL) { //too many args
            write(1, "cd: Too many arguments.\n",
                    strlen("cd: Too many arguments.\n"));
            return 0;//no more action needed...
        } else if (argv[1] == NULL) {
            cdStat = chdir(getenv("HOME"));
            if (cdStat < 0){
                write(1, "cd: failed to change to home directory.\n",
                        strlen("cd: failed to change to home directory.\n"));
                free(argv);
                return 0;
            }
            strncpy(lastPath, cwd, sizeof(cwd)); //save off previous cwd...
        } else if ((strncmp(argv[1], "-", 1) == 0) 
                && (strlen(argv[1]) == 1)) { //cd -
            if (chdir(lastPath) < 0) {
                write(1,"cd: cd - failed.\n",
                        strlen("cd: cd - failed.\n"));
            }
            strncpy(lastPath, cwd, sizeof(cwd)); //save off previous cwd...
        } else {
            cdStat = chdir(argv[1]); //normal cd
            if (cdStat<0) {
                write(1,"cd: not a valid directory.\n",
                        strlen("cd: not a valid directory.\n"));
                return 0;
            }
            strncpy(lastPath, cwd, sizeof(cwd)); //save off previous cwd...
        }
        getcwd(cwd,4096); //update cwd...
        return 0;
    }
    return 1;
}

int runinternal(char** argv) { //check if commands are internal and run
    if (!exitinternal(argv)) return 0;
    if (!setinternal(argv)) return 0;
    if (!cdinternal(argv)) return 0;
    return 1;
}

int debugmsg(char* message, char* string) {
    write(1, message, strlen(message));
    write(1, string, strlen(string));
    write(1, "\n", strlen("\n"));
}

int runcommands(char*** commands) { //run list of piped commands
    if (commands[1] == NULL && !runinternal(commands[0])) {
        return 0; //if internal command was executed
    }
    pid_t pid;
    int child_status;
    int fd[2];
    int cursor = 0;
    int in = 0;

    while (commands[cursor] != NULL) { //run individual commands
        char* fileinpath;
        char* fileoutpath;
        int i;
        int fileout = 0;
        int filein = 0;
        int isquote = 0;
        int cmdout = 0;
        int cmdin = 1;
        //check command for redirects, set file descriptors as needed
        for (i = 0; commands[cursor][i] != NULL; ++i) {
            //check for quoted string
            if (commands[cursor][i][0] == '"') isquote = 0;
            if (commands[cursor][i][strlen(commands[cursor][i]-1)] == '"') isquote = 0;
            if (isquote) continue;
            //check for string of single < or >
            if (!strncmp(commands[cursor][i], ">", 1) && strlen(commands[cursor][i]) == 1) {
                if (debugging) debugmsg("fileout: ", commands[cursor][i+1]);
                fileout = open(commands[cursor][i + 1], 
                        O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH);
                commands[cursor][i] = NULL;
            } else if (!strncmp(commands[cursor][i], "<", 1) && strlen(commands[cursor][i]) == 1) {
                if (debugging) debugmsg("filein: ", commands[cursor][i+1]);
                filein = open(commands[cursor][i + 1], O_RDONLY);
                commands[cursor][i] = NULL;
            } else {
                //check for redirects without spaces
                char* pos = strchr(commands[cursor][i], '>');
                char* tmpfile;
                int hasspace = 0;
                if (pos != NULL) {
                    if (pos - commands[cursor][i] != 0) {
                        int tmpin = (int)strtol(commands[cursor][i], &pos, 10);
                        if (tmpin > 0) cmdin = tmpin;
                    }
                    tmpfile = pos + 1;
                    if (tmpfile[0] == '\0') {//ls> file.txt 
                        tmpfile = commands[cursor][i + 1];
                        hasspace = 1;
                    }
                    if (debugging) debugmsg("fileout: ", tmpfile);
                    fileout = open(tmpfile, 
                            O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH);
                    commands[cursor][i][pos - commands[cursor][i]] = '\0';
                    if (hasspace) commands[cursor][i+1] = NULL;
                    if (pos - commands[cursor][i] == 0 || cmdin > 0) {
                        commands[cursor][i] = NULL;
                    } else commands[cursor][i][pos - commands[cursor][i]] = '\0';
                } else if ((pos = strchr(commands[cursor][i], '<')) != NULL) {
                    if (pos - commands[cursor][i] != 0) {
                        int tmpout = (int)strtol(commands[cursor][i], &pos, 10);
                        if (tmpout > 0) cmdout = tmpout;
                    }
                    tmpfile = pos + 1;
                    if (tmpfile[0] == '\0') { //cat file.txt< out.txt
                        tmpfile = commands[cursor][i + 1];
                        hasspace = 1;
                    }
                    if (debugging) debugmsg("filein: ", tmpfile);
                    filein = open(tmpfile, O_RDONLY);
                    if (hasspace) commands[cursor][i+1] = NULL;
                    if (pos - commands[cursor][i] == 0 || cmdout > 0) {
                        commands[cursor][i] = NULL;
                    } else commands[cursor][i][pos - commands[cursor][i]] = '\0';
                }
            }
        }

        if (debugging) { //print out argv
            write(1, "argv: ", strlen("argv: "));
            for (i = 0; commands[cursor][i] != NULL; ++i) {
                write(1, commands[cursor][i], strlen(commands[cursor][i]));
                write(1, " ", strlen(" "));
            }
            write(1, "\n", strlen("\n"));
        }

        if (commands[1] != NULL) //if more than one command, set up pipe
            pipe(fd);

        if (filein < 0 || fileout < 0) {
            write(1,"Could not find file specified, or invalid args.\n",
                    strlen("Could not find file specified, or invalid args.\n"));
            exit(0);
        }

        pid = fork();
        switch(pid) {
            case -1:
                write(1,"ERROR FORKING CHILD PROCESS\n",
                        strlen("ERROR FORKING CHILD PROCESS\n"));
                exit(EXIT_FAILURE);
            case 0:
                if (filein != 0) { //if redirecting in
                    dup2(filein, cmdout);
                    close(filein);
                } else if (commands[1] != NULL) { //if more than one command
                    dup2(in, 0);
                    close(in);
                }
                if (fileout != 0) { //if redirecting out
                    dup2(fileout, cmdin); 
                    close(fileout);
                } else if (commands[cursor + 1] != NULL) //if next command not null
                    dup2(fd[1], 1);
                execvp(commands[cursor][0], commands[cursor]);
                write(1,"Could not find file specified, or invalid args.\n",
                        strlen("Could not find file specified, or invalid args.\n"));
                exit(0);
            default:
                if (debugging) {
                    char running[MAX_INPUT + sizeof("RUNNING: \n")];
                    sprintf(running, "RUNNING: %s\n", commands[cursor][0]);
                    write(1, running, strlen(running));
                }
                waitpid(pid, &child_status, 0);
                if (commands[1] != NULL) { //if more than one command
                    close(fd[1]);
                    in = fd[0];
                }
                char tmp[30];
                sprintf(tmp, "%d", child_status);
                setenv("?", tmp, 1); //set $? environment variable
                if (debugging) {
                    char ended[MAX_INPUT + sizeof("ENDED:  (ret=)\n")];
                    sprintf(ended, "ENDED: %s (ret=%d)\n", commands[cursor][0], child_status);
                    write(1, ended, strlen(ended));
                }
                free(commands[cursor]);
                cursor++;
        }
    }
    free(commands);
    return 0;
}


int main (int argc, char ** argv, char **envp) {
    char* script_name;
    int script_handle;
    int script_mode=0;
    int toRead=0; //either stdin or handle for script
    int finished = 0;
    char *prompt = "thsh> ";
    char cmd[MAX_INPUT];

    getcwd(cwd,4096); //apparently pathMax in linux

    int i;
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-d", 2) == 0 && strlen(argv[i]) == 2) {
            debugging = 1;
        }
        else{
            script_name=argv[i]; //expect any arg not -d to be script
            script_mode=1;
        }
    }
    if(script_mode){
        script_handle=open(script_name,O_RDONLY);
        if(script_handle==-1){
            puts("Could not open .sh file. Either permissions issue or file did not exist");
            exit(EXIT_FAILURE);
        }
        else {
            script_mode=1;
            toRead=script_handle;
        }
    }


    while (!finished) {
        char *cursor;
        char last_char;
        int rv;
        int count;

        // Print the prompt, but cwd first...
        if(!script_mode){
            write(1,"[",1);
            write(1,cwd,strlen(cwd));
            write(1,"] ",2);
            rv = write(1, prompt, strlen(prompt));
        }
        else rv=1;

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

            rv = read(toRead, cursor, 1);
            last_char = *cursor;
        }
        *cursor = '\0';

        if (!rv) {
            finished = 1;
            break;
        }

        // Execute the command, handling built-in commands separately
        // Just echo the command line for now
        if(strncmp(cmd,"\n",1)==0||strncmp(cmd,"#",1)==0)continue;	//if they just type in enter,or starts w comment
        char* comment= strchr(cmd,'#'); //find comments if any
        if(comment){ //trim off comments
            comment[0]='\0';
            comment++;
            memset(comment,0,strlen(comment));
        }
        if(strWhiteSpace(cmd))continue; //are we now dealing "empty" line?

        char*** commands = parsepipes(cmd);
        int status = runcommands(commands);

    }

    return 0;
}

