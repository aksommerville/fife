#include "serial.h"
#include <string.h>
#include <limits.h>

/* Evaluate integer.
 */
 
int sr_int_eval(int *dst,const char *src,int srcc) {
  if (!src) return -1;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!srcc) return -1;
  int srcp=0,positive=1,base=10,overflow=0;
  
  if (src[srcp]=='-') {
    positive=0;
    if (++srcp>=srcc) return -1;
  } else if (src[srcp]=='+') {
    if (++srcp>=srcc) return -1;
  }
  
  if ((srcp<=srcc-3)&&(src[srcp]=='0')) switch (src[srcp+1]) {
    case 'b': case 'B': base=2; srcp+=2; break;
    case 'o': case 'O': base=8; srcp+=2; break;
    case 'd': case 'D': base=10; srcp+=2; break;
    case 'x': case 'X': base=16; srcp+=2; break;
  }
  
  *dst=0;
  int limit=positive?(UINT_MAX/base):(INT_MIN/base);
  for (;srcp<srcc;srcp++) {
    int digit=sr_digit_eval(src[srcp]);
    if ((digit<0)||(digit>=base)) return -1;
    if (positive) {
      if ((unsigned int)(*dst)>limit) overflow=1;
      (*dst)*=base;
      if ((unsigned int)(*dst)>UINT_MAX-digit) overflow=1;
      (*dst)+=digit;
    } else {
      if (*dst<limit) overflow=1;
      (*dst)*=base;
      if (*dst<INT_MIN+digit) overflow=1;
      (*dst)-=digit;
    }
  }
  
  if (overflow) return 0;
  if (positive&&(*dst<0)) return 1;
  return 2;
}

/* Represent decimal integer.
 */

int sr_decsint_repr(char *dst,int dsta,int src) {
  int dstc;
  if (src<0) {
    dstc=2;
    int limit=-10;
    while (src<=limit) { dstc++; if (limit<INT_MIN/10) break; limit*=10; }
    if (dstc>dsta) return dstc;
    int i=dstc;
    for (;i-->0;src/=10) dst[i]='0'-src%10;
    dst[0]='-';
  } else {
    dstc=1;
    int limit=10;
    while (src>=limit) { dstc++; if (limit>INT_MAX/10) break; limit*=10; }
    if (dstc>dsta) return dstc;
    int i=dstc;
    for (;i-->0;src/=10) dst[i]='0'+src%10;
  }
  if (dstc<dsta) dst[dstc]=0;
  return dstc;
}

int sr_decuint_repr(char *dst,int dsta,int src) {
  int dstc=1;
  unsigned int limit=10;
  while ((unsigned int)src>=limit) { dstc++; if ((unsigned int)limit>UINT_MAX/10) break; limit*=10; }
  if (dstc>dsta) return dstc;
  int i=dstc;
  for (;i-->0;src=(unsigned int)src/10) dst[i]='0'+(unsigned int)src%10;
  if (dstc<dsta) dst[dstc]=0;
  return dstc;
}
