// Copyright 2021 Huan Xu

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *open_file(char *path_to_file, char *mode) {
  FILE *file;
  // use stdin as input if path_to_file is not set
  if (path_to_file == NULL) {
    file = stdin;
  } else {
    // try to open path_to_file otherwise
    file = fopen(path_to_file, mode);
    if (file == NULL) {
      printf("my-rev: cannot open file\n");
      exit(1);
    }
  }
  return file;
}

void close_file(FILE *file) {
  // close file if it is not stdin
  if (file != stdin) {
    if (fclose(file) != 0) {
      printf("file close failed: %s\n", strerror(errno));
      exit(1);
    }
  } else {
    // redirect output to /dev/null otherwise
    freopen("/dev/null", "r", file);
  }
  return;
}

int length(char *line) {
  int i = 0;
  while (line[i] != '\n' && line[i] != '\0') i++;
  return i;
}

void reverse(char *line) {
  int kLength = length(line), i = kLength, j = 0;
  char rev[kLength];
  for (; i > 0; i--) rev[kLength - i] = line[i - 1];
  for (; j < kLength; j++) line[j] = rev[j];
  return;
}

int main(int argc, char *argv[]) {
  // parse command line arguments
  char *path_to_file = NULL;
  int opt;
  while (optind < argc) {
    if ((opt = getopt(argc, argv, "Vhf:")) != -1) {
      switch (opt) {
        case 'V':
          printf("my-rev from CS537 Spring 2021\n");
          exit(0);
        case 'h':
          printf(
              "usage: my-rev [-f file] [-V] [-h]\n"
              "\t -f file: use file as the input dictionary\n"
              "\t -V: display program specification and exit\n"
              "\t -h: display this help and exit\n");
          exit(0);
        case 'f':
          path_to_file = optarg;
          break;
        case '?':
          printf("my-rev: invalid command line\n");
          exit(1);
      }
    } else {
      printf("my-rev: invalid command line\n");
      exit(1);
    }
  }

  // prepare variables
  FILE *file = open_file(path_to_file, "r");

  // reverse each line of the input dictionary
  int kLength = 512;
  char line[kLength];
  while (fgets(line, kLength, file) != NULL) {
    reverse(line);
    printf("%s", line);
  }

  // close file
  close_file(file);

  return 0;
}
