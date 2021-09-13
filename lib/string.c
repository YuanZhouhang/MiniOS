#include "string.h"

void* memset(void *dst, int c, uint n) {
  char *cdst = (char *) dst;
  for(int i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

void* memcpy(void *dst, const void *src, uint num){

	char* pdst =(char*) dst;
	char* psrc = (char*)src;
	while (num--)
	{
		*pdst++ = *psrc++;
	}
	return dst;


}
