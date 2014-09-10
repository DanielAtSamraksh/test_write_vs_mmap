#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


/// Create the directories necessary to have a path. 
/// if the `to` flog is true, treat the path as a filename
/// and do not make the last component a directory.

int do_mkpath(const char *path, bool to) {
  // printf("making %s\n", path);
  char *pathcopy = strdup(path);
  if (!pathcopy) {
    printf ("strdup error\n");
    return 1;
  }
  else {
    // printf("successfully copied %s\n", pathcopy);
  }
  int retval = 0;
  int len = strlen(path);
  // printf("len is %d\n", len);
  char *last = pathcopy;
  
// build the path component-wise, starting at the beginning, zeroing the last segment.
  do {
    // printf("in while loop, pathcopy = %ld, last = %ld\n", (long) pathcopy, (long) last); 
    // last = strchrnul(last+1, '/');
    last = strchr(last+1, '/');
    if (last) *last = '\0';
    else if (to) { // stop here
      break;
    }
    else last = pathcopy + len;
    // printf("working on %s\n", pathcopy);

    struct stat st;
    if (stat(pathcopy, &st) == 0) { // path exists
      printf("path %s exists\n", pathcopy);
      if (! S_ISDIR(st.st_mode)) {
	printf ("mkpath: error: %s exists and is not a directory!\n", pathcopy);
	retval = 1;
	break;
      }
    }
    else if (mkdir(pathcopy, 0777)) {
      printf ("mkpath: error: cannot create %s!\n", pathcopy); 
      retval = 1;
      break;
    }
    *last = '/'; // continue
    // printf("Changing back\n");
  } while (last < pathcopy + len - 1);

  free (pathcopy);
  return retval;
}      

int mkpath(const char *path) { return do_mkpath(path, false); }
int mkpathto(const char *path) { return do_mkpath(path, true); }

#ifdef TEST_MKPATH


int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s directory_name\n", argv[0]);
    exit (1);
  }
  mkpath(argv[1]);
  return 0;
}

#endif /* TEST */
