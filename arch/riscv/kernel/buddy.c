#include "buddy.h"

extern char* _end;
struct buddy buddy_system;

void init_buddy_system(void){
    buddy_system.size = (unsigned int)((PHY_END - PHY_START)/PAGE_SIZE);
//    printf("buddy size: %u\n", buddy_system.size);
    int index = 2;
    unsigned int size = buddy_system.size;
    
    for(int i=0; i<buddy_system.size*2 - 1; i++){
    	if(i == index-1){
    	    index *= 2;
    	    size /= 2;
    	}
    	buddy_system.bitmap[i] = size;
    }
    
//    for(int i=0; i<buddy_system.size*2 - 1; i+=10){
//    	printf("[%d]: %u\n", i, buddy_system.bitmap[i]);
//    }
    
    
    //将kernel代码部分标记为已分配(PHY_START ~ _end)
    int page_num = ((uint64)VA2PA(&_end) - PHY_START) / PAGE_SIZE;
//    printf("kernel size:%d\n", page_num);
    void* p = alloc_pages(page_num);
    
}

void *alloc_pages(int npages){
    int page_size = 1;
    while(page_size < npages){
    	page_size *= 2;
    }

    void* addr;
    int index = 0;
    
//    printf("page_size:%d\n", page_size);
//    index = searchforson(page_size);
    for(index=0; index < buddy_system.size*2 - 1; index++){
       if((buddy_system.bitmap[index] == page_size) && ((index+1)*buddy_system.bitmap[index] >= buddy_system.size)){
//	   printf("index: %d\n", index);
           buddy_system.bitmap[index] = 0;
           int i = (index+1)/2 - 1;
           int pre_i  = index;
           int pre_i_sibling = (index%2 == 0)?(index-1):(index+1);
           int son1, son2;
           while(i >= 0){
           	if(buddy_system.bitmap[i] == 0) break;
           	son1 = buddy_system.bitmap[pre_i];
           	son2 = buddy_system.bitmap[pre_i_sibling];
           	buddy_system.bitmap[i] = (son1 < son2)?son2:son1;
           	pre_i = i;
           	pre_i_sibling = (i%2 == 0)?(i-1):(i+1);
           	i = (i+1)/2 - 1;
           }
           if(i >= 0)	continue;
           break;
       }
    }
    
    
    if(index >= buddy_system.size*2 - 1 || index < 0){
    	panic("Run out of free physic pages");
    }

    //根据index和page_size计算出地址
    int same_level_start_index = buddy_system.size/page_size - 1;
    addr = PHY_START + PAGE_SIZE * (index - same_level_start_index) * page_size;
    
    printf("[S] Buddy allocate addr: 0x%lx, page_num: %d, size: %d\n", addr, (long int)(addr-PHY_START)/PAGE_SIZE, page_size);
    return (void*)PA2VA(addr);
}

void free_pages(void* addr){
    int index = ((long int)addr - PHY_START)/PAGE_SIZE + buddy_system.size - 1;
    
    while(index >= 0){
        if(buddy_system.bitmap[index] == 0){
            int value = 1;
	    while(value * (index+1) < buddy_system.size){
	    	value *= 2;
	    }
	    buddy_system.bitmap[index] = value;
	    int i = (index+1)/2 - 1;
	    int pre_i  = index;
	    int pre_i_sibling = (index%2 == 0)?(index-1):(index+1);
	    int son1, son2;
	    while(i >= 0){
           	son1 = buddy_system.bitmap[pre_i];
           	son2 = buddy_system.bitmap[pre_i_sibling];
           	buddy_system.bitmap[i] = (son1 < son2)?son2:son1;
           	pre_i = i;
           	pre_i_sibling = (i%2 == 0)?(i-1):(i+1);
           	i = (i+1)/2 - 1;
           }
           break;
        }
        if(index%2 != 0)
            panic("Invalid physic page address to free");
    	index = (index+1)/2 -1;
    }

    if(index < 0){
    	panic("Invalid physic page address to free");
    }
}
