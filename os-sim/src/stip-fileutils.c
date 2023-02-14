#include "stip-fileutils.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
// #include <stdio.h>

void stip_fileutils_write_to_file(gchar *insert, gchar *filename, int open_flags)
{
  int fd = open(filename, open_flags, S_IRWXU);
  if (fd < 0)
  {
    g_message("%s: Could not open file, errno:%d, msg:%s", __func__, errno, strerror(errno));
  }
  
  gchar *data_to_write = g_strconcat (insert, "\n", NULL);
  
  int rv = write(fd, data_to_write, strlen(data_to_write));
  if (rv < 0)
  {
    g_message("%s: Could not write to file, errno:%d, msg:%s", __func__, errno, strerror(errno));
  }
  
  g_free(data_to_write);
  close(fd);
}
