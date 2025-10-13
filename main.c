#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"

#define VSH_RL_BUFFERSIZE 1024;
#define VSH_TOK_DELIM " \t\r\n\a"
#define VSH_TOK_BUFSIZE 24

char *vsh_read_line(void) {
  char *line = NULL;
  size_t buffsize = 0;

  int check = getline(&line, &buffsize, stdin);
  if (check == -1) {
    if (feof(stdin))
      exit(EXIT_SUCCESS);
    else {
      printf("Error while reading line\n");
      exit(EXIT_FAILURE);
    }
  }
}

// Assumes no quotes to group tokens
char **vsh_split_line(char *line) {
  int strTokBufSize = VSH_TOK_BUFSIZE;
  int retTokBufSize = VSH_TOK_BUFSIZE;
  int strtokPos = 0;
  int retTokenPos = 0;
  char *token;
  char **tokens = malloc(sizeof(char *) * strTokBufSize);
  char **retTokens = malloc(sizeof(char *) * retTokBufSize);
  char *currToken = NULL;

  if (!tokens) {
    fprintf(stderr, "Error allocating tokens buffer\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, VSH_TOK_DELIM);
  while (token != NULL) {
    if (token[0] == '"' && currToken == NULL) {
      printf("Here1\n");
      currToken = token;
    } else if (token[strlen(token) - 1] == '"' && currToken == NULL) {
      printf("Here2\n");
      currToken = strcat(currToken, token);
      retTokens[retTokenPos++] = currToken;
      currToken = NULL;
    } else if (currToken) {
      printf("Here3\n");
      currToken = strcat(currToken, token);
    } else {
      printf("Here4\n");
      retTokens[retTokenPos++] = token;
    }

    if (retTokenPos >= retTokBufSize) {
      retTokBufSize += VSH_TOK_BUFSIZE;
      retTokens = realloc(retTokens, retTokBufSize * sizeof(char *));
      if (!retTokens) {
        fprintf(stderr, "Error allocating return tokens buffer\n");
        exit(EXIT_FAILURE);
      }
    }

    token = tokens[strtokPos++];

    if (strtokPos >= strTokBufSize) {
      strTokBufSize += VSH_TOK_BUFSIZE;
      tokens = realloc(tokens, strTokBufSize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "Error allocating current tokens buffer\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  if (currToken != NULL)
    retTokens[retTokenPos++] = currToken;

  retTokens[retTokenPos] = NULL;
  free(tokens);
  return retTokens;
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
    printf("%s:%s\n", args[1], builtin_str[i]);
    if (strcmp(args[1], builtin_str[i]) == 0)
      return (*builtin_func[i])(args);
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
