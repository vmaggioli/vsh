#include <curses.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"

#define VSH_RL_BUFFERSIZE 1024
#define VSH_TOK_DELIM " \t\r\n\a"
#define VSH_TOK_BUFSIZE 24
#define PROMPT "> "

bool delete_char(void) {
  int y, x;
  getyx(stdscr, y, x);
  if (x <= strlen(PROMPT))
    return false;
  move(y, x - 1);
  delch();
  refresh();
  return true;
}

void print_autocomplete_suggestions(char *command) {
  char *path, *pathToken, *firstMatch;
  const char *delim = ":";
  DIR *dir;
  struct dirent *directory;
  int commandLength = strlen(command);

  path = strdup(getenv("PATH"));
  pathToken = strtok(path, delim);
  printw("\n");
  while (pathToken != NULL) {
    dir = opendir(pathToken);
    if (!dir) {
      printw("\nError opening \"%s\"\n", pathToken);
      printw("%s\n", errno ? strerror(errno) : "Unknown error");
      closedir(dir);
      refresh();
      return;
    }

    while ((directory = readdir(dir)) != NULL) {
      if (strncmp(directory->d_name, command, commandLength) == 0)
        printw("%s  ", directory->d_name);
    }

    pathToken = strtok(NULL, delim);
    closedir(dir);
  }

  printw("\n");
  refresh();
}

char *vsh_read_line(void) {
  int ch, position;
  int buffersize = VSH_RL_BUFFERSIZE;
  char *line = malloc(buffersize * sizeof(char *));
  char *retLine = line;

  while ((ch = getch()) != '\n') {
    switch (ch) {
    case KEY_BACKSPACE: {
      if (!delete_char())
        continue;
      line--;
      *line = '\0';
      position--;
      continue;
    }
    case '\t': {
      print_autocomplete_suggestions(retLine);
      printw("\n%s %s", PROMPT, retLine);
      refresh();
      continue;
    }
    }

    addch(ch);
    refresh();

    *line = ch;
    position++;

    if (position == buffersize) {
      buffersize += VSH_RL_BUFFERSIZE;
      line = realloc(line, buffersize);
    }

    line++;
  }

  line = NULL;
  addch('\n');
  refresh();
  return retLine;
}

char **vsh_split_line(char *line) {
  int bufferSize = VSH_TOK_BUFSIZE;
  int position = 0;
  char *token;
  char **tokens = malloc(sizeof(char *) * bufferSize);
  char *currToken = NULL;

  if (!tokens) {
    printw("Error allocating tokens buffer\n");
    refresh();
    exit(EXIT_FAILURE);
  }

  token = strtok(line, VSH_TOK_DELIM);
  while (token != NULL) {
    if (*token == '"' && !currToken) {
      currToken = token;
    } else if (token[strlen(token) - 1] == '"' && !currToken) {
      currToken = strcat(currToken, token);
      tokens[position++] = currToken;
      currToken = NULL;
    } else if (currToken) {
      currToken = strcat(currToken, token);
    } else {
      tokens[position++] = token;
    }

    if (position >= bufferSize) {
      bufferSize += VSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufferSize * sizeof(char *));
      if (!tokens) {
        printw("Error allocating return tokens buffer\n");
        refresh();
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, VSH_TOK_DELIM);
  }

  if (currToken != NULL)
    tokens[position++] = currToken;

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
    if (strcmp(args[0], builtin_str[i]) == 0)
      return (*builtin_func[i])(args);
  }

  return vsh_launch(args);
}

void vsh_loop() {
  char *line;
  char **args;
  int status, index;

  do {
    printw(PROMPT);
    refresh();
    // Can't free 'line' - why?
    line = vsh_read_line();
    args = vsh_split_line(line);
    status = vsh_execute(args);
  } while (status);

  index = 0;
  while (args[index] != NULL)
    free(args[index++]);
  free(args);
}

int main(int argc, char *argv[]) {
  initscr();
  cbreak();
  noecho();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
  vsh_loop();
  return EXIT_SUCCESS;
}
