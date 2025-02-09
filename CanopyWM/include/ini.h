#ifndef INI_H
#define INI_H

/* Simple INI parser */
typedef int (*ini_handler)(void* user, const char* section,
                          const char* name, const char* value);

int ini_parse(const char* filename, ini_handler handler, void* user);
int ini_parse_string(const char* string, ini_handler handler, void* user);

#endif
