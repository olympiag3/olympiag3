#include "dirent.h"

#include <stdlib.h>
#include <string.h>
#include <io.h>

DIR *
opendir(const char *name)
{
  DIR * direct = malloc(sizeof(DIR));

  direct->first = 1;
//  strncpy(direct->name, name, sizeof(direct->name));
  strncpy_s(direct->name, sizeof(direct->name), name, sizeof(direct->name));
  return direct;
}

struct dirent *
readdir(DIR * thedir)
{
  static struct _finddata_t ft; /* STATIC_RESULT: used for return, not across calls */
  static struct dirent de; /* STATIC_RESULT: used for return, not across calls */
  char where[_MAX_PATH];

  strcat(strcpy(where, thedir->name), "/*");
  if (thedir->first) {
    thedir->first = 0;
    de.hnd = _findfirst(where, &ft);
  } else {
    if (_findnext(de.hnd, &ft) != 0)
      return NULL;
  }
  _splitpath(ft.name, de.d_drive, de.d_dir, de.d_name, de.d_ext);
  return &de;
}

int closedir(DIR * thedir)
{
  free(thedir);
  return 0;
}
