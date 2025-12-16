#include "fs.h"
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

/* POSIX has a flag like this but MacOS doesn't respect it.
 * So we'll figure it out on our own. (TODO Figure it out on our own).
 */
#define DIRENT_HAS_D_TYPE 1

// O_BINARY is a Windows thing. If we don't have it, no worries, call it zero.
#ifndef O_BINARY
  #define O_BINARY 0
#endif

#if USE_mswin
  //TODO Windows headers.
  const char path_separator='\\';
#else
  //TODO Which of the headers above are Unix-only?
  const char path_separator='/';
#endif

/* Read regular file, one shot.
 */

int file_read(void *dstpp,const char *path) {
  if (!path) return -1;
  int fd=open(path,O_RDONLY|O_BINARY);
  if (fd<0) return -1;
  off_t flen=lseek(fd,0,SEEK_END);
  if ((flen<0)||(flen>INT_MAX)||lseek(fd,0,SEEK_SET)) {
    close(fd);
    return -1;
  }
  char *dst=malloc(flen?flen:1);
  if (!dst) {
    close(fd);
    return -1;
  }
  int dstc=0;
  while (dstc<flen) {
    int err=read(fd,dst+dstc,flen-dstc);
    if (err<=0) {
      close(fd);
      free(dst);
      return -1;
    }
    dstc+=err;
  }
  close(fd);
  *(void**)dstpp=dst;
  return dstc;
}

/* Write regular file, one shot.
 */

int file_write(const char *path,const void *src,int srcc) {
  if (!path||(srcc<0)||(srcc&&!src)) return -1;
  int fd=open(path,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0666);
  if (fd<0) return -1;
  int srcp=0;
  while (srcp<srcc) {
    int err=write(fd,(char*)src+srcp,srcc-srcp);
    if (err<=0) {
      close(fd);
      unlink(path);
      return -1;
    }
    srcp+=err;
  }
  close(fd);
  return 0;
}

/* Read from stdin.
 */

int stdin_read(void *dstpp) {
  int dstc=0,dsta=8192;
  char *dst=malloc(dsta);
  if (!dst) return -1;
  for (;;) {
    if (dstc>=dsta) {
      if (dsta>=0x40000000) {
        free(dst);
        return -1;
      }
      dsta<<=1;
      void *nv=realloc(dst,dsta);
      if (!nv) {
        free(dst);
        return -1;
      }
      dst=nv;
    }
    int err=read(STDIN_FILENO,dst+dstc,dsta-dstc);
    if (err<0) {
      free(dst);
      return -1;
    }
    if (!err) {
      *(void**)dstpp=dst;
      return dstc;
    }
    dstc+=err;
  }
}

/* Write to stdout.
 */
 
int stdout_write(const void *src,int srcc) {
  if ((srcc<0)||(srcc&&!src)) return -1;
  int srcp=0;
  while (srcp<srcc) {
    int err=write(STDOUT_FILENO,(char*)src+srcp,srcc-srcp);
    if (err<=0) return -1;
    srcp+=err;
  }
  return 0;
}

/* Iterate directory.
 */

int dir_read(const char *path,int (*cb)(const char *path,const char *base,char ftype,void *userdata),void *userdata) {
  if (!path||!cb) return -1;
  DIR *dir=opendir(path);
  if (!dir) return -1;
  char subpath[1024];
  int pathc=0;
  while (path[pathc]) pathc++;
  if (pathc>=sizeof(subpath)) {
    closedir(dir);
    return -1;
  }
  subpath[pathc++]=path_separator;
  struct dirent *de;
  while (de=readdir(dir)) {
    const char *base=de->d_name;
    int basec=0;
    while (base[basec]) basec++;
    
    if (pathc>=sizeof(subpath)-basec) {
      closedir(dir);
      return -1;
    }
    memcpy(subpath+pathc,base,basec+1); // +1 to copy the terminator
    
    char ftype=0;
    #if DIRENT_HAS_D_TYPE
      switch (de->d_type) {
        case DT_REG: ftype='f'; break;
        case DT_DIR: ftype='d'; break;
        default: ftype='?'; break;
      }
    #endif
    
    int err=cb(subpath,base,ftype,userdata);
    if (err) {
      closedir(dir);
      return err;
    }
  }
  closedir(dir);
  return 0;
}

/* Stat for file type.
 */

char file_get_type(const char *path) {
  if (!path) return 0;
  struct stat st={0};
  if (stat(path,&st)<0) return 0;
  if (S_ISREG(st.st_mode)) return 'f';
  if (S_ISDIR(st.st_mode)) return 'd';
  return '?';
}

/* Join path.
 */

int path_join(char *dst,int dsta,const char *a,int ac,const char *b,int bc) {
  if (!dst||(dsta<0)) dsta=0;
  if (!a) ac=0; else if (ac<0) { ac=0; while (a[ac]) ac++; }
  if (!b) bc=0; else if (bc<0) { bc=0; while (b[bc]) bc++; }
  
  // If one input is empty, our answer is the other, verbatim.
  if (!ac) {
    if (bc<=dsta) memcpy(dst,b,bc);
    if (bc<dsta) dst[bc]=0;
    return bc;
  }
  if (!bc) {
    if (ac<=dsta) memcpy(dst,a,ac);
    if (ac<dsta) dst[ac]=0;
    return ac;
  }
  
  // Strip trailing separators from (a) and leading from (b).
  // Note whether (a) starts with a separator.
  int absolute=((ac>=1)&&(a[0]==path_separator));
  while (ac&&(a[ac-1]==path_separator)) ac--;
  while (bc&&(b[0]==path_separator)) { b++; bc--; }
  
  // If (a) is now empty but was absolute, the answer is `path_separator + b`.
  if (!ac&&absolute) {
    int dstc=1+bc;
    if (dstc<=dsta) {
      dst[0]=path_separator;
      memcpy(dst+1,b,bc);
      if (dstc<dsta) dst[dstc]=0;
    }
    return dstc;
  }
  
  // If (b) is now empty, the answer is (a). What garbage is this that they're calling us with?
  if (!bc) {
    if (ac<=dsta) memcpy(dst,a,ac);
    if (ac<dsta) dst[ac]=0;
    return ac;
  }
  
  // The usual case: `a + path_separator + b`.
  if (ac>INT_MAX-bc) return -1;
  int dstc=ac+1+bc;
  if (dstc<=dsta) {
    memcpy(dst,a,ac);
    dst[ac]=path_separator;
    memcpy(dst+ac+1,b,bc);
    if (dstc<dsta) dst[dstc]=0;
  }
  return dstc;
}

/* Locate path separator.
 */

int path_split(const char *src,int srcc) {
  if (!src) return -1;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  while (srcc&&(src[srcc-1]==path_separator)) srcc--; // Trailing separators don't count.
  int i=srcc;
  while (i-->0) {
    if (src[i]==path_separator) return i;
  }
  return -1;
}
