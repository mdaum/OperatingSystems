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

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int debugging = 0;
char cwd[4096]; //current working directory,
char lastPath[4096]; //previous working directory
int currJobs=0; // when inserting, assign ++currJobs....curJobs-- when popping...



struct Job{ //will add in char** later....some tricky mem managment I believe due to dynamic size
	int pid;
	int retStatus;
	char * fileName;
	char * args;
	int jobNum;
	struct Job *next;
};

struct Job *head = NULL;//list initially empty

void push(int pid,char *fName, char*args){
	struct Job *temp=malloc(sizeof(struct Job));
	temp->pid=pid;
	temp->retStatus=1;
	temp->fileName=fName;
	temp->args=args;
	temp->jobNum=++currJobs;
	temp->next=head;
	head=temp;
}

void print_job(struct Job *j){
	char* status;
		if(!j->retStatus)status="DONE\0";
		else status="IN PROGRESS\0";
		fprintf(stdout,"[%d]\t%s\t%s",j->jobNum,status,j->fileName);
		if(j->args!=NULL)fprintf(stdout," %s",j->args);
/* 		if(j->next!=NULL)fprintf(stdout,"\tmy next is %d",j->next->pid);
		else fprintf(stdout,"\tmy next is NULL"); */
		fprintf(stdout,"\n");
}

void print_job_list(){
	struct Job * curr = head;
	while(curr!=NULL){
		print_job(curr);
		curr=curr->next;
	}
}

struct Job* find(int pid){
	struct Job *curr=head;
	while(curr!=NULL){
		if(curr->pid==pid)return curr;
		curr=curr->next;
	}
	fprintf(stdout,"WHAT? How did you try to find a non-existent job?");
	return NULL; //shouldn't happen
}

struct Job* findBefore(int pid){ //finds just before pid, wont call if pid is head
	struct Job *curr=head;
	while(curr!=NULL){
		if(curr->next->pid==pid)return curr;
		curr=curr->next;
	}
	return NULL;//shouldn't happen
}

void removeJob(int pid){ //could reduce lines
	struct Job *toRemove=find(pid);
	currJobs--;
	if(toRemove==head){
		head=toRemove->next;
		toRemove->retStatus=0;
		print_job(toRemove);
		free(toRemove);
		return;
	}
	struct Job *Before=findBefore(pid);
	Before->next=toRemove->next;
	toRemove->retStatus=0;
	print_job(toRemove);
	free(toRemove);
	return;	
}

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
        } else if (argv[1] == NULL || (!strncmp(argv[1],"~",1) 
                    && strlen(argv[1]) == 1)) { //cd or cd ~
            cdStat = chdir(getenv("HOME"));
            if (cdStat < 0){
                write(1, "cd: failed to change to home directory.\n",
                        strlen("cd: failed to change to home directory.\n"));
                free(argv);
                return 0;
            }
            strncpy(lastPath, cwd, sizeof(cwd)); //save off previous cwd...
        } else if ((strncmp(argv[1], "~", 1) == 0)) { //cd ~X
            char p[4096]; 
            strcat(p, getenv("HOME"));
            strcat(p, argv[1] + sizeof(char));//append all but ~ to home path...
            cdStat = chdir(p);
            memset(p, 0, sizeof p); // because stupid strcat....
            if (cdStat < 0) {
                write(1, "cd: not a valid directory.\n",
                        strlen("cd: not a valid directory.\n"));
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
        //check command for redirects, set file descriptors as needed
        for (i = 0; commands[cursor][i] != NULL; ++i) {
            if (!strncmp(commands[cursor][i], ">", 1) && strlen(commands[cursor][i]) == 1) {
                if (debugging) {
                    write(1, "fileout: ", strlen("fileout: "));
                    write(1, commands[cursor][i+1], strlen(commands[cursor][i+1]));
                    write(1, "\n", strlen("\n"));
                }
                fileout = open(commands[cursor][i + 1], 
                        O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH);
                commands[cursor][i] = NULL;
            } else if (!strncmp(commands[cursor][i], "<", 1) && strlen(commands[cursor][i]) == 1) {
                if (debugging) {
                    write(1, "filein: ", strlen("filein: "));
                    write(1, commands[cursor][i+1], strlen(commands[cursor][i+1]));
                    write(1, "\n", strlen("\n"));
                }
                filein = open(commands[cursor][i + 1], O_RDONLY);
                commands[cursor][i] = NULL;
            } else {
                //check for redirects without spaces
                char* pos = strchr(commands[cursor][i], '>');
                char* tmpfile;
                int hasspace = 0;
                if (pos != NULL) {
                    tmpfile = pos + 1;
                    if (tmpfile[0] == '\0') {//ls> file.txt 
                        tmpfile = commands[cursor][i + 1];
                        hasspace = 1;
                    }
                    if (debugging) {
                        write(1, "fileout: ", strlen("fileout: "));
                        write(1, tmpfile, strlen(tmpfile));
                        write(1, "\n", strlen("\n"));
                    }
                    fileout = open(tmpfile, 
                            O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH);
                    commands[cursor][i][pos - commands[cursor][i]] = '\0';
                    if (hasspace) commands[cursor][i+1] = NULL;
                    if (pos - commands[cursor][i] == 0) {
                        commands[cursor][i] = NULL;
                    } else commands[cursor][i][pos - commands[cursor][i]] = '\0';
                } else if ((pos = strchr(commands[cursor][i], '<')) != NULL) {
                    //puts("wat");
                    tmpfile = pos + 1;
                    if (tmpfile[0] == '\0') { //cat file.txt< out.txt
                        tmpfile = commands[cursor][i + 1];
                        hasspace = 1;
                    }
                    if (debugging) {
                        write(1, "filein: ", strlen("filein: "));
                        write(1, tmpfile, strlen(tmpfile));
                        write(1, "\n", strlen("\n"));
                    }
                    filein = open(tmpfile, O_RDONLY);
                    if (hasspace) commands[cursor][i+1] = NULL;
                    if (pos - commands[cursor][i] == 0) {
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
                    dup2(filein, 0);
                    close(filein);
                } else if (commands[1] != NULL) { //if more than one command
                    dup2(in, 0);
                    close(in);
                }
                if (fileout != 0) { //if redirecting out
                    dup2(fileout, 1); 
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
	
	//begin testing linked list
	push(1111,"file1 weoifjowefij","args ggggggg");
	push(1112,"file2 wewfeeeeeeeeeeeeeij","args gggggaaaaaaaff");
	push(1113,"file3","args");
	push(1114,"file4","args");
	push(1115,"file5","args");
	print_job_list();
	puts("\n");
	removeJob(1111);
	print_job_list();
	puts("\n");
 	removeJob(1113);
	print_job_list();
	puts("\n");
	removeJob(1115);
	print_job_list();
	puts("\n");
	removeJob(1114);
	print_job_list();
	puts("\n");
	removeJob(1112);
	print_job_list();
	puts("\n");
	push(1112,"file2 wewfeeeeeeeeeeeeeij","args gggggaaaaaaaff");
	push(1113,"file3","args");
	print_job_list();
	puts("\n");	
	removeJob(1113);
	print_job_list();
	exit(EXIT_SUCCESS); //testing linked list for now...
	//end testing linked list...
	
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

