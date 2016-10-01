# README #

Written by Max Daum and James Barbour

### Files Included ###
* thsh.c (our shell)
* thsh executable
* makefile
* fun (script)

### Ex 1 ###

* Chose to use execvp flavor of exec
* Runs through method (runcommand)

### Piping and Redirection ###

* used a triple char pointer to hold multiple "argv" to pass into multiple execvp calls
*Added extra layer of pipe parsing (parse pipes)

### Scripts ###
* We re-use the same code for reading stdin in script execution
* comment stripping is done before parsing pipes, and completely stripped lines are treated as skipped command

### Job Control ###
*Used a linked list of structs representing jobs.
*If job being run in background we do a preliminary fork in runcommands() in order to have a "parent" of all potential execvp calls made in job.
*Used a currpid global var to avoid suspending or terminating shell, and to also determine whether user owns foreground or not