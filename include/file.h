#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <stdlib.h>


char *read_file(const char *filename);
void free_file_content(char *content);
#endif // __FILE_H__