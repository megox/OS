/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//uint32 page_size = 4 * 1024;
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

bool is_initialized = 0;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================

	if (initSizeOfAllocatedSpace == 0)
		return ;

	is_initialized = 1;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()

	struct BlockMetaData *head = (struct BlockMetaData *)daStart;
	head->size = initSizeOfAllocatedSpace;
    LIST_INIT(&block_list);
	head->is_free = (uint8)1;
	LIST_INSERT_HEAD(&block_list , head);
}
//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================


//void *alloc_block_FF(uint32 size)
//{
//	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
//		    if(size==0)
//				return NULL;
//
//		    if (!is_initialized)
//		    {
//				uint32 required_size = size + sizeOfMetaData();
//				uint32 da_start = (uint32)sbrk(required_size);
//				//get new break since it's page aligned! thus, the size can be more than the required one
//				uint32 da_break = (uint32)sbrk(0);
//				initialize_dynamic_allocator(da_start, da_break - da_start);
//		    }
//
//			uint32 required_size = size + sizeOfMetaData();
//			struct BlockMetaData* blk;
//			LIST_FOREACH(blk, &block_list)
//			{
//				if((blk)->is_free == 1 && blk->size == required_size){
//					blk->is_free = 0;
//					return (struct BlockMetaData*)((void*)blk + sizeOfMetaData());
//				}
//				else if(blk->is_free == 1 && blk->size > required_size){
//
//					if(blk->size -required_size>=sizeOfMetaData()){
//					struct BlockMetaData * new_block = (void*)blk +required_size;
//					new_block->size = blk->size - required_size;
//					new_block->is_free = 1;
//					blk->size = required_size;
//					blk->is_free = 0;
//					LIST_INSERT_AFTER(&block_list,blk,new_block);
//					}
//					else{
//						blk->is_free = 0;
//
//					}
//						return(struct BlockMetaData*) ((void*)blk+sizeOfMetaData());
//				}
//			}
//
//			// there is not enough space
//			struct BlockMetaData * tail = LIST_LAST(&block_list);
//			void* ret;
//			uint32 new_block_address = 0;
//			uint32 old_brk_address =(uint32)sbrk(0);ret = sbrk(required_size);
//
//			if(ret == (void *)-1){
//				return NULL;
//			}
//			else{
//				struct BlockMetaData * new_block = ret;
//				new_block->size = ((uint32)sbrk(0) - old_brk_address);
//				new_block->is_free = 0;
//				LIST_INSERT_AFTER(&block_list,tail,new_block);
//				return(struct BlockMetaData*)( (void*)ret + sizeOfMetaData());
//			}
//
//		    return NULL;
//}

void *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
	//panic("alloc_block_FF is not implemented yet");

	if(size==0){
		return NULL;
	}

	if (!is_initialized)
	{

		uint32 required_size = size + sizeOfMetaData();
		uint32 da_start = (uint32)sbrk(required_size);



		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break = (uint32)sbrk(0);


		initialize_dynamic_allocator(da_start, da_break - da_start);

	}
	struct BlockMetaData *block;

	LIST_FOREACH (block,&(block_list)){
		if(!(block->is_free)){
			continue;
		}
		else if(block->size-sizeOfMetaData()>=size){
			if(block->size-sizeOfMetaData()==size){
				block->is_free=(1==2);

				return (struct BlockMetaData *)(block+1);
			}
			else if(block->size-sizeOfMetaData()>(size+sizeOfMetaData())){
//	cprintf("here4\n");
				struct BlockMetaData *new_block;
				//assign new_block

				void* addres;
				addres=block;
				addres+=size+sizeOfMetaData();
				new_block=addres;
				new_block->size=block->size-(size+sizeOfMetaData());
				new_block->is_free=(1==1);
				//new_block->prev_next_info.le_next=block->prev_next_info.le_next;


				LIST_INSERT_AFTER(&(block_list),block,new_block);
				 //modify block
				block->is_free=(1==2);
				block->size=size+sizeOfMetaData();
//				cprintf("ptr=%p\n",(struct BlockMetaData *)(block+1));


				return (struct BlockMetaData *)(block+1);


			}
			else{
				block->is_free=(1==2);

				return(struct BlockMetaData *) (block+1);
			}

		}
		//cprintf("%x\n",block->prev_next_info.le_prev);
//		cprintf("%x\n",block);

//		cprintf("%x\n",LIST_LAST(&block_list)->prev_next_info);


	}
	struct BlockMetaData *sbrk_block=LIST_LAST(&(block_list));

//		 cprintf("herer 6\n");
				 uint32* x=sbrk(0);
				 void* check=sbrk(size+sizeOfMetaData());
				 		 if(check!=NULL){
				 			void* addres;
				 			addres=sbrk_block;
				 			addres+=sbrk_block->size;
				 			 struct BlockMetaData *dblock;
				 			 dblock = addres;
				 			 dblock->is_free = 0;
				 			 dblock->size=size+sizeOfMetaData();
				 			LIST_INSERT_TAIL(&(block_list),dblock);
				 			 struct BlockMetaData *new_block;
				 			 void* new_addres;
				 			 new_addres=dblock;
				 			 new_addres+=dblock->size;
				 			 new_block=new_addres;
				 			 new_block->is_free=1;
				 			 new_block->size= 4096 - dblock->size;
				 			 new_block->prev_next_info.le_prev=dblock;
				 		  	 LIST_INSERT_TAIL(&(block_list),new_block);
//				 			 cprintf("here 7\n");
				 			 return (struct BlockMetaData *) dblock + 1;

	 }
	return NULL;
}




//void *alloc_block_FF(uint32 size)
//{
//	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
//		    if(size==0)
//				return NULL;
//
//		    if (!is_initialized)
//		    {
//				uint32 required_size = size + sizeOfMetaData();
//				uint32 da_start = (uint32)sbrk(required_size);
//				//get new break since it's page aligned! thus, the size can be more than the required one
//				uint32 da_break = (uint32)sbrk(0);
//				initialize_dynamic_allocator(da_start, da_break - da_start);
//		    }
//
//			uint32 required_size = size + sizeOfMetaData();
//			struct BlockMetaData* blk;
//			LIST_FOREACH(blk, &block_list)
//			{
//				if((blk)->is_free == 1 && blk->size == required_size){
//					blk->is_free = 0;
//					return (struct BlockMetaData*)((void*)blk + sizeOfMetaData());
//				}
//				else if(blk->is_free == 1 && blk->size > required_size){
//
//					if(blk->size -required_size>=sizeOfMetaData()){
//					struct BlockMetaData * new_block = (void*)blk +required_size;
//					new_block->size = blk->size - required_size;
//					new_block->is_free = 1;
//					blk->size = required_size;
//					blk->is_free = 0;
//					LIST_INSERT_AFTER(&block_list,blk,new_block);
//					}
//					else{
//						blk->is_free = 0;
//
//					}
//						return(struct BlockMetaData*) ((void*)blk+sizeOfMetaData());
//				}
//			}
//
//			// there is not enough space
//			struct BlockMetaData * tail = LIST_LAST(&block_list);
//			void* ret;
//			uint32 new_block_address = 0;
//			uint32 old_brk_address =(uint32)sbrk(0);
//		    if(tail->is_free == 0){
//				ret = sbrk(required_size);
//				if(ret == (void *)-1){
//					return NULL;
//				}
//				else{
//					struct BlockMetaData * new_block = ret;
//					new_block->size = ((uint32)sbrk(0) - old_brk_address);
//					new_block->is_free = 0;
//					LIST_INSERT_AFTER(&block_list,tail,new_block);
//
//					return(struct BlockMetaData*)( (void*)ret + sizeOfMetaData());
//				}
//			}
//			else{
//				ret = sbrk(required_size - (tail->size));
//				if(ret == (void *)-1){
//					return NULL;
//				}
//			    else{
//			    	tail->size = tail->size + ((uint32)sbrk(0) - old_brk_address);
//			    	tail->is_free=0;
//					return (struct BlockMetaData*)((void*)tail + sizeOfMetaData());
//				}
//			}
//}


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



uint32 get_block_up(struct BlockMetaData *blk)
{
	if(blk == NULL || blk->is_free == 0 || blk == LIST_FIRST(&(block_list)))
		return 0;

	uint32 ret = blk->size;

	ret += get_block_up(blk->prev_next_info.le_next);
	LIST_REMOVE(&(block_list),blk);

	blk->size = 0;
	blk->is_free = 0;

	return ret;
}
struct BlockMetaData *get_block_down(struct BlockMetaData *blk)
{

	if(blk == LIST_FIRST(&(block_list)) && blk->is_free)
		return blk;

	if(blk->is_free == 0)
		return blk->prev_next_info.le_next;

	struct BlockMetaData *ret = get_block_down(blk->prev_next_info.le_prev);

	if(ret == blk)
		return ret;
	LIST_REMOVE(&(block_list),blk);

	ret->size += blk->size;
	blk->size = 0;
	blk->is_free = 0;

	return ret;
}
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()

//	panic("wslttttt");

	struct BlockMetaData *blk = va;
	blk--;
	blk->is_free = (1==1);
	uint32 sz = 0;

	if(blk != LIST_LAST(&(block_list)))
		sz += get_block_up(blk->prev_next_info.le_next);

	struct BlockMetaData *curr_bot = blk;

	if(blk != LIST_FIRST(&(block_list)))
		curr_bot = get_block_down(blk->prev_next_info.le_prev);

	if(curr_bot != blk)
	{
		LIST_REMOVE(&(block_list),blk);
		sz += blk->size;
		blk->size = 0;
		blk->is_free = 0;
	}

	curr_bot->size += sz;
	curr_bot->is_free = (1==1);
}




//void free_block(void *va)
//{
//	if(va !=NULL){
//	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
//		struct BlockMetaData* blk =(void*)va-sizeOfMetaData();
//		struct BlockMetaData* prevblk=	LIST_PREV(blk);
//		struct BlockMetaData* nextblk=	LIST_NEXT(blk);
//		if(blk==LIST_FIRST(&block_list)){
//			if((nextblk)->is_free==1)
//			{
//				(blk)->is_free=1;
//				(blk)->size+=(nextblk)->size;
//				nextblk->size=0;
//				nextblk->is_free=0;
//				LIST_REMOVE(&block_list, nextblk);
//				nextblk=0;
//			}
//			else{
//
//				(blk)->is_free=1;
//			}
//
//		}
//		else if (blk==LIST_LAST(&block_list)){
//			if((prevblk)->is_free==1){
//		    	(prevblk)->is_free=1;
//				(prevblk)->size+=(blk)->size;
//				blk->size=0;
//				blk->is_free=0;
//				LIST_REMOVE(&block_list, blk);
//		 		blk=0;
//		    }
//		    else{
//				(blk)->is_free=1;
//		    }
//		}
//		else{
//		if((prevblk)->is_free==1)
//		{
//			if((nextblk)->is_free==1)
//			{
//				(prevblk)->is_free=1;
//				(prevblk)->size+=(blk)->size + (nextblk)->size;
//				blk->size=0;
//				blk->is_free=0;
//				nextblk->size=0;
//				nextblk->is_free=0;
//				LIST_REMOVE(&block_list, blk);
//				LIST_REMOVE(&block_list, nextblk);
//				blk=0;
//				nextblk=0;
//			}
//			else{
//			(prevblk)->size+=(blk)->size;
//			(prevblk)->is_free=1;
//			blk->size=0;
//			blk->is_free=0;
//			LIST_REMOVE(&block_list, blk);
//			blk=0;}
//		}
//
//		else if((nextblk)->is_free==1 &&(prevblk)->is_free==0)
//		{
//			(blk)->is_free=1;
//			(blk)->size+= (nextblk)->size;
//			nextblk->size=0;
//			nextblk->is_free=0;
//			LIST_REMOVE(&block_list,nextblk);
//			nextblk=0;
//
//		}
//		else if((prevblk)->is_free==0&&(nextblk)->is_free==0)
//		(blk)->is_free=1;
//	}
//  }
//}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	//special cases

	if((void*)va == NULL){
		return (void *)alloc_block_FF(new_size);
	}
	else if(new_size == 0){
		free_block((void*)va);
		return (void *)NULL;
	}
	new_size += sizeOfMetaData();
	// (regard that new size is not include meta_data && address is of the free space without meta_data) mego___o
	struct BlockMetaData * blk = (struct BlockMetaData*)((void*)va - sizeOfMetaData());
	struct BlockMetaData * tail = LIST_LAST(&block_list);
	if((uint32)new_size > (uint32)blk->size){
		uint32 required_size_to_expend = new_size - blk->size;
		if(blk!=tail){
			struct BlockMetaData* next_blk = LIST_NEXT((struct BlockMetaData*)blk);
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

					if(next_blk_new_size >= sizeOfMetaData()){
						//remove the next block we have use and create new one with the free space.
						struct BlockMetaData* new_next_blk = (void *)next_blk + required_size_to_expend;
						new_next_blk->size = next_blk_new_size;
						new_next_blk->is_free = 1;
						next_blk->size = 0;
						next_blk->is_free=0;
						blk->size = new_size;
						LIST_REMOVE(&block_list , next_blk);
						LIST_INSERT_AFTER(&block_list , blk ,new_next_blk);

					}
					else{
						blk->size += ((uint32) next_blk->size);
						next_blk->is_free = 0;
						next_blk->size = 0;
						LIST_REMOVE(&block_list , next_blk);
					}
					return (struct BlockMetaData*)((void*)blk + sizeOfMetaData());  // still in same address
				}
				else{
					void * ret = alloc_block_FF(new_size-sizeOfMetaData());
					if(ret == (void*)NULL){
						return (void *)-1;//no suitable block
					}else{
						free_block((void*)va);
						return (struct BlockMetaData*) ret; //the new allocated _blk
					}
				}
			}
			else{
				void * ret = alloc_block_FF(new_size-sizeOfMetaData());
				if(ret == (void*)NULL){
				return (void *)-1;//no suitable block
				}else{
					free_block((void*)va);
					return (struct BlockMetaData*) ret; //the new allocated _blk
				}
			}
		}
		else if(blk==tail){
			void * ret = alloc_block_FF(new_size-sizeOfMetaData());
			if(ret == (void*)NULL){
			  return (void *)-1;//no suitable block
			}
			else{
			  free_block((void*)va);
			  return (struct BlockMetaData*) ret; //the new allocated _blk
			}
		}
	}
	else if((uint32)new_size < (uint32)blk->size){
		uint32 new_blk_size = blk->size - new_size;
		if(blk==tail){
			if(new_blk_size >= sizeOfMetaData()){
			 struct BlockMetaData* new_next_blk = (void *)blk + new_size;
			 new_next_blk->size = new_blk_size;
			 new_next_blk->is_free = 1;
			 blk->size = new_size;
			 LIST_INSERT_AFTER(&block_list , blk ,new_next_blk);
			}
		}
		else{
			struct BlockMetaData* next_blk = LIST_NEXT((struct BlockMetaData*)blk);
			uint32 new_blk_size = blk->size - new_size;
			if(next_blk->is_free==1){
				blk->size = new_size;
				struct BlockMetaData * new_block = (void*)blk + new_size;
			    new_block->size = new_blk_size + next_blk->size;
				new_block->is_free = 1;
				next_blk->is_free=0;
				next_blk->size=0;
				LIST_REMOVE(&block_list , next_blk);
				LIST_INSERT_AFTER(&block_list,blk,new_block);
			}
			else{
				if(new_blk_size >= sizeOfMetaData()){
				struct BlockMetaData* new_next_blk = (void *)blk + new_size;
				new_next_blk->size = new_blk_size;
				new_next_blk->is_free = 1;
				blk->size = new_size;
				LIST_INSERT_AFTER(&block_list , blk ,new_next_blk);
			    }
			}
		}
		return (struct BlockMetaData*)((void*)blk + sizeOfMetaData()) ;  // still in same address
	}
	else{
		return (struct BlockMetaData*)((void*)blk + sizeOfMetaData()) ; // nothing to happen
	}

	return NULL;
}
