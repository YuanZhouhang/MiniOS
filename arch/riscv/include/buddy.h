#ifndef _BUDDY_H
#define _BUDDY_H

#include "string.h"
#include "stdio.h"
#include "types.h"
#include "rinux.h"

struct buddy {
  unsigned long size;
  unsigned bitmap[8200]; 
};

void init_buddy_system(void);
void *alloc_pages(int);
void free_pages(void*);

#endif
