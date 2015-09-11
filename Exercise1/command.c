#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "command.h"

#define MAX_CMD_COUNT 50
#define MAX_CMD_LEN 25


/*
 * PURPOSE: Parse the supplied string and store in in cmd structure provided
 * INPUTS:
 *		-input : user command line input
 *		-cmd : Pointer to Commands_t* to store the parsed input in
 * RETURN: True on successful parse and storing, else false
 **/
bool parse_user_input (const char* input, Commands_t** cmd) {

	// Check parameters
	if(!input || !cmd) {
		return false;
	}

	char *string = strdup(input);
	
	*cmd = calloc (1,sizeof(Commands_t));
	(*cmd)->cmds = calloc(MAX_CMD_COUNT,sizeof(char*));

	unsigned int i = 0;
	char *token;
	token = strtok(string, " \n");
	for (; token != NULL && i < MAX_CMD_COUNT; ++i) {
		(*cmd)->cmds[i] = calloc(MAX_CMD_LEN,sizeof(char));
		if (!(*cmd)->cmds[i]) {
			perror("Allocation Error\n");
			return false;
		}	
		strncpy((*cmd)->cmds[i],token, strlen(token) + 1);
		(*cmd)->num_cmds++;
		token = strtok(NULL, " \n");
	}
	free(string);
	return true;
}

/*
 * PURPOSE: Free the allocated memory used for storing parsed commands
 * INPUTS:
 *		-cmd : Pointer to Commands_t* of memory to free with
 * RETURN: NONE
 **/
void destroy_commands(Commands_t** cmd) {

	// Check parameter
	if(!cmd || !(*cmd)) {
		return;
	}
	
	for (int i = 0; i < (*cmd)->num_cmds; ++i) {
		free((*cmd)->cmds[i]);
	}
	free((*cmd)->cmds);
	free((*cmd));
	*cmd = NULL;
}

