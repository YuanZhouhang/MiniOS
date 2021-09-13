.section .text.entry

.global trap_s
.extern handler_s
.extern output
.equ reg_size, 0x8
.align 2

trap_s:
//	la t0, init_stack_top
//	li t1, 1
//	slli t1, t1, 12
//	sub t0, t0, t1
//	csrw sscratch, t0

	# To set sscratch
	csrrw sp, sscratch, sp
//	la sp, init_stack_top
//	csrr t0, scause
//	addi t0, t0, -8
//	bne t0, zero, not_ecall_U
//	csrr sp, sscratch

	# save 32 registers and spec
	addi sp, sp, -33*reg_size
	sd x1, 1*reg_size(sp)
	sd x2, 2*reg_size(sp)
	sd x3, 3*reg_size(sp)
	sd x4, 4*reg_size(sp)
	sd x5, 5*reg_size(sp)
	sd x6, 6*reg_size(sp)
	sd x7, 7*reg_size(sp)
	sd x8, 8*reg_size(sp)
	sd x9, 9*reg_size(sp)
	sd x10, 10*reg_size(sp)
	sd x11, 11*reg_size(sp)
	sd x12, 12*reg_size(sp)
	sd x13, 13*reg_size(sp)
	sd x14, 14*reg_size(sp)
	sd x15, 15*reg_size(sp)
	sd x16, 16*reg_size(sp)
	sd x17, 17*reg_size(sp)
	sd x18, 18*reg_size(sp)
	sd x19, 19*reg_size(sp)
	sd x20, 20*reg_size(sp)
	sd x21, 21*reg_size(sp)
	sd x22, 22*reg_size(sp)
	sd x23, 23*reg_size(sp)
	sd x24, 24*reg_size(sp)
	sd x25, 25*reg_size(sp)
	sd x26, 26*reg_size(sp)
	sd x27, 27*reg_size(sp)
	sd x28, 28*reg_size(sp)
	sd x29, 29*reg_size(sp)
	sd x30, 30*reg_size(sp)
	sd x31, 31*reg_size(sp)

	csrr t0, sepc
	sd t0, 32*reg_size(sp)

//	call output

	# call handler
	csrr a0, scause
	csrr a1, sepc
	add a2, zero, sp
	call handler_s

	# load sepc and 32 registers
	ld t0, 32*reg_size(sp)
	add t0, t0, t1
	csrw sepc, t0
	
//	sd x10, a0
	ld x1, 1*reg_size(sp)
	ld x2, 2*reg_size(sp)
	ld x3, 3*reg_size(sp)
	ld x4, 4*reg_size(sp)
	ld x5, 5*reg_size(sp)
	ld x6, 6*reg_size(sp)
	ld x7, 7*reg_size(sp)
	ld x8, 8*reg_size(sp)
	ld x9, 9*reg_size(sp)
//	ld x10, 10*reg_size(sp)
	ld x11, 11*reg_size(sp)
	ld x12, 12*reg_size(sp)
	ld x13, 13*reg_size(sp)
	ld x14, 14*reg_size(sp)
	ld x15, 15*reg_size(sp)
	ld x16, 16*reg_size(sp)
	ld x17, 17*reg_size(sp)
	ld x18, 18*reg_size(sp)
	ld x19, 19*reg_size(sp)
	ld x20, 20*reg_size(sp)
	ld x21, 21*reg_size(sp)
	ld x22, 22*reg_size(sp)
	ld x23, 23*reg_size(sp)
	ld x24, 24*reg_size(sp)
	ld x25, 25*reg_size(sp)
	ld x26, 26*reg_size(sp)
	ld x27, 27*reg_size(sp)
	ld x28, 28*reg_size(sp)
	ld x29, 29*reg_size(sp)
	ld x30, 30*reg_size(sp)
	ld x31, 31*reg_size(sp)
	addi sp, sp, 33*reg_size
	csrrw sp, sscratch, sp
	sret


/*
 * Integer register context switch
 * The callee-saved registers must be saved and restored.
 *
 *   a0: previous task_struct (must be preserved across the switch)
 *   a1: next task_struct
 *
 * The value of a0 and a1 must be preserved by this function, as that's how
 * arguments are passed to schedule_tail.
 */
 .globl __switch_to
__switch_to:
	/* Save context into prev->thread */
	li    a4,  40
	add   a3, a0, a4
	add   a4, a1, a4
//	sd ra,  0*reg_size(a3)
	sd sp,  1*reg_size(a3)
	sd s0,  2*reg_size(a3)
	sd s1,  3*reg_size(a3)
	sd s2,  4*reg_size(a3)
	sd s3,  5*reg_size(a3)
	sd s4,  6*reg_size(a3)
	sd s5,  7*reg_size(a3)
	sd s6,  8*reg_size(a3)
	sd s7,  9*reg_size(a3)
	sd s8,  10*reg_size(a3)
	sd s9,  11*reg_size(a3)
	sd s10, 12*reg_size(a3)
	sd s11, 13*reg_size(a3)
	
	csrr t0, sscratch
	sd t0, 15*reg_size(a3)
	ld t0, 15*reg_size(a4)
	csrw sscratch, t0
	
//	csrr t0, satp
//	sd t0, 14*reg_size(a3)
//	ld t0, 14*reg_size(a4)
//	csrw satp, t0

//	li t0, 0x100
//	csrs sstatus, t0

	/* Restore context from next->thread */
    ld ra,  0*reg_size(a4)
	ld sp,  1*reg_size(a4)
	ld s0,  2*reg_size(a4)
	ld s1,  3*reg_size(a4)
	ld s2,  4*reg_size(a4)
	ld s3,  5*reg_size(a4)
	ld s4,  6*reg_size(a4)
	ld s5,  7*reg_size(a4)
	ld s6,  8*reg_size(a4)
	ld s7,  9*reg_size(a4)
	ld s8,  10*reg_size(a4)
	ld s9,  11*reg_size(a4)
	ld s10, 12*reg_size(a4)
	ld s11, 13*reg_size(a4)

	ret


 	.globl __sret
__sret:
	call switch_to_user
	li t0, 0x100
	csrc sstatus, t0
	csrrw sp, sscratch, sp
//	li sp, 0x07000000
	add t0, zero, zero
	csrw sepc, t0
	sret
