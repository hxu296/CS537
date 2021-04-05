#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_COMMAND_LEN 1024
#define MAX_ALIAS_LEN 10000

typedef struct{
    alias *alias_pt;
    node *next;
} node;

typedef struct {
    char *command;
    char *tag;
    char **args;
} alias;

int parse_command(char command[], char *args[], char new_out[], int *redirect_pt){
    int i = 0, cmd_len = strlen(command), gt_pos = strcspn(command, ">");
    char *out;
    // parse output file path
    if(gt_pos < cmd_len){ 
        *redirect_pt = 1;
        command[gt_pos] = '\0'; // divide the argument part of command from the output part
        out = command + gt_pos + 1;
        if(strcspn(out, ">") < strlen(out)) return -1; // multiple > 
        if((out = strtok(out, " \t")) == NULL || strtok(NULL, " \t") != NULL) return -1; // end with > or multiple files
        strncpy(new_out, out, MAX_COMMAND_LEN);
    } else *redirect_pt = 0;
    // parse args
    args[i++] = strtok(command, " \t");
    while((args[i++] = strtok(NULL, " \t")) != NULL);
    return 0; 
}

void add_alias(alias *alias_table[], char *command, char *args[]){
    int i = 0;
    // allocate new alias
    alias *alias_pt = malloc(sizeof(alias));
    alias_pt -> command = command;
    alias_pt -> tag = args[1];
    alias_pt -> args = args + 2;
    // add new alias to the alias table
    while(alias_table[i++] != NULL);
    alias_table[i-1] = alias_pt;
}

void print_alias(alias *alias_table[]){
    int i, j;
    alias *alias_pt;
    for(i = 0; i < MAX_ALIAS_LEN; i++){
        if((alias_pt = alias_table[i]) != NULL){
            printf("%s ", alias_pt -> tag);
            fflush(stdout);
            for(j = 0; alias_pt -> args[j] != NULL; j++){
                printf("%s", alias_pt -> args[j]);
                fflush(stdout);
            }
            write(1, "\n", 2);
        }
    }  
}

void get_alias(alias *alias_table[], char **args){
    int i, j;
    alias *alias_pt;
    for(i = 0; i < MAX_ALIAS_LEN; i++){
        if((alias_pt = alias_table[i]) != NULL){
            if(strcmp(args[0], alias_pt -> tag) == 0) {
                for(j = 0; alias_pt -> args[j] != NULL; j++)
                    args[j] = alias_pt -> args[j];
                args[j] = NULL;
                break;
            }
        }
    }
}

void unalias(alias *alias_table[], char **args){
    int i, j;
    alias *alias_pt;
    for(i = 0; i < MAX_ALIAS_LEN; i++){
        if((alias_pt = alias_table[i]) != NULL){
            if(strcmp(args[1], alias_pt -> tag) == 0) {
                free(alias_pt -> command);
                free(alias_pt -> args - 2);
                free(alias_pt);
                alias_table[i] = NULL;
            }
        }
    }
}

int main(int argc, char *argv[]){
    alias *alias_table[MAX_ALIAS_LEN] = {NULL};
    while(1){
        // parse user command
        char * command = malloc(MAX_COMMAND_LEN);
        write(1, "mysh> ", 7);
        if(fgets(command, MAX_COMMAND_LEN, stdin) == NULL) exit(0);
        command[strcspn(command, "\n")] = 0; // replace new line character with end of string
        if(strlen(command) == 0) continue;
        int redirect;
        char **args = malloc(MAX_COMMAND_LEN * sizeof(char*)),
            new_out[MAX_COMMAND_LEN];
        if(parse_command(command, args, new_out, &redirect) != 0){
            write(1, "Redirection misformatted.\n", 27);
            continue;
        }
        if(args == NULL) continue; // no job is specified
        if(strcmp(args[0], "exit") == 0) exit(0);
        if(strcmp(args[0], "alias") == 0) {
            if(args[1] == NULL) print_alias(alias_table);
            else add_alias(alias_table, command, args);
            continue;
        }
        if(strcmp(args[0], "unalias") == 0) {unalias(alias_table, args); continue;}
        if(args[1] == NULL) get_alias(alias_table, args);
        // execute command
        pid_t pid = fork();
        if(pid != 0){ // parent process branch
            int status;
            waitpid(pid, &status, 0);
            if(status != 0){ // execution failed, job not found
                printf("%s: Command not found.\n", args[0]);
                fflush(stdout);
            }
        } else{ // child process branch
            int fd;
            if(redirect){
                fd = open(new_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, fileno(stdout));
                close(fd);
            }
            execv(args[0], args);
            _exit(1); // execution failed, terminate this child
        }
        // free command and args
        free(command);
        free(args);
    }
}        
