#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  if(strncmp(cmd, "echo", 4) == 0) {
    sh_printf("%s", cmd + 5);
  }
  else if(strncmp(cmd, "ls", 2) == 0) {
    sh_printf("ls: not implemented");
  }
  else if(strncmp(cmd, "cat", 3) == 0) {
    sh_printf("cat: not implemented");
  }
  else if(strncmp(cmd, "exit", 4) == 0) {
    exit(0);
  }
  else if(strncmp(cmd, "exec", 4) == 0)
  {
    char *args[10];
    int i = 0;
    char *p = strtok((char *)cmd + 5, " ");
    while(p != NULL)
    {
      args[i++] = p;
      p = strtok(NULL, " ");
    }
    args[i] = NULL;
    execvp(args[0], args);
    sh_printf("exec: command not found");
  }
  else if(strncmp(cmd, "set", 3) == 0)
  {
    char *p = strtok((char *)cmd + 4, " ");
    if(p == NULL)
    {
      sh_printf("set: no variable name");
      return;
    }
    char *name = p;
    p = strtok(NULL, " ");
    if(p == NULL)
    {
      sh_printf("set: no value\n");
      return;
    }
    char *value = p;
    setenv(name, value, 1);
  }
  else if(strncmp(cmd, "get", 3) == 0)
  {
    char *p = strtok((char *)cmd + 4, " ");
    if(p == NULL)
    {
      sh_printf("get: no variable name");
      return;
    }
    char *name = p;
    char *value = getenv(name);
    if(value == NULL)
    {
      sh_printf("get: variable not found");
      return;
    }
    sh_printf("%s\n", value);
  }
  else if(strncmp(cmd, "help", 4) == 0)
  {
    sh_printf("echo [string]: echo string\n");
    sh_printf("ls: list files\n");
    sh_printf("cat [file]: print file content\n");
    sh_printf("exit: exit shell\n");
    sh_printf("exec [command] [args]: execute command\n");
    sh_printf("set [name] [value]: set environment variable\n");
    sh_printf("get [name]: get environment variable\n");
    sh_printf("help: show this message\n");
  }
  else {
    sh_printf("Unknown command: %s", cmd);
  }
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
