#include "serial.h"
#include <string.h>
#include <limits.h>

/* Iterate lines of text.
 */
 
int sr_decode_lines(
  struct sr_decoder *decoder,
  int (*cb)(const char *src,int srcc,int lineno,void *userdata),
  void *userdata,
  int flags
) {
  int lineno=0;
  while (decoder->p<decoder->c) {
  
    // Read to EOF or thru the next LF.
    const char *line=(char*)decoder->v+decoder->p;
    int linec=0;
    while (decoder->p<decoder->c) {
      decoder->p++;
      if (line[linec++]==0x0a) break;
    }
    lineno++;
    
    // If they asked for comment removal, do it.
    if (flags&SR_DECODE_LINES_STRIP_COMMENT) {
      int i=0;
      for (;i<linec;i++) if (line[i]=='#') linec=i;
    }
    
    // Trim fore and aft if they asked.
    if (flags&SR_DECODE_LINES_STRIP_TAIL) {
      while (linec&&((unsigned char)line[linec-1]<=0x20)) linec--;
    }
    if (flags&SR_DECODE_LINES_STRIP_HEAD) {
      while (linec&&((unsigned char)line[0]<=0x20)) { line++; linec--; }
    }
    
    // If it's empty and they haven't asked to include empties, skip it.
    if (!linec&&!(flags&SR_DECODE_LINES_INCLUDE_EMPTY)) {
      continue;
    }
    
    // Show the caller.
    int err=cb(line,linec,lineno,userdata);
    if (err) return err;
  }
  return 0;
}
