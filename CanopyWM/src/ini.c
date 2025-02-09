#include "ini.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 1000
#define MAX_SECTION 50
#define MAX_NAME 50

static char* rstrip(char* s) {
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

static char* lskip(const char* s) {
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

static char* find_char_or_comment(const char* s, char c) {
    while (*s && *s != c && *s != ';' && *s != '#')
        s++;
    return (char*)s;
}

static int ini_parse_stream(FILE* file, ini_handler handler, void* user) {
    char line[MAX_LINE];
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";
    int lineno = 0;

    while (fgets(line, MAX_LINE, file) != NULL) {
        char* start;
        char* end;
        char* name;
        char* value;
        lineno++;

        start = lskip(rstrip(line));
        if (*start == ';' || *start == '#' || *start == '\0')
            continue;

        if (*start == '[') {
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy(section, start + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
                *prev_name = '\0';
            }
            continue;
        }

        end = find_char_or_comment(start, '=');
        if (*end != '=')
            continue;

        name = rstrip(start);
        value = lskip(end + 1);
        end = find_char_or_comment(value, '\0');
        if (*end == ';' || *end == '#')
            *end = '\0';
        rstrip(value);

        if (handler(user, section, name, value) < 0)
            return 0;
    }
    return 1;
}

int ini_parse(const char* filename, ini_handler handler, void* user) {
    FILE* file = fopen(filename, "r");
    if (!file)
        return -1;
    int error = ini_parse_stream(file, handler, user);
    fclose(file);
    return error;
}
