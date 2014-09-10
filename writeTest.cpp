// write out to mmapped file.
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <cstdlib>
#include <stdio.h>
#include "mkpath.h"
#include <string>
#include <string.h>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


/// Open the filename for reading, truncating it if necessary. 
/// Create any necessary directories.
int openFile(const char* filename) {
  if ( mkpathto(filename) ) {
    printf("openFile: error in mkpathto.\n");
    return -1;
  }
  int fd = open(filename,  O_RDWR | O_CREAT | O_TRUNC, 0777);
  if (fd < 0) {
    printf("openFile: error opening %s.\n", filename);
  }
  return fd;
}

/// Time writing a 100 character string to a file via mmap and write.
/// 
/// Print the results as a tab-separated line: total_bytes mmap_time write_time.
/// Proceeds as follows:
/// For mmap and then for write:
///   1. Start timer.
///   2. For each page: 
///      - open the file, 
///      - write the string to the file strings_in_pages times,
///      - close the file.
///   3. Stop the timer.
/// Print the results: total_bytes_written total_time_mmap total_time_write.
/// Total bytes written for each procedure is string_length * strings_in_pages * total_pages.
int main(int argc, char** argv){
  if (argc < 5) {
    printf("Usage: %s output_file total_pages strings_in_pages string_to_write\n", argv[0]);
    return 1;
  }

  std::string filename = argv[1], 
    output_mmap = filename + ".mmap", output_write = filename + ".write";

  int pages = atoi(argv[2]);
  int strings_in_a_page = atoi(argv[3]);
  std::string str; // the rest of the arguments form the string
  for (int i= 3; i < argc; i++) str += argv[i];

  const char* s = str.c_str();
  int s_size = strlen(s);
  int page_size = s_size * strings_in_a_page;

  clock_t startTime, endTime, clockTicksTaken; 
  double timeInSeconds_mmap, timeInSeconds_write;

  /////////////////// MMAP
  // from http://stackoverflow.com/a/3220493/268040
  startTime = clock(); 
  for (int page=0; page < pages; page++) {
    int fd = openFile(output_mmap.c_str());
    if (lseek (fd, page*page_size + page_size - 1, SEEK_SET) == -1) {
      printf("mmap: lseek error for page %d\n", page);
      return 1;
    }
    if (::write (fd, "", 1) != 1) {
      printf("mmap: write error");
      return 1;
    }
    // mmap it
    /* The offset for mmap() must be page aligned. This means the
       offset may have some lower bits zeroed out (pa_offset). The
       difference between the offset and the pa_offset is extra_length
       which must be accounted for when specifying the length of the
       memory block to be mmapped, the starting position when writing
       to the mmapped memory, and when unmapping the memory. */
    off_t offset = page*page_size;
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    off_t extra_length = offset - pa_offset;

    char *buf = (char*) mmap (0, page_size + extra_length,
			      PROT_READ | PROT_WRITE, MAP_SHARED, 
			      fd, pa_offset);
    if (buf == (caddr_t) -1) { 
      printf("mmap: mmap error");
      return 1;
    }
    for (int i=0; i < strings_in_a_page; i++) {
      memcpy(buf+extra_length+i*s_size, s, s_size);
    }
    if (munmap(buf, page_size + extra_length)) {
      printf("mmap: munmap error for page %d\n", page);
      printf("page size %ld\n", sysconf(_SC_PAGE_SIZE));
      printf("page %d\n", page);
      printf("page_size %d\n", page_size);
      printf("offset %ld\n", offset);
      printf("pa_offset %ld\n", pa_offset);
      printf("extra_length %ld\n", extra_length);
      return 1;
    }
    close(fd);
  }

  endTime = clock();
  clockTicksTaken = endTime - startTime;
  timeInSeconds_mmap = clockTicksTaken / (double) CLOCKS_PER_SEC;
  // printf("MMAP: %f seconds\n", timeInSeconds);


  /////////////////// WRITE
  // same as above only now we just write instead if mmap
  startTime = clock(); 
  for (int page=0; page < pages; page++) {
    int fd = openFile(output_write.c_str());
    if (lseek (fd, page*page_size, SEEK_SET) == -1) {
      printf("write: lseek error for page %d\n", page);
      return 1;
    }
    for (int i=0; i < strings_in_a_page; i++) {
      write(fd, s, s_size);
    }
    close(fd);
  }

  endTime = clock();
  clockTicksTaken = endTime - startTime;
  timeInSeconds_write = clockTicksTaken / (double) CLOCKS_PER_SEC;
  // printf("write: %f seconds\n", timeInSeconds);
  printf("%d\t%f\t%f\n", pages*page_size, timeInSeconds_mmap, timeInSeconds_write);

}
