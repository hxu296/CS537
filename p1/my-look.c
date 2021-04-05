// Copyright 2021 Huan Xu

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *open_file(char *path_to_file, char *mode) {
  FILE *file;
  // use stdin if path_to_file is not set
  if (path_to_file == NULL) {
    file = stdin;
  } else {
    // try to open path_to_file otherwise
    file = fopen(path_to_file, mode);
    if (file == NULL) {
      printf("my-look: cannot open file\n");
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

bool starts_with(char *pre, char *str) {
  size_t pre_len = strlen(pre), str_len = strlen(str);
  return pre_len > str_len ? false : strncasecmp(pre, str, pre_len) == 0;
}

int main(int argc, char *argv[]) {
  // parse command line arguments
  char *path_to_file = NULL, *prefix = NULL;

  int opt;
  while (optind < argc) {
    if ((opt = getopt(argc, argv, "Vhf:")) != -1) {
      switch (opt) {
        case 'V':
          printf("my-look from CS537 Spring 2021\n");
          exit(0);
        case 'h':
          printf(
              "usage: my-look [-f file] [-V] [-h] prefix\n"
              "\t -f file: use file as the input dictionary\n"
              "\t -V: display program specification and exit\n"
              "\t -h: display this help and exit\n"
              "\t prefix: echo input dictionary lines that start with "
              "prefix\n");
          exit(0);
        case 'f':
          path_to_file = optarg;
          break;
        case '?':
          printf("my-look: invalid command line\n");
          exit(1);
      }
    } else if (optind == argc - 1) {
      prefix = argv[optind];
      break;
    } else {
      printf("my-look: invalid command line\n");
      exit(1);
    }
  }

  // prepare variables
  FILE *file = open_file(path_to_file, "r");

  if (prefix == NULL) {
    printf("my-look: invalid command line\n");
    exit(1);
  }

  // print lines in file that begin with prefix
  int kLength = 512;
  char line[kLength];
  while (fgets(line, kLength, file) != NULL) {
    if (starts_with(prefix, line)) {
      printf("%s", line);
    }
  }

  // close file
  close_file(file);

  return 0;
}
