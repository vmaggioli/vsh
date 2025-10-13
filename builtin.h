/*
 * Declarations for builtin commands for the shell
 * */
int vsh_cd(char **args);
int vsh_help(char **args);
int vsh_exit(char **args);
int vsh_num_builtins(void);
char **get_builtin_str(void);
int (**get_builtin_func(void))(char **);
