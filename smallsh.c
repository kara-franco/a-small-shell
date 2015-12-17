/************************************************************************************************************
 * Author: Kara Franco				    				                smallsh.c   *
 * CS 344 Operating Systems								 		    *
 * Due Date: November 23, 2015					 					    *
 *													    *
 * Program #3- This program is a small shell that will run command line instructions and return the results *
 * similar to other shells. The shell will allow for the redirection of standard input and standard output  *
 * and it will support both foreground and background processes. This small shell will support three built  *
 * in commands: exit, cd, and status.                                                                       *
 ***********************************************************************************************************/

// standard out- printf, fprintf, getline()
#include <stdio.h>
// getenv(), free(), exit()
#include <stdlib.h>
// strtok(), strcmp()
#include <string.h>
// chdir(), dup2(), fork(), exec(), close()
#include <unistd.h>
// waitpid(), signals
#include <sys/wait.h>
// pid_t, open()
#include <sys/types.h>
// open() - to be safe
#include <sys/stat.h>
// file control
#include <fcntl.h>
// exit status error numbers
#include <errno.h>
// signals
#include <signal.h>

/* -------------------------------------- global variables ----------------------------------------------*/

// use white space as delimitors (\t=tab, \r=CR, \n=newline, \a=alert)
#define TOKEN_DELIM " \t\r\n\a"
#define MAX_LENGTH 2048
#define MAX_ARGS 512
// the number of arguments including the command
int num_args;
// boolean variable, isBackground, IDs background processes
int isBackground;

/* ------------------------------------ function declarations ------------------------------------------- */

char *readLine();
char **parseLine(char *line);
int launch(char **args);

/* --------------------------------------- main function ----------------------------------------------- */

int main() {

	// main compares the args array to built in commands and redirection
	// if the args array is a not a built in command, it is passed to launch()

	// boolean variable, exitCalled, controls shell and while loop
	// when exit() is called exitCalled is set to 1 and the shell exits
	int exitCalled = 0;
	// boolean variable, exitStatus, exit success = 0, exit failure = 1
	int exitStatus = 0;

	// while the exit() command is not called, perform the commands inputed by user
	do {

		// after each command runs, reprint the : prompt
		printf(": ");
		// after each call to print, flush the output buffer
		fflush(stdout);
		// variable to store the line read from getline() in the readLine function
		char *line = NULL;
		// store the tokenized arguments from line
		char **args;
		// file descriptor used for indicating number returned from opening file
		int fd;
		num_args = 0;
		isBackground = 0;

		// read line that is entered by user
		line = readLine();
		// parse each token from line, store in args[]
		args = parseLine(line);

		// check if process will run in background (use of &), if so, set isBackground to true (1)
		if (!(strncmp(args[num_args - 1], "&", 1))) {
			isBackground = 1;
			// clear the & arguement from the end of the array
			args[num_args - 1] = NULL;
		}

		// compare arguments with built in commands
		// check for comments or blank commands
		if(args[0] == NULL || !(strncmp(args[0], "#", 1))) {
			exitStatus = 0;
		}


/* ----------------------------- Built in command: "cd"  ----------------------------------------------- */
		// source: http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
		else if(strcmp(args[0], "cd") == 0) {
			// change to directory specified in the HOME environment variable
			if(args[1]){
				// attempt to move to that directory
				if(chdir(args[1]) != 0){
					printf("No such file or directory\n");
					exitStatus = 1;
				}
				// If cd had no argument, navigate to home directory
			} else {
				chdir(getenv("HOME"));
				exitStatus = 0;
			}
		}

/* --------------------------------- Built in command: "status" ----------------------------------------*/
		// compare first element of args to the word status
		else if (strcmp(args[0], "status") == 0) {
			printf("Exit status: %d\n", exitStatus);
			exitStatus = 0;
		}


/* -------------------------------- Built in command: "exit"---------------------------------------------*/
		// compare the first element of args to the word exit
		else if(strcmp(args[0], "exit") == 0) {
			// set bool exit flag to 1 to exit loop and small shell
			exitCalled = 1;
			exitStatus = 0;
		}

		// test for input and output redirection in the command
		// compare the num of args and check the char at element 1 to determine method
		else if (num_args == 3 && ( (strcmp(args[1], ">") == 0) || (strcmp(args[1], "<") == 0) )) {
			int copy_stdout, copy_stdin;
			// copy the file descriptor for std out and std in
			copy_stdout = dup(1);
			copy_stdin = dup(0);
			if (strcmp(args[1], ">") == 0) {
				// open the file to be directed to after '>'
				fd = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0644);
				if (fd == -1) {
					printf("No such file or directory\n");
					exitStatus = 1;
				} else {
					// copy stdout to this file, delete old fd
					dup2(fd, 1);
					args[1] = NULL;
					close(fd);
					// launch command with arguments
					exitStatus = launch(args);
				}
			} else if (strcmp(args[1], "<") == 0) {
				// open file to be read from
				fd = open(args[2], O_RDONLY);
				if (fd == -1) {
					printf("No such file or directory\n");
					exitStatus = 1;
				} else {
					// redirect stdin to the specified file
					dup2(fd, 0);
					args[1] = NULL;
					close(fd);
					// launch command with arguments
					exitStatus = launch(args);
				}
			}
			// clean up stdout and stdin
			dup2(copy_stdout, 1);
			close(copy_stdout);
			dup2(copy_stdin, 0);
			close(copy_stdin);
		}
		// finally, launch any other commands
		else {
			exitStatus = launch(args);
		}
		// free memory
		free(line);
		free(args);
	} while (!exitCalled);
	return 0;
}

/* ------------------------------------------ function definitions ------------------------------------- */

/************************************************************************************************************
 * readLine() function gets the user input stream and allocates a buffer through getline() from stdio.h     *
 *  input: none						 						    *
 *  output: line read from getline                                                                          *
 ***********************************************************************************************************/

char *readLine() {
	// readLine utilizes the function getline() to allocate a buffer
	// stores the line read from getline()
	char *line = NULL;
	// getline will allocate a buffer for storing the line
	ssize_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

/************************************************************************************************************
 * parseLine() function receives the line read from readLine() (array of char) and tokenizes each argument  *
 * the user inputted so that it can be interpretted.							    *
 * input: user input read from readLine() 								    *
 * output: array of char pointers (tokens)                                                                  *
 ***********************************************************************************************************/

char **parseLine(char *line) {
	// source: http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
	// set the buffer size for each token
	int bufsize = 64, position = 0;
	// allocate memory for array of tokens
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;
	// break up the string from the line into a series of tokens, using the token delimeters to find breaks
	token = strtok(line, TOKEN_DELIM);
	// build the tokens, find the number of agruments and allocate memory
	while (token != NULL) {
		// store the current token in the array of char pointers
		tokens[position] = token;
		// increment total argument count
		num_args++;
		position++;
		// add more memory for the next string token, if at the end of the buffer
		if (position >= bufsize) {
			bufsize += 64;
			tokens = realloc(tokens, bufsize * sizeof(char*));
		}
		// repeat until no tokens are returned by strtok
		token = strtok(NULL, TOKEN_DELIM);
	}
	// null-terminate to mark the end of the array
	tokens[position] = NULL;
	return tokens;
}

/************************************************************************************************************
 * launch() function This function passes the list of arguments that were collected, fork()'s the program   *
 * and/or passes the not built in commands to exec().							    *
 * input: array of arguments								                    *
 * output: exit status                                                                                      *
 ***********************************************************************************************************/

int launch(char **args) {
	// use the pid_t data type to represent process ID and wait process ID
	pid_t pid, wpid;
	int status, exitStatus = 0;
	// fork the current process, creating a child proces, store the parent's pid
	pid = fork();
	// there are now two processes running
	// if pid == 0, this is the child process
	if (pid == 0) {
		// use execvp() to run the command, execvp() takes in a program name and an array of args
		// child process executes a new program using the passed arguments
		if (execvp(args[0], args) == -1) {
			printf("Command or file does not exist\n");
			// if execvp() returns at all there was an error
			// force an exit from this child.
			exit(1);
		}
	}
	// if pid < 0, there was an error in forking
	else if (pid < 0) {
		perror("smallsh");
	}
	// else, the fork executed and this is the parent
	else {
		// parent process waits for the child process specified with wpid
		do {
			if (isBackground == 0)
				// use macro WUNTRACED (from waitpid()) to report any child that is exited or terminated
				wpid = waitpid(pid, &status, WUNTRACED);

			// if the process is in the background use macro WNOHANG to tell parent not to wait
			// there is no child process ready to be noticed
			else if (isBackground == 1)
				wpid = waitpid(-1, &status, WNOHANG);

			// continue waiting for the child to exited or killed
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	if (isBackground == 1)
		// display background process ID
		printf("Background PID: %d\nExit status: %d", pid, exitStatus);

	// use WIFSIGNALED macro to show value if the child process terminated from a signal
	// status should be 0, to indicate success
	if (status != 0 || WIFSIGNALED(status))
		// in grading script the process is killed with pkill- SIGTERM (signal 15)
		exitStatus = 1;

	return exitStatus;
}
