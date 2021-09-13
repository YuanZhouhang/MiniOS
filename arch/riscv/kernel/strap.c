#include "stdio.h"
#include "sched.h"
#include "syscall.h"
#define uLL unsigned long long

int count = 0;
/*void handler_s(uLL cause, uLL epc )
{
	if (cause >> 63) {		// interrupt
		if ( ( (cause << 1) >> 1 ) == 5 ) {	// supervisor timer interrupt
			asm volatile("ecall");
			do_timer();
		}
	}
	return;
}*/

uintptr_t sys_write(unsigned int fd, const char* buf, size_t count){
//	printf("in sys_write");
	int i;
	for(i=0; i<count; i++){
//	printf("%d, %lx\n",i, *(buf+i));
	//	if(*(buf+i) == '\0')	break;
	//	printf("in loop");
		//	break;
	//	printf("%c", *(buf+i));
	//	puts(buf);
		putchar(*(buf+i));
	}
	return i;

//	uintptr_t num = (uintptr_t)printf("%s", buf);
//	return num;
}

extern struct task_struct *current;
uintptr_t sys_getpid(){
	return (uintptr_t)current->pid;
}

void handler_s(size_t scause, size_t sepc, uintptr_t* regs){
//	printf("In handler_s, scause:%lx, sepc:%lx\n", scause, sepc);
	asm volatile ("add t1, zero, zero");
	if (scause >> 63) {		// interrupt
		
		if ( ( (scause << 1) >> 1 ) == 5 ) {	// supervisor timer interrupt
			asm volatile("ecall");
			do_timer();
		}
		return;
	}else if(scause >> 63 == 0){	//exception
		if(scause == 8){
			asm volatile("	li t0, 0x40000 \n \
					csrs sstatus, t0");
		//	printf("ecall from U in handler.\n");	
			size_t a7 = (size_t)*(regs + 17);
			size_t a0;
		//	a7 = 64;
		//	printf("a7:%lx\n", a7);
			if(a7 == 64){
			//	printf("a7 is 64\n");
			//	printf("a0, a1, a2: %lx, %lx, %lx", (size_t)*(regs + 10),(size_t)*(regs + 11),(size_t)*(regs + 12));
				unsigned int fd = (unsigned int)*(regs + 10);
				const char* buf = (char*)*(regs + 11);
				size_t count = (size_t)*(regs + 12);
			//	printf("%s", buf);
			//	unsigned int fd = 1;
			//	const char* buf = "[User] pid: ";
			//	size_t count = 12;
				a0 = sys_write(fd, buf, count);
			}else if(a7 == 172){
				a0 = sys_getpid();
			//	printf("[User] pid:%d\n", a0);
			}
			asm volatile ("addi t1, zero, 4");
			asm volatile ("add a0, zero, %0" :: "rK"(a0));
			return;
		}else if(scause == 12 || scause == 13 || scause == 15){
			do_page_fault(scause, sepc, regs);
		}else{
			printf("Unkonw page fault on 0x%lx.\n", sepc);
		}
	}	
}

void output(){
	puts("in output.\n");
}
