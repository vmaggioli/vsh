#include "builtin.h"
#include <curses.h>
#include <stdio.h>
#include <unistd.h>

char *builtin_str[] = {"cd", "help", "?", "exit", "echo"};
int (*builtin_func[])(char **) = {&vsh_cd, &vsh_help, &vsh_help, &vsh_exit,
                                  &vsh_echo};
// TODO - Do this dynamically
int vsh_num_builtins(void) { return 5; }

char **get_builtin_str(void) { return builtin_str; };
int (**get_builtin_func(void))(char **) { return builtin_func; }

int vsh_cd(char **args) {
  if (args[1] == NULL) {
    printw("vsh: expected argument to \"cd\"\n");
    refresh();
  } else {
    if (chdir(args[1]) != 0)
      perror("vsh");
  }
  return 0;
}

int vsh_help(char **args) {
  int i, j;
  printw("Vincent Maggioli's vsh\n");
  printw("Type program names and arguments, and hit enter.\n");
  printw("The following are built in:\n");

  for (i = 0; i < vsh_num_builtins(); i++) {
    printw("%s\t", builtin_str[i]);
  }

  printw("\n\n");

  printw("Use the man command for information on other programs.\n");
  refresh();
  return 1;
}

// Only support echo of one arg - no flags
int vsh_echo(char **args) {
  int count = 0;
  char *check = *args;
  // Command should be echo and a quoted set of words or single word
  while (check && count != 3) {
    count++;
    check = *(args + count);
  }
  if (count != 2) {
    printw("Error running 'echo': invalid number of arguments\n");
    refresh();
    return -1;
  }

  printw("%s\n", args[1]);
  refresh();
  return 0;
}

int vsh_exit(char **args) {
  endwin();
  return 0;
}
