/* ls1.c
 *   purpose list contens of directory or directories
 *   action if no args, use . else list files in args
 *
 *   modify from
 *   Bruce Molay's <Understanding Unix/Linux Programming>
 *   by tgsheng(GitHub)
 */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define errExit(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void do_ls(const char *path);

int main(int argc, const char **argv) {
  if (argc == 1)
    do_ls(".");
  else
    while (--argc) {
      printf("%s:\n", *++argv);
      do_ls(*argv);
    }

  return 0;
}

void do_ls(const char *path) {
  DIR *dirp;
  struct dirent *direntp;

  if ((dirp = opendir(path)) == NULL)
    errExit("opendir");

  errno = 0;
  while ((direntp = readdir(dirp)))
    printf("%s\n", direntp->d_name);
  if (errno)
    errExit("readdir");
  closedir(dirp);
}