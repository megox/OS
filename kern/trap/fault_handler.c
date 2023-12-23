/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif

if(isPageReplacmentAlgorithmFIFO()){
		if(wsSize < (curenv->page_WS_max_size))
		{
			//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
			//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
			// Write your code here, remove the panic and write your code

			struct FrameInfo *new_frame ;
			int ret = allocate_frame(&new_frame);
			if(ret==0)
			{
				ret=map_frame(curenv->env_page_directory,new_frame,fault_va,PERM_USER|PERM_WRITEABLE|PERM_PRESENT);
				if(ret==0)
				{
					int ret3 = pf_read_env_page(curenv, (void*)fault_va);
					//the page not exist in page file
					if (ret3 == E_PAGE_NOT_EXIST_IN_PF)
					{
						if((fault_va<USER_HEAP_START || fault_va>USER_HEAP_MAX)
								&& (fault_va>USTACKTOP||fault_va<USTACKBOTTOM))
						{
							sched_kill_env(curenv->env_id);
						}
					}
					struct WorkingSetElement*new_elm= env_page_ws_list_create_element(curenv,fault_va);
					LIST_INSERT_TAIL(&(curenv->page_WS_list),new_elm);
					if((curenv->page_WS_max_size)==LIST_SIZE(&(curenv->page_WS_list)))
					{
						curenv->page_last_WS_element=LIST_FIRST(&(curenv->page_WS_list));
					}
					else
					{
						curenv->page_last_WS_element=NULL;
					}
				}
			}

		}
else
{
	//refer to the project presentation and documentation for details
		//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");

	bool last=0;          //check if it's last elm in tail of list
	if(curenv->page_last_WS_element==LIST_LAST(&(curenv->page_WS_list))){
		last=1;
	}

	struct FrameInfo *new_frame;
	int ret = allocate_frame(&new_frame);
	if(ret==0){
	ret=map_frame(curenv->env_page_directory,new_frame,fault_va,PERM_USER|PERM_WRITEABLE|PERM_PRESENT);
	if(ret==0){
		int ret3 = pf_read_env_page(curenv, (void*)fault_va);
		//the page not exist in page file
		if (ret3 == E_PAGE_NOT_EXIST_IN_PF){
			if((fault_va<USER_HEAP_START || fault_va>USER_HEAP_MAX)
					&& (fault_va>USTACKTOP||fault_va<USTACKBOTTOM)){
				sched_kill_env(curenv->env_id);
			}
		}

		struct WorkingSetElement* victim=curenv->page_last_WS_element;
		uint32 *ptr_page_table=NULL;
		struct FrameInfo *old_frame=get_frame_info(curenv->env_page_directory,victim->virtual_address,&ptr_page_table);
		get_page_table(curenv->env_page_directory,victim->virtual_address,&ptr_page_table);
		int index=PTX(victim->virtual_address);
		if((ptr_page_table[index]&PERM_MODIFIED)==PERM_MODIFIED)
		{
			pf_update_env_page(curenv,victim->virtual_address,old_frame);
		}

		env_page_ws_invalidate(curenv, victim->virtual_address);
		unmap_frame(curenv->env_page_directory,victim->virtual_address);
		struct WorkingSetElement*new_elm= env_page_ws_list_create_element(curenv,fault_va);
		if(last==1){
			LIST_INSERT_TAIL(&(curenv->page_WS_list),new_elm);
		}
		else
		{
			LIST_INSERT_BEFORE(&(curenv->page_WS_list),curenv->page_last_WS_element,new_elm);
		}

	}
	}



	if(last==1)
	{
		curenv->page_last_WS_element=LIST_FIRST(&(curenv->page_WS_list));
	}


}
	}
else
{

//refer to the project presentation and documentation for details
// FIRST WE NEED TO CHECK IF THE FAULTED VA IN THE SECONDE LIST OR NOT
//fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);

uint32*page_table=NULL;
get_page_table(curenv->env_page_directory,fault_va,&page_table);
struct FrameInfo*check_frame=get_frame_info(curenv->env_page_directory,fault_va,&page_table);
uint32*page_table_2=NULL;
get_page_table(curenv->env_page_directory,ROUNDDOWN(fault_va,PAGE_SIZE),&page_table_2);
struct FrameInfo*check_frame_2=get_frame_info(curenv->env_page_directory,ROUNDDOWN(fault_va,PAGE_SIZE),&page_table_2);
//env_page_ws_print(curenv);
if(check_frame_2!=NULL)
check_frame=check_frame_2;

struct WorkingSetElement *exist=NULL;
if(check_frame!=NULL){
exist=check_frame->element;
}


if(exist!=NULL){
LIST_REMOVE(&(curenv->SecondList),exist);
check_frame->in_second=0;
pt_set_page_permissions(curenv->env_page_directory,exist->virtual_address,PERM_PRESENT,0);
if(LIST_SIZE(&(curenv->ActiveList))<curenv->ActiveListSize){
LIST_INSERT_HEAD(&(curenv->ActiveList),exist);
check_frame->in_active=1;
}

else{
struct WorkingSetElement *vectim=LIST_LAST(&(curenv->ActiveList));
uint32*page_table_vectim=NULL;
get_page_table(curenv->env_page_directory,vectim->virtual_address,&page_table_vectim);
struct FrameInfo*vectim_frame=get_frame_info(curenv->env_page_directory,vectim->virtual_address,&page_table_vectim);
vectim_frame->in_active=0;
vectim_frame->in_second=1;
LIST_REMOVE(&(curenv->ActiveList),vectim);
pt_set_page_permissions(curenv->env_page_directory,vectim->virtual_address,0,PERM_PRESENT);
LIST_INSERT_HEAD(&(curenv->SecondList),vectim);
LIST_INSERT_HEAD(&(curenv->ActiveList),exist);
check_frame->in_active=1;
}
}
else
{
struct FrameInfo* new_frame;
int ret = allocate_frame(&new_frame);
if(ret==0)
{
	ret=map_frame(curenv->env_page_directory,new_frame,fault_va,PERM_USER|PERM_WRITEABLE|PERM_PRESENT);
	if(ret==0)
	{

		int ret3 = pf_read_env_page(curenv, (void*)fault_va);
		//the page not exist in page file
		if (ret3 == E_PAGE_NOT_EXIST_IN_PF)
		{
		if((fault_va<USER_HEAP_START || fault_va>USER_HEAP_MAX)
				&& (fault_va>USTACKTOP||fault_va<USTACKBOTTOM)){
		unmap_frame(curenv->env_page_directory,fault_va);
				//cprintf("3333333333333=%x\n",fault_va);
			sched_kill_env(curenv->env_id);
		}
//					else
//						ret=pf_update_env_page(curenv,fault_va,new_frame);
	//	WE WILL UPDATE THE PAGE FILE TO CONTAIN THE FAULTED PAGE



		}
		if(ret==0)
		{

			struct WorkingSetElement*new_elm= env_page_ws_list_create_element(curenv,fault_va);
			if(LIST_SIZE(&(curenv->ActiveList))+LIST_SIZE(&(curenv->SecondList))<curenv->ActiveListSize+curenv->SecondListSize)
			{

				//THRE IS STILL A PLACE IN THE MEMORY SO WE WILL DO A LRU PLACMENT
				if(LIST_SIZE(&(curenv->ActiveList))<curenv->ActiveListSize)
				{
					//pt_set_page_permissions(curenv->env_page_directory,new_elm->virtual_address,PERM_PRESENT,0);

					LIST_INSERT_HEAD(&(curenv->ActiveList),new_elm);
					new_frame->in_active=1;
				}
				else
				{

					struct WorkingSetElement *vectim=LIST_LAST(&(curenv->ActiveList));
					uint32*page_table_vectim=NULL;
					get_page_table(curenv->env_page_directory,vectim->virtual_address,&page_table_vectim);
					struct FrameInfo*vectim_frame=get_frame_info(curenv->env_page_directory,vectim->virtual_address,&page_table_vectim);
					vectim_frame->in_active=0;
					vectim_frame->in_second=1;
					LIST_REMOVE(&(curenv->ActiveList),vectim);
					pt_set_page_permissions(curenv->env_page_directory,vectim->virtual_address,0,PERM_PRESENT);
					LIST_INSERT_HEAD(&(curenv->SecondList),vectim);
					LIST_INSERT_HEAD(&(curenv->ActiveList),new_elm);
					new_frame->in_active=1;
				}
			}
			else
			{


				//THRE IS NO PLACE IN THE MEMORY SO WE WILL DO A LRU REPLACMENT
				struct WorkingSetElement *dead=LIST_LAST(&(curenv->SecondList));
				if((pt_get_page_permissions(curenv->env_page_directory,dead->virtual_address)&PERM_MODIFIED)==PERM_MODIFIED){

					uint32*dead_page_table = NULL;
					get_page_table(curenv->env_page_directory,dead->virtual_address,&dead_page_table);
					struct FrameInfo*dead_frame=get_frame_info(curenv->env_page_directory,dead->virtual_address,&dead_page_table);
					pf_update_env_page(curenv,dead->virtual_address,dead_frame);

				}

				env_page_ws_invalidate(curenv,dead->virtual_address);

					//unmap_frame(curenv->env_page_directory,dead->virtual_address);
//											LIST_REMOVE(&(curenv->SecondList),dead);
				//working_set_elm_free(dead);
				struct WorkingSetElement *vectim=LIST_LAST(&(curenv->ActiveList));
				uint32*page_table_vectim=NULL;
				get_page_table(curenv->env_page_directory,vectim->virtual_address,&page_table_vectim);
				struct FrameInfo *vectim_frame=get_frame_info(curenv->env_page_directory,vectim->virtual_address,&page_table_vectim);
				vectim_frame->in_active=0;
				vectim_frame->in_second=1;
				LIST_REMOVE(&(curenv->ActiveList),vectim);
				pt_set_page_permissions(curenv->env_page_directory,vectim->virtual_address,0,PERM_PRESENT);
				LIST_INSERT_HEAD(&(curenv->SecondList),vectim);

				LIST_INSERT_HEAD(&(curenv->ActiveList),new_elm);
				new_frame->in_active=1;

			}
		}

	}



}




}


}






}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



