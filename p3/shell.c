#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 1024

typedef struct {
  char *command_pt;
  char **args_pt;
  char *tag;
  char **args;
} alias;

typedef struct {
  alias *alias_pt;
  void *next;
  void *prev;
} node;

int parse_command(char command[], char *args[], char new_out[],
                  int *redirect_pt) {
  int i = 0, cmd_len = strlen(command), gt_pos = strcspn(command, ">");
  char *out;
  // parse output file path
  if (gt_pos < cmd_len) {
    *redirect_pt = 1;
    command[gt_pos] =
        '\0';  // divide the argument part of command from the output part
    out = command + gt_pos + 1;
    if (strcspn(out, ">") < strlen(out)) return -1;  // multiple >
    if ((out = strtok(out, " \t")) == NULL || strtok(NULL, " \t") != NULL)
      return -1;  // end with > or multiple files
    strncpy(new_out, out, MAX_COMMAND_LEN);
  } else {
    *redirect_pt = 0;
  }
  // parse args
  args[i++] = strtok(command, " \t");
  while ((args[i++] = strtok(NULL, " \t")) != NULL) {
    continue;
  }
  return 0;
}

void add_alias(node *head, char *command, char **args) {
  // check if potential tag is valid
  if (strcmp("alias", args[1]) == 0 || strcmp("unalias", args[1]) == 0 ||
      strcmp("exit", args[1]) == 0) {
    fprintf(stderr, "alias: Too dangerous to alias that.\n");
    fflush(stderr);
    return;
  }
  // allocate new alias node
  alias *new_alias = malloc(sizeof(alias));
  new_alias->command_pt = command;
  new_alias->args_pt = args;
  new_alias->tag = args[1];
  new_alias->args = args + 2;
  node *node_pt = malloc(sizeof(node));
  node_pt->alias_pt = new_alias;
  node_pt->next = NULL;
  // add new node to the alias linked list
  node *runner = head;
  alias *curr_alias;
  while (runner->next != NULL) {
    curr_alias = ((node *)runner->next)->alias_pt;
    if (strcmp(new_alias->tag, curr_alias->tag) == 0) {
      ((node *)runner->next)->alias_pt = new_alias;
      return;
    }
    runner = (node *)runner->next;
  }
  node_pt->prev = (void *)runner;
  runner->next = (void *)node_pt;
}

void print_alias(node *head) {
  node *runner = (node *)head->next;
  alias *alias_pt;
  int i;
  while (runner != NULL) {
    alias_pt = runner->alias_pt;
    printf("%s ", alias_pt->tag);
    fflush(stdout);
    for (i = 0; alias_pt->args[i] != NULL; i++) {
      printf("%s", alias_pt->args[i]);
      fflush(stdout);
      if (alias_pt->args[i + 1] != NULL) {
        printf(" ");
        fflush(stdout);
      }
    }
    printf("\n");
    fflush(stdout);

    runner = (node *)runner->next;
  }
}

void get_alias(node *head, char **args) {
  node *runner = (node *)head->next;
  alias *alias_pt;
  int i;
  while (runner != NULL) {
    alias_pt = runner->alias_pt;
    if (strcmp(args[0], alias_pt->tag) == 0) {
      for (i = 0; alias_pt->args[i] != NULL; i++) args[i] = alias_pt->args[i];
      args[i] = NULL;
      break;
    }
    runner = (node *)runner->next;
  }
}

void find_alias(node *head, char **args) {
  node *runner = (node *)head->next;
  alias *alias_pt;
  int i;
  while (runner != NULL) {
    alias_pt = runner->alias_pt;
    if (strcmp(args[1], alias_pt->tag) == 0) {
      printf("%s ", alias_pt->tag);
      fflush(stdout);
      for (i = 0; alias_pt->args[i] != NULL; i++) {
        printf("%s", alias_pt->args[i]);
        fflush(stdout);
        if (alias_pt->args[i + 1] != NULL) {
          printf(" ");
          fflush(stdout);
        }
      }
      printf("\n");
      fflush(stdout);
      break;
    }
    runner = (node *)runner->next;
  }
}

void unalias(node *head, char **args) {
  node *runner = (node *)head->next;
  alias *alias_pt;
  while (runner != NULL) {
    alias_pt = runner->alias_pt;
    if (strcmp(args[1], alias_pt->tag) == 0) {
      ((node *)runner->prev)->next = runner->next;
      if (runner->next != NULL) ((node *)runner->next)->prev = runner->prev;
      free(runner -> alias_pt -> args_pt);
      free(runner -> alias_pt -> command_pt);
      free(runner -> alias_pt);
      free(runner);
      break;
    }
    runner = (node *)runner->next;
  }
}

void free_heap(node *head){
    node *runner = (node *) head -> next;
    node *next;
    while(runner != NULL){
        next = (node*) runner -> next;
        free(runner -> alias_pt -> args_pt);
        free(runner -> alias_pt -> command_pt);
        free(runner -> alias_pt);
        free(runner);
        runner = next;
    }
    free(head);
}

int main(int argc, char *argv[]) {
  node *head = malloc(sizeof(node));  // dummy head for alias node linked list
  head->alias_pt = head->next = head->prev = NULL;
  int batch;
  if (argc == 2) {  // batch mode
    batch = 1;
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "Error: Cannot open file %s.\n", argv[1]);
      fflush(stderr);
      exit(1);
    } else {
      dup2(fd, fileno(stdin));
      close(fd);
    }
  } else if (argc == 1) {
    batch = 0;
  } else {  // incorrect number of arguments
    fprintf(stderr, "Usage: mysh [batch-file]\n");
    fflush(stderr);
    exit(1);
  }
  while (1) {
    // parse user command
    char *command = malloc(MAX_COMMAND_LEN);
    if (!batch) write(1, "mysh> ", 6);
    if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) exit(0);
    if (batch) {
      printf("%s", command);
      fflush(stdout);
    }
    command[strcspn(command, "\n")] =
        0;  // replace new line character with end of string
    if (strlen(command) == 0) continue;
    int redirect;
    char **args = malloc(MAX_COMMAND_LEN * sizeof(char *)),
         new_out[MAX_COMMAND_LEN];
    if (parse_command(command, args, new_out, &redirect) != 0 ||
        (redirect == 1 && args[0] == NULL)) {
      fprintf(stderr, "Redirection misformatted.\n");
      fflush(stderr);
      continue;
    }
    if (args[0] == NULL) continue;  // no job is specified
    if (strcmp(args[0], "exit") == 0) {free_heap(head); free(command); free(args); exit(0);}
    if (strcmp(args[0], "alias") == 0) {
      if (args[1] == NULL)
        print_alias(head);
      else if (args[2] == NULL)
        find_alias(head, args);
      else
        add_alias(head, command, args);
      continue;
    }
    if (strcmp(args[0], "unalias") == 0) {
      if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "unalias: Incorrect number of arguments.\n");
        fflush(stderr);
        continue;
      }
      unalias(head, args);
      continue;
    }
    if (args[1] == NULL) get_alias(head, args);
    // execute command
    pid_t pid = fork();
    if (pid != 0) {  // parent process branch
      int status;
      waitpid(pid, &status, 0);
    } else {  // child process branch
      int fd;
      if (redirect) {
        fd = open(new_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
          fprintf(stderr, "Cannot write to file %s", new_out);
          fflush(stderr);
          _exit(1);
        }
        dup2(fd, fileno(stdout));
        close(fd);
      }
      if (access(args[0], F_OK) != 0) {
        fprintf(stderr, "%s: Command not found.\n", args[0]);
        fflush(stderr);
        _exit(1);
      }
      execv(args[0], args);
      _exit(1);  // execution failed, terminate this child
    }
    // free command and args
    free(command);
    free(args);
  }
}
