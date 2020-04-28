#ifndef STDIO_H
#define STDIO_H

#include <types.h>
#include <stdarg.h>
#include <stddef.h>

#define BUFSIZ 512
#define _IOFBF 0x0000
#define _IOLBF 0x0040
#define _IONBF 0x0004
#define EOF (-1)

#define FILENAME_MAX 260
#define FOPEN_MAX 20

#define L_tmpnam 13

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define TMP_MAX 32767

// MIN-GW implementation
typedef struct _iobuf
{
    char *_ptr;
    int _cnt;
    char *_base;
    int _flag;
    int _file;
    int _charbuf;
    int _bufsiz;
    char *_tmpfname;
} FILE;

extern FILE stds[3];
#define stderr &stds[0]
#define stdin &stds[1]
#define stdout &stds[2]

typedef u64 fpos_t;

// Closes stream. All buffers are flushed.
// extern int fclose(FILE *stream);

// Clears the end-of-file and error indicators of stream
// extern void clearerr(FILE *stream);

// Tests the EOF indicator for the given stream
// extern int feof(FILE *stream);

// Test error indicator for the given stream
// extern int ferror(FILE *stream);

// Flushes the output buffer of the given stream
// extern int fflush(FILE *stream);

// Gets the current file position of the stream and writes it to pos
// extern int fgetpos(FILE *stream, fpos_t *pos);

// Opens the file name pointed to by filename using the given mode
// extern FILE *fopen(const char *filename, const char *mode);

// Reads data from the given stream into the array pointed to by ptr.
// extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

// Associates a new filename with the given open stream and same time closing the old file in stream.
// extern FILE *freopen(const char *filename, const char *mode, FILE *stream);

// Sets the file position of the stream to the given offset. The argument offset signifies the number of bytes to seek from the given whence position.
// extern int fseek(FILE *stream, long int offset, int whence);

// Sets the file position of the given stream to the given position. The argument pos is a position given by the function fgetpos.
// extern int fsetpos(FILE *stream, const fpos_t *pos);

// Returns the current file position of the given stream.
// extern long int ftell(FILE *stream);

// Writes data from the array pointed to by ptr to the given stream.
// extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

// Deletes the given filename so that it is no longer accessible.
// extern int remove(const char *filename);

// Renames file
extern int rename(const char *old_filename, const char *new_filename);

// Sets the file position to the beginning of the file
// extern void rewind(FILE *stream);

// Defines how a stream should be buffered
// extern void setbuf(FILE *stream, char *buffer);

// Defines how a stream should be buffered
// extern int setvbuf(FILE *stream, char *buffer, int mode, size_t size);

// Creates a temporary file in binary update mode (wb+)
// extern FILE *tmpfile(void);

// Generates and returns a valid temporary filename which does not exist
// extern char *tmpnam(char *str);

// Sends formatted output to a stream
extern int fprintf(FILE *stream, const char *fmt, ...);

// Sends formatted output to stdout
extern int printf(const char *fmt, ...);

// Sends formatted output to a string
extern int sprintf(char *str, const char *fmt, ...);

// Sends formatted output to a stream using va_list
extern int vfprintf(FILE *stream, const char *fmt, va_list args);

// Sends formatted output to stdout using va_list
extern int vprintf(const char *fmt, va_list args);
#define vprintf(fmt, args) vfprintf(stdout, fmt, args)

// Sends formatted output to a string using va_list
extern int vsprintf(char *str, const char *fmt, va_list args);

// Reads formatted input from a stream
extern int fscanf(FILE *stream, const char *fmt, ...);
// Reads formatted input from stdin
extern int scanf(const char *fmt, ...);
// Reads formatted input from a string
extern int sscanf(char *str, const char *fmt, ...);

// Writes a character to stream
extern int putc(int c, FILE *stream);
extern int fputc(int c, FILE *stream);
// Writes a character to stdout
extern int putchar(int c);
#define fputc(c, s) putc(c, s)
#define putchar(c) putc(c, stdout)

// Writes a null-terminated string to the stream
extern int fputs(const char *str, FILE *stream);
extern int puts(const char *str);
#define puts(str) fputs(str, stdout)

// Gets next character from stream
extern int getc(FILE *stream);
extern int fgetc(FILE *stream);
#define fgetc(s) getc(s)
// Gets character from stdin
extern int getchar(void);
#define getchar() getc(stdin)

// Reads line from specified stream and stores it in a string.
// Stops when either n-1 characters are read, newline is read or EOF is reached,whichever comes first
extern char *fgets(char *str, int n, FILE *stream);
extern char *gets(char *str);

// Pushes the character char onto stream so that the next character is read
// int ungetc(int c, FILE *stream);

// Prints descriptive error message to stderr. First the string str is printed followed by a colon and then a space.
void perror(const char *str);

#endif