/* fs.h
 * Filesystem adapter.
 */
 
#ifndef FS_H
#define FS_H

/* Read a regular file in one shot.
 * On success, caller frees (*dstpp), even if reported length is zero.
 */
int file_read(void *dstpp,const char *path);

/* Write a regular file in one shot, clobbering anything already there.
 * File is unlinked if write fails.
 */
int file_write(const char *path,const void *src,int srcc);

/* Equivalent to (file_read,file_write) but on stdin and stdout.
 * Writing does not close the stream.
 */
int stdin_read(void *dstpp);
int stdout_write(const void *src,int srcc);

/* Call (cb) synchronously for each file under directory (path).
 * Stops when you return nonzero, and returns the same.
 * (ftype) is zero if the OS doesn't provide it; use file_get_type() if you need to.
 */
int dir_read(const char *path,int (*cb)(const char *path,const char *base,char ftype,void *userdata),void *userdata);

/* Returns one of:
 *  0: Error eg not found.
 *  'f': Regular file.
 *  'd': Directory.
 *  '?': Something else.
 */
char file_get_type(const char *path);

/* '/' or '\\'.
 */
extern const char path_separator;

/* Basically `dst = a + path_separator + b`.
 */
int path_join(char *dst,int dsta,const char *a,int ac,const char *b,int bc);

/* Returns position of the final separator, not counting trailing separators.
 * <0 if there isn't one.
 */
int path_split(const char *src,int srcc);

#endif
