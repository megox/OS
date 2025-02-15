#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"



int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	da_Start = daStart;
	HARD_LIMIT = daLimit;
	brk = daStart + ROUNDUP( initSizeToAllocate , PAGE_SIZE);
	uint32 pages_to_alloc = ROUNDUP( initSizeToAllocate , PAGE_SIZE) / PAGE_SIZE;
	if(brk > daLimit || LIST_SIZE(&free_frame_list) < pages_to_alloc || daLimit > KERNEL_HEAP_MAX) return E_NO_MEM;
	uint32 va = KERNEL_HEAP_START;

	//allocate and map the mapped region
	for(int i = 0 ; i < pages_to_alloc ; i++){
	    struct FrameInfo * ptr = NULL;
	    int ret = allocate_frame(&ptr);
	    map_frame(ptr_page_directory , ptr ,va, PERM_WRITEABLE );
		va = va + PAGE_SIZE;
	}
	initialize_dynamic_allocator(daStart , ROUNDUP( initSizeToAllocate , PAGE_SIZE));
	return 0;
}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	// increment > 0
	if(increment > 0){
	  uint32 new_brk = ROUNDUP(brk + increment , PAGE_SIZE);
      if(new_brk <= HARD_LIMIT){
    	  uint32 ret = brk;
    	  uint32 new_va=ROUNDUP(brk,PAGE_SIZE);
    	  while(new_va<new_brk){
    		  struct FrameInfo * ptr = NULL;
    		  int ret = allocate_frame(&ptr);
    		  map_frame(ptr_page_directory , ptr ,new_va, PERM_WRITEABLE);
    		  new_va+=PAGE_SIZE;
    	  }
    	  brk = new_brk;
    	  return (void *)ret;
      }
      else{
    	  panic("NO MEM !");
      }
	}
	else if(increment < 0){
		if(brk + increment < KERNEL_HEAP_START)  panic("can not decrees the brk with this amount !");
		uint32 pages_to_dealloc = ( -1 * increment) / PAGE_SIZE;
		uint32 new_brk=brk+increment;
	uint32 new_va=ROUNDUP(new_brk,PAGE_SIZE);
	while(new_va<brk){
		 unmap_frame(ptr_page_directory , new_va);
		 new_va+=PAGE_SIZE;
	}

		brk = new_brk;
		return (void *)brk;
	}
	else{
		return (void *)brk;
	}

 return NULL;
}


void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	uint32 ret = -1;
	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		return alloc_block_FF(size);
	}
	else{
			 if(isKHeapPlacementStrategyFIRSTFIT()){
			 uint32 pages_to_alloc = ROUNDUP( size , PAGE_SIZE) / PAGE_SIZE;
			 uint32 counter = 0;
			 uint32 va = HARD_LIMIT + PAGE_SIZE;
             if(LIST_SIZE(&free_frame_list) < pages_to_alloc) panic("no mem !");

			 while(va != KERNEL_HEAP_MAX){
				 uint32* page_table_ret = NULL;
				 uint32  start_va_to_map;
				 struct FrameInfo* return_frame = get_frame_info(ptr_page_directory,va,&page_table_ret);
				 if(return_frame == NULL){
					 counter++;
					 if(counter == 1) start_va_to_map = va;
					 if(counter == pages_to_alloc){
						 ret = start_va_to_map;
						 for(int i = 0 ; i < counter ;i++){
							struct FrameInfo * ptr = NULL;
							int r = allocate_frame(&ptr);
							map_frame(ptr_page_directory , ptr ,start_va_to_map, PERM_WRITEABLE);
							if(i==0){
								ptr->size = pages_to_alloc;
							}else{
								ptr->size = -1;
							}

							start_va_to_map += PAGE_SIZE;
						 }
						 return (void *)ret;
					 }
				 }
				 else{
					counter = 0;
				 }

				 va = va + PAGE_SIZE;
			 }
		 }
	}
	return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()

	uint32 va = (uint32)virtual_address;
	uint32* page_table_ret = NULL;
    struct FrameInfo* return_frame = get_frame_info(ptr_page_directory,va,&page_table_ret);

    if(return_frame!=NULL){

    	if(virtual_address >= (void *)KERNEL_HEAP_START&&
    	   virtual_address <=(void *) brk){
    		free_block(virtual_address);

		}
		else if (virtual_address >=(void *) (HARD_LIMIT + PAGE_SIZE)&&
				 virtual_address <=(void *) KERNEL_HEAP_MAX){

			uint32 frames_to_free = return_frame->size;
			return_frame->size = -1;
			for(int i = 0;i<frames_to_free;i++){
				unmap_frame(ptr_page_directory,va);
				va+=PAGE_SIZE;
			}
		}
		else{
			panic("kfree() invalid virtual address !!");
		}

    }
    else{
    	panic("kfree() invalid virtual address !!");
    }

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
//	panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================


	uint32 offest = (uint32) physical_address % PAGE_SIZE;
	struct FrameInfo *ptr = to_frame_info((uint32)physical_address);

	if(ptr != NULL && ptr->va != 0)
	{
		return ptr->va + offest;
	}

	//change this "return" according to your answer
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	uint32 *ptr_page_table = NULL;
	uint32 va = (uint32)virtual_address % PAGE_SIZE;
	struct FrameInfo *ptr = get_frame_info(ptr_page_directory , (uint32)virtual_address , &ptr_page_table );
	if(ptr != NULL)
	{
		return (uint32)to_physical_address(ptr) + va;
	}
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().


void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	uint32 new_pages_to_allocate = ROUNDUP(new_size , PAGE_SIZE)/PAGE_SIZE;
	uint32 new_va = -1;
	if(new_size == 0)
	{
		kfree(virtual_address);
		return NULL;
	}

	uint32 vir_add = (uint32) virtual_address;
	if(virtual_address == NULL)
	{
		return kmalloc(new_size);
	}

	if(vir_add  >= da_Start && vir_add  <= brk)
	{
		if(new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			return realloc_block_FF(virtual_address , new_size);
		}else {
			kfree(virtual_address);
			return kmalloc(new_size);
		}
	}else if(vir_add  >= HARD_LIMIT + PAGE_SIZE
			&& vir_add <= KERNEL_HEAP_MAX)
	{
		uint32 * ptr_page_table = NULL;
		struct FrameInfo *ptr = get_frame_info(ptr_page_directory
				,vir_add ,&ptr_page_table);
		uint32 old_pages = ptr->size;

		if(new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			kfree(virtual_address);
			return kmalloc(new_size);
		}else if(old_pages > new_pages_to_allocate) {

			uint32 diff = old_pages - new_pages_to_allocate;

			new_va = vir_add + (new_pages_to_allocate+1)
					* PAGE_SIZE;

			ptr->size = new_pages_to_allocate;

			for(int i = 0; i<diff ; i++)
			{
				unmap_frame(ptr_page_directory , new_va);
				new_va+=PAGE_SIZE;
			}
			return virtual_address;
		}else if(old_pages < new_pages_to_allocate){
			uint32 diff = new_pages_to_allocate - old_pages;

			new_va = vir_add + (old_pages+1)
					* PAGE_SIZE;
			uint32 cnt = 0;
			uint32 start_va = new_va;
			while(new_va != KERNEL_HEAP_MAX)
			{
				uint32 * page_table=NULL;
				struct FrameInfo *ptr = get_frame_info(ptr_page_directory
						,new_va,&page_table);

				if(ptr == NULL)
				{
					cnt++;
				}
				else {
					kfree(virtual_address);
					return kmalloc(new_size);
				}

				if(cnt == diff)
				{
					ptr->size = new_pages_to_allocate;
					for(int i = 0; i < diff ; i++)
					{
						struct FrameInfo * ptr2 = NULL;
						int r = allocate_frame(&ptr2);
						map_frame(ptr_page_directory , ptr2
								,start_va, PERM_WRITEABLE);
						start_va += PAGE_SIZE;
					}
					return virtual_address;
				}
				new_va += PAGE_SIZE;
			}

			kfree(virtual_address);
			return kmalloc(new_size);
		}
	}
	return NULL;
//	panic("krealloc() is not implemented yet...!!");
}


















