#pragma once

#include "rinux.h"
#include "types.h"

typedef uint64 pte_t;
typedef uint64* pagetable_t;

void kvminit();
void kvmmap(pagetable_t pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
pagetable_t user_kvminit();

