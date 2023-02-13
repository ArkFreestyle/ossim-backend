#ifndef _STIP_FILEUTILS_H_
#define _STIP_FILEUTILS_H_

#include <glib.h>

void stip_write_to_file(gchar* data_to_write, gchar* filename, int open_flags);

#endif