#include "stdio.h"
#include "sched.h"
#include "syscall.h"
#define uLL unsigned long long

void do_page_fault(size_t scause, size_t sepc, uintptr_t* regs){
    printf("PAGE_FAULT: scause: %d, sepc: 0x%lx, badaddr: \n", (int)scause, sepc);
    panic("page fault\n");


}
