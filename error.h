#ifndef ERROR_H
#define ERROR_H
struct CFile;
void error_at(struct CFile* cfile, char* loc, const char* fmt, ...);
void error(const char* fmt, ...);
#endif