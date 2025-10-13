#include "builtin.h"
#include <stdio.h>
#include <unistd.h>

char *builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[])(char **) = {&vsh_cd, &vsh_help, &vsh_exit};
int vsh_num_builtins(void) { return sizeof(builtin_str) / sizeof(char *); }

char **get_builtin_str(void) { return builtin_str; };
int (**get_builtin_func(void))(char **) { return builtin_func; }

int vsh_cd(char **args) {
  if (args[1] == NULL)
    fprintf(stderr, "vsh: expected argument to \"cd\"\n");
  else {
    if (chdir(args[1]) != 0)
      perror("vsh");
  }
  return -1;
}

int vsh_help(char **args) {
  int i;
  printf("Vincent Maggioli's vsh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < vsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int vsh_exit(char **args) { return 0; }
