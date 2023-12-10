#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
  if(FirstTimeFlag)
  {
#if UHP_USE_BUDDY
    initialize_buddy();
    cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
    FirstTimeFlag = 0;
  }
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
  return (void*) sys_sbrk(increment);
}


//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

char mark[200000];//mego_o
void* malloc(uint32 size)
{
  //DON'T CHANGE THIS CODE========================================
  InitializeUHeap();
  if (size == 0) return NULL ;
  //==============================================================
  //TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
  // Write your code here, remove the panic and write your code
  uint32 HARD_LIMIT = sys_get_hard_limit();
  void * ret = NULL;
  if(sys_isUHeapPlacementStrategyFIRSTFIT()){
	    if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
	       return (void *) alloc_block_FF(size);
	    }
	    else{
	     uint32 pages_to_alloc = ROUNDUP( size , PAGE_SIZE) / PAGE_SIZE;
	     uint32 counter = 0;
	     uint32 va = sys_get_hard_limit() + PAGE_SIZE;
	     uint32 off = sys_get_hard_limit() + PAGE_SIZE;
	     uint32  start_va_to_mark;
	      while(va != USER_HEAP_MAX){
	         int r = myEnv->mark[(va-off) / PAGE_SIZE];
	         if(r!=1 && r!=5){
	           counter++;
	           if(counter == 1) start_va_to_mark = va;
	           if(counter == pages_to_alloc){
	             uint32 virtual_address = start_va_to_mark;
	             for(int i=0 ; i<counter ;i++){
	              myEnv->mark[(virtual_address-off) / PAGE_SIZE] = 1;
	              if(i == counter - 1) myEnv->mark[(virtual_address-off) / PAGE_SIZE] = 5;//(5) is to mark the last page of block
	              virtual_address+=PAGE_SIZE;
	             }
	             ret = (void *)start_va_to_mark;
	             sys_allocate_user_mem(start_va_to_mark , PAGE_SIZE * pages_to_alloc);
	             return (void *)ret;
	           }
	          }
	          else{
	          counter = 0;
	          }
	          va = va + PAGE_SIZE;
	       }
	       return(void*) NULL;
	    }
  }
  return (void*)NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address){
//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
    uint32 off = sys_get_hard_limit() + PAGE_SIZE;
    if(virtual_address >= (void *)USER_HEAP_START &&
       virtual_address <= (void *) sys_get_hard_limit()){
        free_block(virtual_address);
    }
    else if (virtual_address >=(void *) (sys_get_hard_limit() + PAGE_SIZE)&&
             virtual_address <=(void *) USER_HEAP_MAX){
      uint32 block_counter = 0;
	  uint32 va = (uint32)virtual_address;
	  if(myEnv->mark[(va-off)/PAGE_SIZE]!=1 && myEnv->mark[(va-off)/PAGE_SIZE]!=5) panic("userfree() invalid virtual address !!");
	  while(va!=USER_HEAP_MAX){
		  if(myEnv->mark[(va-off)/PAGE_SIZE]==5){
			  block_counter++;
			  myEnv->mark[(va-off)/PAGE_SIZE] = 0;
			  break;
		  }
		  else{
			  block_counter++;
			  myEnv->mark[(va-off)/PAGE_SIZE] = 0;
		  }
        va+=PAGE_SIZE;
      }
      sys_free_user_mem((uint32)virtual_address,block_counter*PAGE_SIZE);
    }
    else{
      panic("userfree() invalid virtual address !!");
    }
}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
  //==============================================================
  //DON'T CHANGE THIS CODE========================================
  InitializeUHeap();
  if (size == 0) return NULL ;
  //==============================================================
  panic("smalloc() is not implemented yet...!!");
  return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
  //==============================================================
  //DON'T CHANGE THIS CODE========================================
  InitializeUHeap();
  //==============================================================
  // Write your code here, remove the panic and write your code
  panic("sget() is not implemented yet...!!");
  return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
  //==============================================================
  //DON'T CHANGE THIS CODE========================================
  InitializeUHeap();
  //==============================================================

  // Write your code here, remove the panic and write your code
  if(virtual_address == NULL)
  {
	  return malloc(new_size);
  }
  if(new_size == 0)
  {
	  free(virtual_address);
	  return NULL;
  }

  if(virtual_address >= (void *)USER_HEAP_START
    && virtual_address<=(void *) myEnv->user_brk)
  {
	  if(new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
	  {
		  return realloc_block_FF(virtual_address , new_size);
	  }else
	  {
		  free(virtual_address);
		  return malloc(new_size);
	  }
  }else if(virtual_address >= (void *)(sys_get_hard_limit() + PAGE_SIZE)
		  && virtual_address <= (void *)USER_HEAP_MAX)
  {
	  uint32 start_va = (uint32) virtual_address;
	  uint32 base = sys_get_hard_limit() + PAGE_SIZE;
	  uint32 old_pages = 0;

	  while(start_va != USER_HEAP_MAX)
	  {
		  if(myEnv->mark[(start_va  - base) / PAGE_SIZE] == 5)
		  {
			  old_pages++;
			  break;
		  }
		  old_pages++;
		  start_va+=PAGE_SIZE;
	  }

	  uint32 new_pages_alloc = ROUNDUP(new_size , PAGE_SIZE)/PAGE_SIZE;
	  if(new_pages_alloc < old_pages)
	  {
		  uint32 diff = old_pages - new_pages_alloc;
		  for(int i = 0;i<diff ;i++)
		  {
			  myEnv->mark[(start_va - base) / PAGE_SIZE] =  0;
			  start_va -= PAGE_SIZE;
		  }
		  myEnv->mark[(start_va - base) / PAGE_SIZE] = 5;

		  return virtual_address;
	  }else if(new_pages_alloc > old_pages)
	  {
		  uint32 diff =  new_pages_alloc - old_pages;
		  uint32 cnt = 0;
		  uint32 temp_start = start_va;
		  for(int i = 0;i<diff ;i++)
		  {
			  if(myEnv->mark[(start_va - base) / PAGE_SIZE] != 1
					  &&myEnv->mark[(start_va - base) / PAGE_SIZE] != 5)
			  {
				  cnt++;
			  }
			  start_va += PAGE_SIZE;
		  }
		  if(cnt == diff)
		  {
			  for(int i = 0;i<diff ;i++)
			  {
				  myEnv->mark[(start_va - base) / PAGE_SIZE] = 1;
				  if(i == diff -1)
				  {
					  myEnv->mark[(start_va - base) / PAGE_SIZE] = 5;
				  }
				  temp_start += PAGE_SIZE;
			  }
			  return virtual_address;
		  }
		  else
		  {
			  free(virtual_address);
			  return malloc(new_size);
		  }
	  }
  }
  return NULL;
  //  panic("realloc() is not implemented yet...!!");

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
  // Write your code here, remove the panic and write your code
  panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
  panic("Not Implemented");

}
void shrink(uint32 newSize)
{
  panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
  panic("Not Implemented");

}
