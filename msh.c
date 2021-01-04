//Aawaj Bhaukajee
//1001397724

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017, 2020 Trevor Bakker
// // Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
						   // so we need to define what delimits our tokens.
						   // In this case  white space
						   // will separate the tokens on our command line

// The maximum command-line size
#define MAX_COMMAND_SIZE 255

// Mav shell only supports five arguments
#define MAX_NUM_ARGUMENTS 5

//for most of the functions like showing pid value and history,
//15 values are required, so array of 15.
pid_t pidArray[15];

int pidIndex = 0;

//this is for storing 15 values of processes to find specific history.
char *histCmds[15];
int histIndex = 0;

int main()
{
	char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

	//char *history[15];
	//int history_index;

	while (1)
	{
		// Print out the msh prompt
		printf("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input

		while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));

		//to check history we start by entering '!', followed by the number
		//of the command which we want to execute again.

		if (cmd_str[0] == '!')
		{
			int extract;
			sscanf(cmd_str, "%*c%d", &extract);
			strcpy(cmd_str, histCmds[extract]);
		}

		//if the user enters nothing as an initial command,
		//then it will not be saved in the history.

		histCmds[histIndex] = (char *)malloc(strlen(cmd_str));
		if (strcmp(cmd_str, "\n") != 0)
		{
			strcpy(histCmds[histIndex], cmd_str);

			histIndex++;
		}

		// for loop to save atmost 15 commands in the history.
		if (histIndex >= 16)
		{
			int newHistIndex = 0;
			for (newHistIndex = 0; newHistIndex < histIndex - 1; newHistIndex++)
			{
				histCmds[newHistIndex] = histCmds[newHistIndex + 1];
			}
			histIndex--;
		}

		/* Parse input */

		char *token[MAX_NUM_ARGUMENTS];
		int token_count = 0;

		// Pointer to point to the token
		// parsed by strsep

		char *argument_ptr;

		char *working_str = strdup(cmd_str);

		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end

		char *working_root = working_str;

		// Tokenize the input stringswith whitespace used as the delimiter

		while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
			   (token_count < MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
			if (strlen(token[token_count]) == 0)
			{
				token[token_count] = NULL;
			}
			token_count++;
		}
		free(working_root);

		//whenever user does not type anything, shell will prompt and
		//accept a new line of input without giving any output.

		if (cmd_str[0] == '\n')
		{
			continue;
		}

		//whenever user wants to end a program, typing 'quit' or 'exit' in a shell will do so.
		else if ((strcmp(token[0], "quit") == 0) || (strcmp(token[0], "exit") == 0))
		{
			exit(0);
		}

		else if ((strcmp(token[0], "history")) == 0)
		{
			int i = 0;
			for (i = 0; i < histIndex; i++)
			{
				printf("[%d]: %s", i, histCmds[i]);
			}
		}

		//this will handle the 'cd' command:whenever user wants to go back and forth between
		//any directories in a shell

		else if (strcmp(token[0], "cd") == 0)
		{
			if (chdir(token[1]) == -1)
			{
				printf("%s: Folder not in directory.\n", token[1]);
			}
			continue;
		}

		//when user enters 'showpids', the for loop wil direct the shell to list the PIDs of
		// the latest 15 processes spawned by it, if there are less processes, then only the
		//PIDs of those processes will be spawned.

		else if (strcmp(token[0], "showpids") == 0)
		{
			int i = 0;

			for (i = 0; i < pidIndex; i++)
			{
				printf("%d\n", pidArray[i]);
			}
		}

		else
		{
			//error if pid is less than 0(when fork()returns les than 0)

			pid_t pid = fork();
			if (pid < 0)
			{
				perror("fork failed");
				exit(0);
			}
			else if (pid == 0)
			{
				//we are in the child process of fork() returns 0.

				//array of 10 to support 10 command lines parameter in addition
				//to the command.
				char *arguments[10];

				int i;

				for (i = 0; i < token_count - 1; i++)
				{
					//dynamically allocating memory for the paths

					arguments[i] = (char *)malloc(strlen(token[i]));
					strncpy(arguments[i], token[i], strlen(token[i]));
				}

				//making the last index 0, because we only need upto 10 command
				//lines parameter.
				arguments[token_count - 1] = NULL;

				//using execvp function to executing the paths.
				int ret = execvp(arguments[0], &arguments[0]);
				if (ret == -1)

				{
					//whenever the built-in commands are not recognized, error message
					//is shown.
					printf("\nCommand not found\n\n");
					exit(0);
				}
			}
			else
			{
				//now we are in the parent process because fork() returns a positive number
				int status;
				wait(&status);

				//pidArray will save the pid of each process
				pidArray[pidIndex] = pid;
				pidIndex++;

				if (pidIndex >= 16)
				{
					//we only need pid of the recent 15 processes.
					int newIndex = 0;
					for (newIndex = 0; newIndex < pidIndex - 1; newIndex++)
					{
						pidArray[newIndex] = pidArray[newIndex + 1];
					}
					pidIndex--;
				}
			}
		}
	}
	return 0;
}
