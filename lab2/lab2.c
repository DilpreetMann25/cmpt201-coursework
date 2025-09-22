#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *basename_const(const char *path) {
  const char *slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

static char *trim(char *s) {
  if (!s)
    return s;
  // left trim
  while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
    s++;
  // right trim
  size_t len = strlen(s);
  while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                     s[len - 1] == '\r' || s[len - 1] == '\n')) {
    s[--len] = '\0';
  }
  return s;
}

int main(void) {
  char *line = NULL;
  size_t cap = 0;

  for (;;) {
    printf("Enter programs to run.\n> ");
    fflush(stdout);

    ssize_t n = getline(&line, &cap, stdin);
    if (n == -1) {
      putchar('\n');
      break;
    }
    char *saveptr = NULL;
    char *first = strtok_r(line, " \t\r\n", &saveptr);

    if (!first) {
      continue;
    }

    if (strcmp(first, "exit") == 0) {
      break;
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      continue;
    } else if (pid == 0) {
      const char *argv0 = basename_const(first);
      execl(first, argv0, (char *)NULL);

      fprintf(stderr, "Exec failure\n");
      _exit(127);
    } else {
      int status = 0;
      (void)waitpid(pid, &status, 0);
    }
  }

  free(line);
  return 0;
}
