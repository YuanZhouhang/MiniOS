#include "stdio.h"
#include "device.h"
#include "rand.h"
#include "sched.h"
#include "mm.h"
#include "vm.h"

union task_union {
    struct task_struct task;
    char stack[PAGE_SIZE];
};

extern pagetable_t kpgtbl;

struct task_struct* current;
struct task_struct* task[NR_TASKS];
size_t sstatus;

extern void __sret(void);

#ifdef SJF
int task_init_done = 0;
void task_init(void) {
    puts("task init...\n");

    current = (struct task_struct*) alloc_pages(1);
    current->state = TASK_RUNNING;
    current->counter = 0;
    current->priority = 5;
    current->blocked = 0;
    current->pid = 0;
    
    current->sscratch = (unsigned long long)alloc_pages(1);
    current->pgtbl = user_kvminit();
    current->sscratch = (unsigned long long)alloc_pages(1) + PAGE_SIZE;
    memset(current->sscratch, 0, PAGE_SIZE);
    kvmmap(current->pgtbl, (uint64)0xffffffdf80000000-PAGE_SIZE, VA2PA(current->sscratch - PAGE_SIZE), (uint64)PAGE_SIZE, (PTE_R | PTE_W | PTE_X | PTE_U));
    current->sscratch = (unsigned long long)0xffffffdf80000000;
//    printf("current->pgtbl:%lx\n", current->pgtbl);

    task[0] = current;
    task[0]->thread.sp = (unsigned long long) task[0] + TASK_SIZE;


    /* set other 4 tasks */
    for (int i = 1; i <= LAB_TEST_NUM; ++i) {
        task[i] = (struct task_struct*) alloc_pages(1);
        task[i]->state = TASK_RUNNING;
        task[i]->priority = 5; // All tasks initialized with the default priority (lowest)
        task[i]->counter = rand(); // 
        task[i]->blocked = PREEMPT_ENABLE;
        task[i]->thread.ra = (unsigned long long) & __sret;
        task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
//        task[i]->thread.sp = (unsigned long long)alloc_page() + PAGE_SIZE;
//	memset(task[i]->thread.sp, 0, PAGE_SIZE);
//    kvmmap(current->pgtbl, (unsigned long long) task[i] + PAGE_SIZE, VA2PA(task[i]->thread.sp + PAGE_SIZE), (uint64)PAGE_SIZE, (PTE_R | PTE_W | PTE_X));
//    	task[i]->thread.sp = (unsigned long long) task[i] - PAGE_SIZE;
        task[i]->pid = i;
        task[i]->pgtbl = user_kvminit();
    //    printf("task->pgtbl:%lx.\n", (unsigned long long)task[i]->pgtbl);
        task[i]->sscratch = (unsigned long long)alloc_pages(1) + PAGE_SIZE;
        memset(task[i]->sscratch, 0, PAGE_SIZE);
        kvmmap(task[i]->pgtbl, (uint64)0xffffffdf80000000-PAGE_SIZE, VA2PA(task[i]->sscratch - PAGE_SIZE), (uint64)PAGE_SIZE, (PTE_R | PTE_W | PTE_X | PTE_U));
        task[i]->sscratch = (unsigned long long)0xffffffdf80000000;
    //    printf("task stack top:%lx.\n", task[i]->sscratch);
        printf("[PID = %d] Process Create Successfully! counter = %d\n", task[i]->pid, task[i]->counter);
    }
    
//    printf("finish task init\n");
    
    asm volatile ("csrr %0, sstatus" : "=r"(sstatus));
    write_csr(satp, MAKE_SATP(kpgtbl));
    asm volatile("sfence.vma");
    
//    printf("finish task init\n");
    
//    asm volatile("	la t0, init_stack_top \n \
	li t1, 0xffffffdf80000000 \n \
	add t0, t0, t1 \n \
	csrw sscratch, t0");
    task_init_done = 1;
}



void do_timer(void) {
    if (!task_init_done)return;

    printf("[PID = %d] Context Calculation: counter = %d\n", current->pid, current->counter);

    /* 当前进程的时间片减1 */
    (current->counter)--;
    /* 如果当前进程时间片已经使用完 则进行进程调度 */
    if ((current->counter) <= 0) {
        current->counter = 0;
        schedule();
    }
    return;
}



void schedule(void) {
    long cmin;
    unsigned char selector;
    unsigned char next;
    struct task_struct** ptr;

    while (1) {
        cmin = 0xFFFF;
        selector = NR_TASKS;
        ptr = &task[NR_TASKS];

        while (--selector) {
            if (!*--ptr || (*ptr)->counter == 0)
                continue;
            if ((*ptr)->state == TASK_RUNNING && (*ptr)->counter < cmin) {
                cmin = (*ptr)->counter;
                next = selector;
                // puti(next);
                // puti(cmin);
            }
        }

        if (cmin != 0xFFFF) {
            break;
        }
        else {
            // 如果task0～4都运行完毕，则重新生成新的不同运行时间长度的任务
            for (int i = 1; i <= LAB_TEST_NUM; ++i) {
                task[i]->counter = rand();
                task[i]->sscratch = (unsigned long long)0xffffffdf80000000;
                task[i]->thread.sp = (unsigned long long) task[i] + TASK_SIZE;
                printf("[PID = %d] Reset counter = %d\n", i, task[i]->counter);
            }
        }

    }


    printf("[!] Switch from task %d [task struct: 0x%lx, sp: 0x%lx] to task %d [task struct: 0x%lx, sp: 0x%lx], prio: %d, counter: %d\n", 
        current->pid, (unsigned long)current, current->thread.sp,
        task[next]->pid, (unsigned long)task[next], task[next]->thread.sp,
        task[next]->priority, task[next]->counter);

    switch_to(task[next]);
}

#endif

#ifdef PRIORITY

int task_init_done = 0;
void task_init(void) {

    current = (struct task_struct*) alloc_page();
    current->state = TASK_RUNNING;
    current->counter = 0;
    current->priority = 5;
    current->blocked = 0;
    current->pid = 0;

    task[0] = current;
    task[0]->thread.sp = (unsigned long long) task[0] + TASK_SIZE;

    /* set other 4 tasks */
    for (int i = 1; i <= LAB_TEST_NUM; ++i) {
        task[i] = (struct task_struct*) alloc_page();
        task[i]->state = TASK_RUNNING;
        task[i]->priority = 5; //All tasks initialized with the default priority (lowest)
        task[i]->counter = 8 - i; // counter = 7, 6, 5, 4（分别对应 task[1-4]运行时长）
        task[i]->blocked = PREEMPT_ENABLE;
        task[i]->thread.ra = (unsigned long long) & __sret;
        task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
        task[i]->pid = i;

        printf("[PID = %d] Process Create Successfully! counter = %d priority = %d\n", task[i]->pid, task[i]->counter, task[i]->priority);
    }
    task_init_done = 1;

}

void do_timer(void) {
    if (!task_init_done) return;
    /* 当前进程的时间片减1 */
    (current->counter)--;
    /* 如果当前进程时间片已经使用完，则从头运行该task */
    /* 当在do_timer中发现当前运行进程剩余运行时间为0（即当前进程运行结束）时，
       需重新为该进程分配其对应的运行时长。
       相当于重启当前进程（每个进程的运行时间长度和初始化的值一致）。
    */
    if ((current->counter) <= 0 && (current->pid) != 0) { // 排除task0，task0不参与后续的优先级调度
        current->counter = 8 - current->pid; // counter = 7, 6, 5, 4（分别对应 task[1-4]的运行时长）
    }
    /* 每次do_timer都进行一次抢占试优先级调度 */
    schedule();
    return;
}




void schedule(void) {
    long pmin;
    long cmin;
    unsigned char selector;
    unsigned char next;
    struct task_struct** ptr;

    while (1) {
        pmin = 5;
        cmin = 0xFFFF;
        next = 0;
        selector = NR_TASKS;
        ptr = &task[NR_TASKS];

        // 打印调度遍历所有进程之前，那一刻的所有进程状态
        // puts("current tasks' state: \n");
        // for(int i=1;i<=4;i++){
        //     puts("[PID = ");
        //     puti(task[i]->pid);
        //     puts("] counter = ");
        //     puti(task[i]->counter);
        //     puts(" priority = ");
        //     puti(task[i]->priority);
        //     puts("\n");
        // }

        while (--selector) {
            if (!*--ptr)
                continue;
            if ((*ptr)->state == TASK_RUNNING && (*ptr)->priority < pmin) {
                cmin = (*ptr)->counter;
                pmin = (*ptr)->priority;
                next = selector;
            }
            if ((*ptr)->state == TASK_RUNNING && (*ptr)->priority == pmin) {
                if ((*ptr)->counter < cmin) {
                    cmin = (*ptr)->counter;
                    next = selector;
                }
            }
        }

        if (next) break;

    }

    if ((current->pid) != next) {
        printf("[!] Switch from task %d [task struct: 0x%lx, sp: 0x%lx] to task %d [task struct: 0x%lx, sp: 0x%lx], prio: %d, counter: %d\n", 
        current->pid, (unsigned long)current, current->thread.sp,
        task[next]->pid, (unsigned long)task[next], task[next]->thread.sp,
        task[next]->priority, task[next]->counter);
    }
    // 动态调整task[1-4]的优先级
    for (int i = 1; i <= LAB_TEST_NUM; ++i) {
        task[i]->priority = rand();
    }

    puts("tasks' priority changed\n");
    for (int i = 1;i <= LAB_TEST_NUM;i++) {
        printf("[PID = %d] counter = %d priority = %d\n", task[i]->pid, task[i]->counter, task[i]->priority);
    }
    switch_to(task[next]);
}

#endif


void switch_to(struct task_struct* next) {
//    printf("switch_to\n");
    if (current == next) return;

    struct task_struct* prev = current;
    current = next;
//    prev->thread.sp = (unsigned long long)prev + TASK_SIZE;
    __switch_to(prev, next);
}


void switch_to_user(void)
{ 
    asm volatile ("csrw sstatus, %0" :: "rK"(sstatus)); 
 //   printf("switch_to_user\n");
    //se/c,sscratch,satp的保存和修改
    unsigned long long __tmp, __temp;
    asm volatile ("csrr %0, sepc" : "=r"(__tmp));
//    current->sepc = __tmp;
//    printf("save sepc:%x\n", current->sepc);
    
//    asm volatile ("csrr %0, satp" : "=r"(__tmp));
//    printf("switch satp:%x,", __tmp);
    __temp = MAKE_SATP(current->pgtbl);
//    printf("pgtbl:%x.\n", __temp);
    asm volatile ("csrw satp, %0" :: "rK"(__temp));
//    current->pgtbl = __tmp;
    
//    printf("finish switch_to_user.\n");
	
}

void back_from_user(void){
    printf("back_from_user.\n");

  /*  unsigned long __tmp, __temp;
//    read_csr(satp);
//    write(satp, current->mm->pgtbl);
    asm volatile ("csrr %0, satp" : "=r"(__tmp));
    __temp = *(current->pgtbl);
    asm volatile ("csrw satp, %0" :: "rK"(__temp));
    *(current->pgtbl) = __tmp;
    
//    read(sscratch);
    asm volatile ("csrr %0, sscratch" : "=r"(__tmp));
    asm volatile ("add sp, zero, %0" :: "rK"(__tmp));
    
//    write_csr(sepc, current->sepc);
    __temp = current->sepc;
    asm volatile ("csrw sepc, %0" :: "rK"(__temp));*/

}

