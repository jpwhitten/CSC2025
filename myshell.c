/*
 * Joseph Whitten - 140197916
 */

//including some libraies
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>

//Defining constants, including the delimiter and standard output colours
#define DELIM " \t\r\n\a"
#define NORMAL_COLOUR "\x1B[0m"
#define ERROR_COLOUR "\x1B[31m"
#define IMPORTANT_COLOUR "\x1B[36m"
#define SUCCES_COLOUR "\x1B[32m"

//Defining the boolean type used in functions later
typedef enum {
	false = 0, 
	true = 1
} bool;

//Declaring some global variables 
char *prompt;					          //Stores the current prompt
size_t functionNumber; 			 		  //Stores the number of built in commands
char *currentWorkingDirectory; 	          //Stores the current working directory
bool showCurrentWorkingDirectory = false; //Stores a boolean value dictating wether or not to show the current working directory
int usercolor = 0;						  //Interger for text color formatting


//Forward declearation of functions for a top down structure
void loop(void);
char* readLine(void);
char** splitLine(char *line);
int execute(char **arguments);
int launchProgram(char** arguments);

//Forward declearation of the functions for the built in commands, needed for the following struct
int exitfunc(char **arguments);
int cdfunc(char **arguments);
int helpfunc(char **arguments);
int promptfunc(char **arguments);
int wdfunc(char **arguments);
int colourChange(char **arguments);

void getwokdir(void) ;

//Structure for my built in commands, all char values are constant
struct builtinfunc {
	//Stores the commands name (what the user needs to input to run the command)
	const char *name;				
	//Stores a brief description of the prompt (formatted for the help command)
	const char *functionDescription;
	//Pointer to the function that controls the commands behavior
	int (*functionPointer) (char**);	 
} commands[] = {
  {"q", "Exit the shell", &exitfunc},
  {"cd", "Change the current working directory to the first argument \n\t\t after this command", &cdfunc},
  {"help", "Open help information", &helpfunc},
  {"prompt", "Change your prompt to what you type after this command", &promptfunc},
  {"wd", "Toggle displaying your current working directory", &wdfunc},
  {"color", "Change the text colour *values from 31 - 37", &colourChange}
};

char *colours[] = {
	"Red",
	"Green",
	"Brown",
	"Blue",
	"Purple",
	"Cyan",
	"White"
};

//main function
int main() {

	//Welcome message
	printf(SUCCES_COLOUR "\n Welcome to Joseph Whitten's Shell!\n");
	printf(NORMAL_COLOUR "\n");

	//Start the shell loop
	loop();

	//Exit when loop ends
	return EXIT_SUCCESS;
}

//Loop the shell
void loop(void) {

	//Set the initial Prompt
	prompt = "> ";
	//allocate space for the current working directory
	currentWorkingDirectory = malloc(PATH_MAX + 1);
	//Calculate and store how many functions are built in
	functionNumber = sizeof(commands) / sizeof(struct builtinfunc);

	//Declare needed variables
	char *line;
	char **arguments;
	int status;

	do {
		//Show current working directory?
		if(showCurrentWorkingDirectory) {
			getwokdir();
			printf("\x1B[%dm [%s] ", usercolor, currentWorkingDirectory);
		}
		//print the prompt
		printf("\x1B[%dm %s ", usercolor, prompt);
		line = readLine();
		arguments = splitLine(line);
		status = execute(arguments);

		//free memory taken by line and arguments
		free(line);
		free(arguments);

		//loop while status is not 0
	} while(status) ;
}

//Read in a line from Standard input
char *readLine(void) {
	//Declare all the variables needed for the getLine function
	char *buffer = NULL;
	size_t len;
	//Store the line in read
	getline(&buffer, &len, stdin);
	//return line
	return buffer;
} 

//Parse the line that has been read and split it into strings (tokens) using the delimeter defined abhove
char **splitLine(char *line) {

	//Initialise intergeres to keep track of the current index and the amount of space we have allocated for tokens
 	int index = 0, tokenMemoryAllocated = 1;

 	//Allocate space for the arrray of tokens
 	char **tokens = malloc(tokenMemoryAllocated * sizeof(char*));

 	//Set the firts token to be the first string
 	char *token = strtok(line, DELIM);

 	//if the token is not null (the line still has more arguments to be parsed)
 	while(token != NULL) {
 		//Reallocate one more space of memory
 		tokens = realloc(tokens, ++tokenMemoryAllocated * sizeof(char*));
 		//Set the current index of the array to store the token, then increment the index
 		tokens[index++] = token;
 		//resume parse from last point on the string
 		token = strtok(NULL, DELIM);
 	}
 	//Set the last index to be null
 	tokens[index] = NULL;
 	//Return the array of strings
  	return tokens;
}

//Check to see if the inputted command is built in, and pass to the launch method if not
int execute(char **arguments) {

	//Deal with no command entered
	if (arguments[0] == NULL) {
		printf(ERROR_COLOUR "Error");
		printf(NORMAL_COLOUR ": A command has not been entered, enter \"help\" for more information on \ncommands\n");
	    return 1;
	}

	//loop through my arracy of builinfuncs and check if the command enetered was one of them
	for (int i = 0; i < functionNumber; i++) {
	  if (strcmp(arguments[0], commands[i].name) == 0) {
	  	/* If the command enetered was one of the buil in commands return the result of that command's function
	  	 * NOTE: I pass the argument array into EVERY built in function even if it is not used in the function,
	  	 * as this is the only way i can structure my code like this (using the struct)
	  	 * doing this avoids a lengthy "if else" statement, and makes it much easier to add new commands to the program
	  	 * and meaans you dont have to woory about which command the user has actually entered
	  	 */  
	    return (commands[i].functionPointer)(arguments);
	  }
	}

	return launchProgram(arguments);
}

//Function forks the program  
int launchProgram(char **arguments) {

	int status;

	//Store the program id
	pid_t pid = fork();

	//Error forking
	if(pid < 0) {
	 	perror(ERROR_COLOUR "Error" NORMAL_COLOUR);
	}
	//Child executes the command and its arguments, printing an error if one occures
	else if(pid == 0) {
	  	execvp(arguments[0], arguments);
	  	perror(ERROR_COLOUR "Error" NORMAL_COLOUR);
	 	exit(EXIT_FAILURE);
	} 
	//Parent waits for child to finish excecuting
	else {
		waitpid(pid, &status, WUNTRACED);
	}

	//Continue running the shell
	return 1;
}

//Exit function returns 0 as the status so the shell can terminate
int exitfunc(char **arguments) {
	printf(SUCCES_COLOUR "\n Shell Exited\n" NORMAL_COLOUR);
	printf(NORMAL_COLOUR "\n");
	return 0;
}

//Change directory function, will attempt to change the directory to the first argument after the command
int cdfunc(char **arguments) {

	//Check if the user inputted an argument
	if(arguments[1] == NULL) {
		printf(ERROR_COLOUR "Error");
		printf(NORMAL_COLOUR ": expected argument\n" );
	}
	//Change directory if first argument is valid, if not print error, command doesnt check any other arguments as they are irrelevent 
	else if(chdir(arguments[1]) != 0) {
		perror(ERROR_COLOUR"Error" NORMAL_COLOUR);
	}
	//Continue running the shell
	return 1;
}

//Help function, will print all names of the built in commands, and their respective descriptions
int helpfunc(char **arguments) {

	printf(NORMAL_COLOUR"\n Joseph Whitten's Shell \n");
	printf(" These commands are built in :\n\n");

	//loop through the commands array of builtinfuncs, formatting and printing the name and description of each
	for(int i = 0; i < functionNumber; i++) {
		//Highlight the name
		printf(IMPORTANT_COLOUR "\t%s\t" NORMAL_COLOUR, commands[i].name); 
		printf(":%s\n", commands[i].functionDescription);
	}
	//Inform the user of the manual command
	printf("\n Enter: <man> <command> (e.g. man ls) to access the manual of commands that\n aren't built in\n\n");
	//Continue running the shell
	return 1;
}

//Prompt function, will change the prompt to be whatever the user entered after the command name (Prompt can be nothing if no arguments are entered)
int promptfunc(char **arguments) {

	//reset the prompt string to be nothing
	memset(prompt, 0, 1);
	prompt = "\0";

	//loop through all the arguments, concatinating all the arguments into a single string (spaces included)
	for(int i = 1; arguments[i] != NULL; i++) {
		strcat(prompt, arguments[i]);
		strcat(prompt, " ");
	}

	//Continue running the shell
	return 1;
}

//Toggle showing the current working directory
int wdfunc(char **arguments) {

	if(showCurrentWorkingDirectory)
		showCurrentWorkingDirectory = false;
	else {
		showCurrentWorkingDirectory = true;
	}
	//Continue running the shell
	return 1;
}

//Get the currecnt working directory and store it
void getwokdir(void) {

	getcwd(currentWorkingDirectory, PATH_MAX + 1);
}

//Change the color of the user text (Black text excluded)
int colourChange(char **arguments) {

	//Check if argment is valid
	if(arguments[1] == NULL || !(atoi(arguments[1]) > 30 && atoi(arguments[1]) < 38)) {
		//Print help message for user
		printf(ERROR_COLOUR"Error:"NORMAL_COLOUR);
		printf(" Invalid argument:\n Useage: \"colour\" [Colour number]\n Valid colors are:\n\n");
		for(int i = 31; i < sizeof(colours) / sizeof(char *) + 31; i++) {
			printf("\t\x1B[%dm%s", i, colours[i - 31]);
			printf("\t:%d\n", i);
		}
		printf("\n");
	} else {
		usercolor = atoi(arguments[1]); //change user text colour
	}
	return 1;
}