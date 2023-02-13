#include "stip-fileutils.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

void stip_write_to_file(gchar* data_to_write, gchar* filename, int open_flags)
{
//   int fd = open("alert-information.txt", O_CREAT | O_RDWR | O_APPEND, S_IRWXU);
  int fd = open(filename, open_flags, S_IRWXU);
  printf("fd: %d\n", fd);
  if (fd < 0)
    {
      printf("stip_write_alarm_to_file: Could not open, error: % d\n", errno);
      perror("perror: ");
    }
  
  int rv = write(fd, "\n", strlen("\n"));
  if (rv < 0)
  {
    printf("stip_write_alarm_to_file: Could not write, error: % d\n", errno);
    perror("perror: ");
  }
  
  rv = write(fd, (gchar*)data_to_write, strlen((gchar*)data_to_write));
  printf("data_to_write: %s\n", (char*)data_to_write);
  // int rv = write(fd, "hello world", strlen("hello world"));
  if (rv < 0)
  {
    printf("stip_write_alarm_to_file: Could not write, error: % d\n", errno);
    perror("perror: ");
  }
  
  close(fd);
}