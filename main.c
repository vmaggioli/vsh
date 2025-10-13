#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"

#define VSH_RL_BUFFERSIZE 1024;
#define VSH_TOK_DELIM " \t\r\n\a"
#define vsh_TOK_BUFSIZE 24

char *vsh_read_line(void) {
  char *line = NULL;
  size_t buffsize = 0;

  while (1) {
    if (!getline(&line, &buffsize, stdin)) {
      if (feof(stdin))
        exit(EXIT_SUCCESS);
      else {
        printf("Error while reading line\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

// Assumes no quotes to group tokens
char **vsh_split_line(char *line) {
  int buffsize = vsh_TOK_BUFSIZE;
  int position = 0;
  char *token;
  char **tokens = malloc(sizeof(char *) * buffsize);

  if (!tokens) {
    fprintf(stderr, "Error allocating tokens buffer\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, VSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position++] = token;

    if (position >= buffsize) {
      buffsize += vsh_TOK_BUFSIZE;
      tokens = realloc(tokens, buffsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "Error allocating tokens buffer\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  tokens[position] = NULL;
  return tokens;
}

int vsh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("vsh");
      exit(EXIT_FAILURE);
    }
  } else if (pid < -1) {
    // Error
    perror("vsh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int vsh_execute(char **args) {
  int i, j;
  char **builtin_str = get_builtin_str();
  int (**builtin_func)(char **) = get_builtin_func();

  if (args[0] == NULL)
    return 1;

  for (i = 0; i < vsh_num_builtins(); i++) {
    for (j = 0; j < vsh_num_builtins(); j++) {
      if (strcmp(args[1], &builtin_str[i][j]) == 0)
        return (*builtin_func[i])(args);
    }
  }

  return vsh_launch(args);
}

void vsh_loop() {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = vsh_read_line();
    args = vsh_split_line(line);
    status = vsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char *argv[]) {
  vsh_loop();
  return EXIT_SUCCESS;
}
