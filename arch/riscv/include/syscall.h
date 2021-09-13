#ifndef SYSCALL_H
#define SYSCALL_H

#include "sched.h"
typedef unsigned long int	uintptr_t;

uintptr_t sys_write(unsigned int fd, const char* buf, size_t count);
uintptr_t sys_getpid();

void handler_s(size_t scause, size_t sepc, uintptr_t* regs);

void do_page_fault(size_t scause, size_t sepc, uintptr_t* regs);

#endif
