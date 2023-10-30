/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================

	if (initSizeOfAllocatedSpace == 0)
		return ;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()

	struct BlockMetaData *head = (struct BlockMetaData *)daStart;
	head->size = initSizeOfAllocatedSpace;
LIST_INIT(&block_list);
	head->is_free = (uint8)1;
	LIST_INSERT_HEAD(&block_list , head);

//	print_blocks_list((block_list));

//	panic("initialize_dynamic_allocator is not implemented yet");
}
//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{

	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
		    if(size==0)
				return NULL;
			uint32 required_size = size + sizeOfMetaData();
			struct BlockMetaData* blk;
			LIST_FOREACH(blk, &block_list)
			{
				if((blk)->is_free == 1 && blk->size == required_size){
					blk->is_free = 0;
					return (struct BlockMetaData*)((void*)blk + sizeOfMetaData());
				}
				else if(blk->is_free == 1 && blk->size > required_size){

					if(blk->size -required_size>=sizeOfMetaData()){
							struct BlockMetaData * new_block = (void*)blk +required_size;
									  new_block->size = blk->size - required_size;
										    new_block->is_free = 1;
										    blk->size = required_size;
										    blk->is_free = 0;
											LIST_INSERT_AFTER(&block_list,blk,new_block);


							}
							else{
								blk->is_free = 0;

							}
							return(struct BlockMetaData*) ((void*)blk+sizeOfMetaData());
				}
			}

			// there is not enough space
			struct BlockMetaData * tail = LIST_LAST(&block_list);
			void* ret;
			uint32 new_block_address = 0;
		    if(tail->is_free == 0){
				ret = sbrk(required_size);
				if(ret== (void *)-1){
					return NULL;
				}
				else{
					struct BlockMetaData * new_block = ret;
					LIST_INSERT_AFTER(&block_list,tail,new_block);
					return(struct BlockMetaData*)( (void*)ret + sizeOfMetaData());
				}
			}
			else{
				ret = sbrk(required_size - tail->size);
				if(ret == (void *)-1){
					return NULL;
				}
			    else{
			    	tail->size=required_size;
			    	tail->is_free=0;
					return (struct BlockMetaData*)((void*)tail + sizeOfMetaData());
				}
			}
}


void *alloc_block_BF(uint32 size)
{
	//cprintf("the size that you want to allock : %d\n",size);
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	 if(size==0)
				return NULL;
	 uint32 min_size=0,count=0;
	 uint8 available_blk=0;
			uint32 required_size = size + sizeOfMetaData();
			struct BlockMetaData* blk;
			struct BlockMetaData* blk_with_best_size;

	LIST_FOREACH(blk, &block_list)
	{

		if((blk)->is_free == 1){

			 if (blk->size >= required_size){
					available_blk=1;
					if(min_size==0){
					min_size=blk->size;
					blk_with_best_size=blk;
					}
					else if (blk->size< min_size){
					min_size=blk->size;
			    	blk_with_best_size=blk;
					}

			}


		}

	}
	if(available_blk==1){
		if(blk_with_best_size->size -required_size>=sizeOfMetaData()){
		struct BlockMetaData * new_block = (void*)blk_with_best_size +required_size;
				  new_block->size = blk_with_best_size->size - required_size;
					    new_block->is_free = 1;
					    blk_with_best_size->size = required_size;
					    blk_with_best_size->is_free = 0;
						LIST_INSERT_AFTER(&block_list,blk_with_best_size,new_block);


		}
		else{
			 blk_with_best_size->is_free = 0;

		}
		return(struct BlockMetaData*) ((void*)blk_with_best_size+sizeOfMetaData());
	}


	else{

	  struct BlockMetaData * tail = LIST_LAST(&block_list);
		void* ret;
		uint32 new_block_address = 0;

		if(tail->is_free == 0){
				ret = sbrk(required_size);
			if(ret== (void *)-1){

						return NULL;
			 }
			else{
				struct BlockMetaData * new_block = (struct BlockMetaData*)ret;
						LIST_INSERT_AFTER(&block_list,tail,new_block);
					return(struct BlockMetaData*)( (void*)ret + sizeOfMetaData());
				}
		}
		else{
			ret = sbrk(required_size - tail->size);
				if(ret == (void *)-1){

					return NULL;
				}
			   else{
					tail->size=required_size;
						tail->is_free=0;
					return (struct BlockMetaData*)((void*)tail+ sizeOfMetaData());
				}
		}

	}

}
//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	if(va !=NULL){
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
		struct BlockMetaData* blk=(void*)va-sizeOfMetaData();
	//	cprintf("%x",blk);
		struct BlockMetaData* prevblk=	LIST_PREV(blk);
		struct BlockMetaData* nextblk=	LIST_NEXT(blk);
		if(blk==LIST_FIRST(&block_list)){
			if((nextblk)->is_free==1)
			{
				(blk)->is_free=1;
				(blk)->size+=(nextblk)->size;
				nextblk->size=0;
				nextblk->is_free=0;
				LIST_REMOVE(&block_list, nextblk);
				nextblk=0;
			}
			else{

				(blk)->is_free=1;
			}

		}
		else if (blk==LIST_LAST(&block_list)){
			if((prevblk)->is_free==1){
		    	(prevblk)->is_free=1;
				(prevblk)->size+=(blk)->size;
				blk->size=0;
				blk->is_free=0;
				LIST_REMOVE(&block_list, blk);
		 		blk=0;
		    }
		    else{
				(blk)->is_free=1;
		    }
		}
		else{
		if((prevblk)->is_free==1)
		{
			if((nextblk)->is_free==1)
			{
				(prevblk)->is_free=1;
				(prevblk)->size+=(blk)->size + (nextblk)->size;
				blk->size=0;
				blk->is_free=0;
				nextblk->size=0;
				nextblk->is_free=0;
				LIST_REMOVE(&block_list, blk);
				LIST_REMOVE(&block_list, nextblk);
				blk=0;
				nextblk=0;
			}
			else{
			(prevblk)->size+=(blk)->size;
			(prevblk)->is_free=1;
			blk->size=0;
			blk->is_free=0;
			LIST_REMOVE(&block_list, blk);
			blk=0;}
		}

		else if((nextblk)->is_free==1 &&(prevblk)->is_free==0)
		{
			(blk)->is_free=1;
			(blk)->size+= (nextblk)->size;
			nextblk->size=0;
			nextblk->is_free=0;
			LIST_REMOVE(&block_list,nextblk);
			nextblk=0;

		}
		else if((prevblk)->is_free==0&&(nextblk)->is_free==0)
		(blk)->is_free=1;
	}
  }
}


//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	//special cases

	if((void*)va == NULL){
		return (void *)alloc_block_FF(new_size);
	}else if(new_size == 0){
		free_block((void*)va);
		return (void *)NULL;
	}
	new_size+= sizeOfMetaData();
	// (regard that new size is not include meta_data && address is of the free space without meta_data) mego___o
	struct BlockMetaData* blk = (void*)va - sizeOfMetaData();
	struct BlockMetaData * tail = LIST_LAST(&block_list);

	if((uint32)new_size > (uint32)blk->size){
		uint32 required_size_to_expend = new_size - blk->size; //?
		if(blk!=tail){
			struct BlockMetaData* next_blk = LIST_NEXT((struct BlockMetaData*)blk); //////the next element ?
			   if(next_blk->is_free==1){
				//the _blk is not the tail and we have a free block in front of our block
				//that need to be expended , so we check if this free block fit the
				//(required_size_to_expend) or not.
				if(next_blk->size == required_size_to_expend){
					blk->size = new_size;
					next_blk->size = 0;
					next_blk->is_free=0;
					LIST_REMOVE(&block_list , next_blk); //remove the next block we have use
					return (struct BlockMetaData*)((void*)blk + sizeOfMetaData()) ;  // still in same address
				}
				else if (next_blk->size > required_size_to_expend){
					uint32 next_blk_new_size = next_blk->size - required_size_to_expend;
					//remove the next block we have use and create new one with the free space
					struct BlockMetaData* new_next_blk = (void *)next_blk + required_size_to_expend;
					new_next_blk->size = next_blk_new_size;
					new_next_blk->is_free = 1;
					next_blk->size = 0;
					next_blk->is_free=0;
					LIST_REMOVE(&block_list , next_blk);
					LIST_INSERT_AFTER(&block_list , blk ,new_next_blk);
					blk->size += (uint32) required_size_to_expend;
					return (struct BlockMetaData*)((void*)blk + sizeOfMetaData()) ;  // still in same address
				}
				else{
					void * ret = alloc_block_FF(new_size-sizeOfMetaData());
					if(ret == (void*)-1){
						return (void *)-1;//no suitable block
					}else{
						free_block((void*)va);
						return (struct BlockMetaData*)ret; //the new allocated _blk
					}
				}
			}
		}
		else if(blk==tail){
			void * ret = sbrk(required_size_to_expend);
			if(ret == (void*)-1){
				return (void *)-1;//no size in heap
			}else{
				return (struct BlockMetaData*)((void*)tail + sizeOfMetaData()) ;
			}
		}
	}
	else if((uint32)new_size < (uint32)blk->size){
		uint32 new_blk_size = blk->size - new_size;
		blk->size = new_size;
		struct BlockMetaData * new_block = (void*)blk + new_size;
	    new_block->size = new_blk_size;
		new_block->is_free = 1;
		LIST_INSERT_AFTER(&block_list,blk,new_block);
		return (struct BlockMetaData*)((void*)blk + sizeOfMetaData()) ;  // still in same address
	}
	else{
		return (struct BlockMetaData*)((void*)blk + sizeOfMetaData()) ; // nothing to happen
	}

	return NULL;
}
