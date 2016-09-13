/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

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


    // Execute the command, handling built-in commands separately
    // Just echo the command line for now
	if((strncmp(cmd,"exit",4)==0)&&(strlen(cmd)==5)){ //if cmd is exit, we successfully exit
		exit(EXIT_SUCCESS);
	}
	
    write(1, cmd, strnlen(cmd, MAX_INPUT));
	
  }

  return 0;
}
