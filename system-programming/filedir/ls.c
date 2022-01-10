#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define ERR_EXIT(msg)                                                          \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define PERM_SIZE sizeof("rwxrwxrwx")

int subDirCounts(const char *pathname) {
  DIR *dirp;
  struct dirent *dp;
  int cnt = 0;
  errno = 0;

  if ((dirp = opendir(pathname)) == NULL)
    ERR_EXIT("opendir");

  while (dp = readdir(dirp))
    if (strncmp(dp->d_name, ".", 1) == 0)
      continue;
    else
    cnt++;
  if (errno)
    ERR_EXIT("readdir");

  return cnt;
}

/* lrwxrwxrwx 1 u20 u20    9 Jan 10 21:36 README.md.soft -> README.md */
void longFormattedInfo(const char *pathname) {
  struct stat sb;
  char perm[PERM_SIZE];
  struct passwd *pwd;
  struct group *grp;
  char a_time[64];

  if (lstat(pathname, &sb) == -1)
    ERR_EXIT("lstat");

  switch (sb.st_mode & S_IFMT) {
  case S_IFREG:
    printf("-");
    break;
  case S_IFDIR:
    printf("d");
    break;
  case S_IFCHR:
    printf("c");
    break;
  case S_IFBLK:
    printf("b");
    break;
  case S_IFLNK:
    printf("l");
    break;
  case S_IFIFO:
    printf("p");
    break;
  case S_IFSOCK:
    printf("s");
    break;
  default:
    printf("?");
  }

  snprintf(
      perm, PERM_SIZE, "%c%c%c%c%c%c%c%c%c", (sb.st_mode & S_IRUSR) ? 'r' : '-',
      (sb.st_mode & S_IWUSR) ? 'w' : '-', (sb.st_mode & S_IXUSR) ? 'x' : '-',
      (sb.st_mode & S_IRGRP) ? 'r' : '-', (sb.st_mode & S_IWGRP) ? 'w' : '-',
      (sb.st_mode & S_IXGRP) ? 'x' : '-', (sb.st_mode & S_IROTH) ? 'r' : '-',
      (sb.st_mode & S_IWOTH) ? 'w' : '-', (sb.st_mode & S_IXOTH) ? 'x' : '-');

  printf("%s ", perm);

  if ((sb.st_mode & S_IFMT) == S_IFDIR)
    printf("%d ", subDirCounts(pathname));
  else
    printf("%lu ", sb.st_nlink);

  errno = 0;
  if ((pwd = getpwuid(sb.st_uid)) == NULL) {
    if (errno)
      ERR_EXIT("getpwuid");
    printf("? ? ");
  } else {
    printf("%s ", pwd->pw_name);
    errno = 0;
    if ((grp = getgrgid(pwd->pw_gid)) == NULL) {
      if (errno)
        ERR_EXIT("getgrgid");
      printf("? ");
    } else
      printf("%s ", grp->gr_name);
  }

  printf("%lu ", sb.st_size);

  strcpy(a_time, ctime(&sb.st_atim));
  a_time[strlen(a_time) - 1] = '\0';
  printf("%s ", a_time);

  if ((sb.st_mode & S_IFMT) == S_IFLNK) {
    char buf[PATH_MAX];
    int ret;
    if ((ret = readlink(pathname, buf, PATH_MAX)) == -1)
      ERR_EXIT("readlink");
    buf[ret] = '\0';
    printf("%s -> %s\n", pathname, buf);
  } else {
    printf("%s\n", pathname);
  }
}

void noFileSpecified(const char *pathname, int l) {
  DIR *dirp;
  struct dirent *dp;

  if ((dirp = opendir(pathname)) == NULL)
    ERR_EXIT("opendir");

  for (;;) {
    errno = 0;
    if ((dp = readdir(dirp)) == NULL) // error or end-of-dir
      break;                          // error will set errno
    if (strncmp(dp->d_name, ".", 1) == 0)
      continue;
    if (l)
      longFormattedInfo(dp->d_name);
    else
      printf("%s\t", dp->d_name);
  }
  printf("\n");

  if (errno)
    ERR_EXIT("readdir");

  if (closedir(dirp) == -1)
    ERR_EXIT("closedir");
}

int main(int argc, char **argv) {
  if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    fprintf(stderr,
            "Usage: %s [-l] [file] [file] ... \n"
            "       -l    use a long listing format.\n"
            "If no file specified, list all files in the directory.\n",
            argv[0]);
  }

  if (argc == 1 || (argc == 2 && strcmp(argv[1], "-l") == 0)) {
    // No file specified
    int l = argc > 1; // if -l specified, l is 1, otherwise l is 0
    noFileSpecified(".", l);
  } else {
    int longFormat = (argc > 1) && strcmp(argv[1], "-l") == 0;
    int filePos = longFormat ? 2 : 1;
    while (filePos++ < argc)
      if (longFormat)
        longFormattedInfo(argv[filePos]);
      else
        printf("%s ", argv[filePos]);
  }

  return 0;
}