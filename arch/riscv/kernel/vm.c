#include "vm.h"
#include "mm.h"
#include "device.h"
#include "symbol.h"

pagetable_t kpgtbl;
//pagetable_t upgtbl;

pagetable_t user_kvminit(){
//  upgtbl = 0x080ff2000;
  kmalloc(PAGE_SIZE);
  pagetable_t upgtbl = (pagetable_t) kmalloc(PAGE_SIZE);
//	  printf("upgtbl: %lx\n", upgtbl);
  memset(upgtbl, 0, PAGE_SIZE);
    
  //将内核页表复制到用户态页表（重做）
  // map devices
  uint64 uart = PA2VA(get_device_addr(UART_MMIO));
  kvmmap(upgtbl, uart, VA2PA(uart), get_device_size(UART_MMIO), (PTE_R | PTE_W));

  uint64 poweroff = PA2VA(get_device_addr(POWEROFF_MMIO));
  kvmmap(upgtbl, poweroff, VA2PA(poweroff), get_device_size(POWEROFF_MMIO), (PTE_R | PTE_W));

  // map kernel text executable and read-only.
  kvmmap(upgtbl, (uint64)&text_start, VA2PA((uint64)&text_start), (uint64)&text_end - (uint64)&text_start, (PTE_R | PTE_X));
  // map kernel data and the physical RAM we'll make use of.
  kvmmap(upgtbl, (uint64)&rodata_start, VA2PA((uint64)&rodata_start), (uint64)&rodata_end - (uint64)&rodata_start, (PTE_R));
  kvmmap(upgtbl, (uint64)&data_start, VA2PA((uint64)&data_start), (uint64)PHY_END - VA2PA((uint64)&data_start), (PTE_R | PTE_W));

//  memset(upgtbl, 0, PAGE_SIZE);
//  memcpy(upgtbl, kpgtbl, PAGE_SIZE * 13);	
  //给用户态程序构建页表
  kvmmap(upgtbl, 0x0, 0x084000000, 0x00020000, (PTE_W | PTE_R | PTE_X | PTE_U));
  
  return upgtbl;
//  write_csr(satp, MAKE_SATP(kpgtbl));
//  asm volatile("sfence.vma");
}

void kvminit() {
  kpgtbl = (pagetable_t) kmalloc(PAGE_SIZE);
  memset(kpgtbl, 0, PAGE_SIZE);
//	printf("kpgtbl:%lx\n", kpgtbl);
    
  // map devices
  uint64 uart = PA2VA(get_device_addr(UART_MMIO));
  kvmmap(kpgtbl, uart, VA2PA(uart), get_device_size(UART_MMIO), (PTE_R | PTE_W));
//  printf("finish first kvmmap, size:%d\n", get_device_size(UART_MMIO));

  uint64 poweroff = PA2VA(get_device_addr(POWEROFF_MMIO));
  kvmmap(kpgtbl, poweroff, VA2PA(poweroff), get_device_size(POWEROFF_MMIO), (PTE_R | PTE_W));
//  printf("finish second kvmmap, size:%d\n", get_device_size(POWEROFF_MMIO));

  // map kernel text executable and read-only.
  kvmmap(kpgtbl, (uint64)&text_start, VA2PA((uint64)&text_start), (uint64)&text_end - (uint64)&text_start, (PTE_R | PTE_X));
//  printf("finish third kvmmap, size:%d\n", (uint64)&text_end - (uint64)&text_start);
  // map kernel data and the physical RAM we'll make use of.
  kvmmap(kpgtbl, (uint64)&rodata_start, VA2PA((uint64)&rodata_start), (uint64)&rodata_end - (uint64)&rodata_start, (PTE_R));
//  printf("finish 4th kvmmap, size:%d\n", (uint64)&rodata_end - (uint64)&rodata_start);
  kvmmap(kpgtbl, (uint64)&data_start, VA2PA((uint64)&data_start), (uint64)PHY_END - VA2PA((uint64)&data_start), (PTE_R | PTE_W));
//  printf("finish 5th kvmmap, size:%d\n", PAGE_SIZE);
  // map kernel stacks
//  write_csr(satp, MAKE_SATP(kpgtbl));
//  asm volatile("sfence.vma");
}

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
pte_t * walk(pagetable_t pagetable, uint64 va, int alloc, uint64 size)
{
  for(int level = 2; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if(*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if(!alloc || (pagetable = (pagetable_t)kmalloc(PAGE_SIZE)) == 0)
        return 0;
     //   printf("pagetable:%lx\n", pagetable);
//      if(size >= PAGE_SIZE)	
//      		memset(PA2VA(pagetable), 0, size);
//      else	
      		memset(pagetable, 0, PAGE_SIZE);
      
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }
  return &pagetable[PX(0, va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;

  a = PAGE_DOWN(va);
 // a = va;
  last = PAGE_DOWN(va + size - 1);
  for(;;){
    if((pte = walk(pagetable, a, PAGE_ALLOC, size)) == 0)
      return -1;
    if(*pte & PTE_V){
      panic("remap");
    }
      
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += PAGE_SIZE;
    pa += PAGE_SIZE;
  }
  return 0;
}

// add a mapping to the kernel page table.
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
  if(mappages(kpgtbl, va, sz, pa, perm) != 0)
    panic("kvmmap");
}
