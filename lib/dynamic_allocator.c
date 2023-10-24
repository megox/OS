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
	head->is_free = (uint8)1;
	LIST_INSERT_HEAD(&block_list , head);

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
					//char* new_address=(char*)blk+ sizeOfMetaData();
				//	char* nd=(char*)blk + required_size;
					struct BlockMetaData * new_block = (void*)blk +required_size;
				    new_block->size = blk->size - required_size;
				    new_block->is_free = 1;
					blk->size = required_size;
					blk->is_free = 0;
					LIST_INSERT_AFTER(&block_list,blk,new_block);
					return(struct BlockMetaData*) ((void*)blk+sizeOfMetaData()  );
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
					return(struct BlockMetaData*)( (void*)ret + sizeOfMetaData());
				}
			}
			else{
				ret = sbrk(required_size - tail->size);
				if(ret == (void *)-1){
					return NULL;
				}
			    else{
					return (struct BlockMetaData*)((void*)tail + sizeOfMetaData());
				}
			}
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	panic("alloc_block_BF is not implemented yet");
	return NULL;
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
	//panic("free_block is not implemented yet");



//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	panic("realloc_block_FF is not implemented yet");
	return NULL;
}
