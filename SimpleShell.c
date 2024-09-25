#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

//This part of the code defines all the variables and sets limits for arguments and commands.

#define MAX_PIPES 10
#define MAX_COMMAND_LEN 1024 //The maximum length of a command.
#define MAX_ARGS 64 //The maximum number of arguments a command can have.
#define MAX_COMMAND_NUM 1000 //The maximum number of commands you can enter.
char* history[MAX_COMMAND_NUM]; //The history array
int history_count = 0;

void history_add(const char* command){ // Command to add the previous command to history.
    if (history_count < MAX_COMMAND_NUM){
        history[history_count] = strdup(command); //THe strdup command duplicates an array and returns a pointer to the location where the duplicate array is stored, which is stored in the history array.
        history_count++;
    }
}

void show_history() { //Prints a list of the history of commands
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

int tokenize(char* input, char** args) { //This function returns the number of arguments in the code, and also splits the string according to " " or "\n".
    int count = 0;
    args[count] = strtok(input, " \n"); //strotk splits the string based on the delimiter given.
    while (args[count] != NULL && count < MAX_ARGS - 1) { //Keeps splitting and storing in the args array.
        count++;
        args[count] = strtok(NULL, " \n");
    }
    args[count] = NULL; //Specifies the end of the list with NULL, since number of arguments can vary, it is important to end the list with NULL.
    return count;
}

void execute_background_cmd(char* input) { //This runs the program in the background and the parent doesn't wait for the child process.
    char* args[MAX_ARGS];
    strtok(input, "&");
    tokenize(input, args);

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1){ // If execvp fails, then it prints error.
            printf("Error executing background command\n");
        }
        exit(0);
    }else if (pid < 0){ //Error checking for the child process.
        printf("Error in forking background command\n");
    }
}

void execute_command(char* input) { //This runs the command using fork. Very similar to the backround except, the parent process waits for the child.
    char* args[MAX_ARGS];
    tokenize(input, args);

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            printf("Error executing command\n");
        }
        exit(0);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        printf("Error while forking child\n");
    }
}

void execute_command_pipe(char* input) { // This function is to execute commands which include the pipe operator ("|")
    char* args[MAX_ARGS];
    char* command1 = strtok(input, "|"); //Splits the string to get command 1 and 2, and store them accordingly.
    char* command2 = strtok(NULL, "|");

    int fd[2]; //This array stores the file descriptors
    pipe(fd); //Makes the pipes

    pid_t pid1 = fork(); //Chils process 1
    if (pid1 == 0) {
        close(fd[0]); //Closes the read end of the file.
        dup2(fd[1], STDOUT_FILENO); //Redirects the output of the first command to the input of the second command by writing in the file
        tokenize(command1, args); //Stores the split arguments in the args array.
        if (execvp(args[0], args) == -1){ // Executes the command and error checks.
            printf("Error in executing command 1\n");
        }
        exit(0);
    }else if (pid1 < 0){
        printf("Error in forking command 1\n");
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(fd[1]); //Closes the write end of the file.
        dup2(fd[0], STDIN_FILENO); //Redirects the input to the read end of the file.
        tokenize(command2, args);
        if (execvp(args[0], args) == -1){  
            printf("Error in executing command 2\n");
        }
        exit(0);
    }else if (pid2 < 1){
        printf("Error in forking command 2\n");
    }

    close(fd[0]); //The parent process closes both the file descriptors.
    close(fd[1]);
    wait(NULL); //The parent process waits for both the children to complete.
    wait(NULL);

}

int main() {
    char input[MAX_COMMAND_LEN]; // Instantiate the input string to a maximum of 1024 characters.
    while (1) {
        printf("iiitdleaknfreaksys> "); //This is where the user types in the commands.
        if (fgets(input, sizeof(input), stdin) == NULL) { //Takes the input from user and if it is NULL (ctrl + C), then program exits from shell
            break;
        }
        if (strcmp(input, "history\n") == 0) { //If the user enters "history", then it prints the history.
            show_history();
            continue;
        }
        history_add(input); // Adds the input to the history of commands
        if(strcmp(input, "exit\n") == 0){
            break;
        }
        if (strchr(input, '|')) { //If the input contains '|' (pipe operator), then use the pipe command function.
            execute_command_pipe(input);
        } 
        else if (strchr(input, '&')) { //If the input contains '&' sign, then run program in background
            execute_background_cmd(input);
        } 
        else {
            execute_command(input); // Standard execution of command.
        }

    }
    return 0;
}