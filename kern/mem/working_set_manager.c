/*
 * working_set_manager.c
 *
 *  Created on: Oct 11, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"

///============================================================================================
/// Dealing with environment working set
#if USE_KHEAP
inline void working_set_elm_free(struct WorkingSetElement* elem){
	kfree(elem);
}
inline struct WorkingSetElement* env_page_ws_list_create_element(struct Env* e, uint32 virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #14] [3] PAGE FAULT HANDLER - Create a new working set element
	// Write your code here, remove the panic and write your code
	struct WorkingSetElement* ret = kmalloc(sizeof(struct WorkingSetElement));
	uint32*page_table=NULL;
	get_page_table(e->env_page_directory,virtual_address,&page_table);
	ret-> virtual_address = virtual_address;
	struct FrameInfo* frame=get_frame_info(e->env_page_directory,virtual_address,&page_table);
	frame->element=ret;
 // t2repn m4 m7tagin da hwa bi7gz bs w himlaha b3din b addres tani
	return ret;
}

inline void env_page_ws_invalidate(struct Env* e, uint32 virtual_address)
{
	if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		bool found = 0;
		uint32*page_table=NULL;
		get_page_table(e->env_page_directory,virtual_address,&page_table);
		struct FrameInfo*Active_frame=get_frame_info(e->env_page_directory,virtual_address,&page_table);
		uint32*page_table_2=NULL;
		get_page_table(e->env_page_directory,ROUNDDOWN(virtual_address,PAGE_SIZE),&page_table_2);
		struct FrameInfo*Active_frame_2=get_frame_info(e->env_page_directory,ROUNDDOWN(virtual_address,PAGE_SIZE),&page_table_2);
		if(Active_frame_2!=NULL)
		Active_frame=Active_frame_2;
			if(Active_frame!=NULL){

		struct WorkingSetElement *ptr_WS_element = Active_frame->element;
			if(Active_frame->in_active==1)
			{
				Active_frame->in_active=0;

				struct WorkingSetElement* ptr_tmp_WS_element = LIST_FIRST(&(e->SecondList));
				unmap_frame(e->env_page_directory, ptr_WS_element->virtual_address);
				LIST_REMOVE(&(e->ActiveList), ptr_WS_element);
				/*EDIT*/ kfree(ptr_WS_element);

				if(ptr_tmp_WS_element != NULL)
				{
					uint32*non_vectim_table=NULL;
					get_page_table(e->env_page_directory,ptr_tmp_WS_element->virtual_address,&non_vectim_table);
					struct FrameInfo*non_vectim_frame=get_frame_info(e->env_page_directory,ptr_tmp_WS_element->virtual_address,&non_vectim_table);
					LIST_REMOVE(&(e->SecondList), ptr_tmp_WS_element);
					LIST_INSERT_TAIL(&(e->ActiveList), ptr_tmp_WS_element);
					non_vectim_frame->in_second=0;
					non_vectim_frame->in_active=1;
					pt_set_page_permissions(e->env_page_directory, ptr_tmp_WS_element->virtual_address, PERM_PRESENT, 0);
				}
				found = 1;

			}


		if (!found)
		{

					Active_frame->in_second=0;

					unmap_frame(e->env_page_directory, ptr_WS_element->virtual_address);
					LIST_REMOVE(&(e->SecondList), ptr_WS_element);

					kfree(ptr_WS_element);

					/*EDIT*/


			}
		}
	}
	else
	{
		uint32*ptr_page_table=NULL;
			struct FrameInfo* Active_frame =get_frame_info(e->env_page_directory,virtual_address,&ptr_page_table);
			if(Active_frame!=NULL&&Active_frame->element!=NULL){
				struct WorkingSetElement*Active_elm=Active_frame->element;
				if (e->page_last_WS_element == Active_elm)
					{
						e->page_last_WS_element = LIST_NEXT(Active_elm);
					}
				LIST_REMOVE(&(e->page_WS_list), Active_elm);

				kfree(Active_elm);
			}
	}
}

void env_page_ws_print(struct Env *e)
{
	if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		int i = 0;
		cprintf("ActiveList:\n============\n") ;
		struct WorkingSetElement * ptr_WS_element ;
		LIST_FOREACH(ptr_WS_element, &(e->ActiveList))
		{
			cprintf("%d:	%x\n", i++, ptr_WS_element->virtual_address);
		}
		cprintf("\nSecondList:\n============\n") ;
		LIST_FOREACH(ptr_WS_element, &(e->SecondList))
		{
			cprintf("%d:	%x\n", i++, ptr_WS_element->virtual_address);
		}
	}
	else
	{
		uint32 i=0;
		cprintf("PAGE WS:\n");
		struct WorkingSetElement *wse = NULL;
		LIST_FOREACH(wse, &(e->page_WS_list))
		{
			uint32 virtual_address = wse->virtual_address;
			uint32 time_stamp = wse->time_stamp;

			uint32 perm = pt_get_page_permissions(e->env_page_directory, virtual_address) ;
			char isModified = ((perm&PERM_MODIFIED) ? 1 : 0);
			char isUsed= ((perm&PERM_USED) ? 1 : 0);
			char isBuffered= ((perm&PERM_BUFFERED) ? 1 : 0);

			cprintf("%d: %x",i, virtual_address);

			//2021
			cprintf(", used= %d, modified= %d, buffered= %d, time stamp= %x, sweeps_cnt= %d",
					isUsed, isModified, isBuffered, time_stamp, wse->sweeps_counter) ;

			if(wse == e->page_last_WS_element)
			{
				cprintf(" <--");
			}
			cprintf("\n");
			i++;
		}
		for (; i < e->page_WS_max_size; ++i)
		{
			cprintf("EMPTY LOCATION");
		}
	}
}
#else
inline uint32 env_page_ws_get_size(struct Env *e)
{
	int i=0, counter=0;
	for(;i<e->page_WS_max_size; i++) if(e->ptr_pageWorkingSet[i].empty == 0) counter++;
	return counter;
}

inline void env_page_ws_invalidate(struct Env* e, uint32 virtual_address)
{
	int i=0;
	for(;i<e->page_WS_max_size; i++)
	{
		if(ROUNDDOWN(e->ptr_pageWorkingSet[i].virtual_address,PAGE_SIZE) == ROUNDDOWN(virtual_address,PAGE_SIZE))
		{
			env_page_ws_clear_entry(e, i);
			break;
		}
	}
}

inline void env_page_ws_set_entry(struct Env* e, uint32 entry_index, uint32 virtual_address)
{
	assert(entry_index >= 0 && entry_index < e->page_WS_max_size);
	assert(virtual_address >= 0 && virtual_address < USER_TOP);
	e->ptr_pageWorkingSet[entry_index].virtual_address = ROUNDDOWN(virtual_address,PAGE_SIZE);
	e->ptr_pageWorkingSet[entry_index].empty = 0;

	e->ptr_pageWorkingSet[entry_index].time_stamp = 0x80000000;
	//e->ptr_pageWorkingSet[entry_index].time_stamp = time;
	return;
}

inline void env_page_ws_clear_entry(struct Env* e, uint32 entry_index)
{
	assert(entry_index >= 0 && entry_index < (e->page_WS_max_size));
	e->ptr_pageWorkingSet[entry_index].virtual_address = 0;
	e->ptr_pageWorkingSet[entry_index].empty = 1;
	e->ptr_pageWorkingSet[entry_index].time_stamp = 0;
}

inline uint32 env_page_ws_get_virtual_address(struct Env* e, uint32 entry_index)
{
	assert(entry_index >= 0 && entry_index < (e->page_WS_max_size));
	return ROUNDDOWN(e->ptr_pageWorkingSet[entry_index].virtual_address,PAGE_SIZE);
}

inline uint32 env_page_ws_get_time_stamp(struct Env* e, uint32 entry_index)
{
	assert(entry_index >= 0 && entry_index < (e->page_WS_max_size));
	return e->ptr_pageWorkingSet[entry_index].time_stamp;
}

inline uint32 env_page_ws_is_entry_empty(struct Env* e, uint32 entry_index)
{
	return e->ptr_pageWorkingSet[entry_index].empty;
}

void env_page_ws_print(struct Env *e)
{
	if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		int i = 0;
		cprintf("ActiveList:\n============\n") ;
		struct WorkingSetElement * ptr_WS_element ;
		LIST_FOREACH(ptr_WS_element, &(e->ActiveList))
		{
			cprintf("%d:	%x\n", i++, ptr_WS_element->virtual_address);
		}
		cprintf("\nSecondList:\n============\n") ;
		LIST_FOREACH(ptr_WS_element, &(e->SecondList))
		{
			cprintf("%d:	%x\n", i++, ptr_WS_element->virtual_address);
		}
	}
	else
	{
		uint32 i;
		cprintf("PAGE WS:\n");
		for(i=0; i< (e->page_WS_max_size); i++ )
		{
			if (e->ptr_pageWorkingSet[i].empty)
			{
				cprintf("EMPTY LOCATION");
				if(i==e->page_last_WS_index )
				{
					cprintf("		<--");
				}
				cprintf("\n");
				continue;
			}
			uint32 virtual_address = e->ptr_pageWorkingSet[i].virtual_address;
			uint32 time_stamp = e->ptr_pageWorkingSet[i].time_stamp;

			uint32 perm = pt_get_page_permissions(e->env_page_directory, virtual_address) ;
			char isModified = ((perm&PERM_MODIFIED) ? 1 : 0);
			char isUsed= ((perm&PERM_USED) ? 1 : 0);
			char isBuffered= ((perm&PERM_BUFFERED) ? 1 : 0);


			cprintf("address @ %d = %x",i, e->ptr_pageWorkingSet[i].virtual_address);

			//2021
			cprintf(", used= %d, modified= %d, buffered= %d, time stamp= %x, sweeps_cnt= %d", isUsed, isModified, isBuffered, time_stamp, e->ptr_pageWorkingSet[i].sweeps_counter) ;

			if(i==e->page_last_WS_index )
			{
				cprintf(" <--");
			}
			cprintf("\n");
		}
	}
}
#endif
// Table Working Set =========================================================

void env_table_ws_print(struct Env *e)
{
	uint32 i;
	cprintf("---------------------------------------------------\n");
	cprintf("TABLE WS:\n");
	for(i=0; i< __TWS_MAX_SIZE; i++ )
	{
		if (e->__ptr_tws[i].empty)
		{
			cprintf("EMPTY LOCATION");
			if(i==e->table_last_WS_index )
			{
				cprintf("		<--");
			}
			cprintf("\n");
			continue;
		}
		uint32 virtual_address = e->__ptr_tws[i].virtual_address;
		cprintf("env address at %d = %x",i, e->__ptr_tws[i].virtual_address);

		cprintf(", used bit = %d, time stamp = %d", pd_is_table_used(e->env_page_directory, virtual_address), e->__ptr_tws[i].time_stamp);
		if(i==e->table_last_WS_index )
		{
			cprintf(" <--");
		}
		cprintf("\n");
	}
}

inline uint32 env_table_ws_get_size(struct Env *e)
{
	int i=0, counter=0;
	for(;i<__TWS_MAX_SIZE; i++) if(e->__ptr_tws[i].empty == 0) counter++;
	return counter;
}

inline void env_table_ws_invalidate(struct Env* e, uint32 virtual_address)
{
	int i=0;
	for(;i<__TWS_MAX_SIZE; i++)
	{
		if(ROUNDDOWN(e->__ptr_tws[i].virtual_address,PAGE_SIZE*1024) == ROUNDDOWN(virtual_address,PAGE_SIZE*1024))
		{
			env_table_ws_clear_entry(e, i);
			break;
		}
	}
}

inline void env_table_ws_set_entry(struct Env* e, uint32 entry_index, uint32 virtual_address)
{
	assert(entry_index >= 0 && entry_index < __TWS_MAX_SIZE);
	assert(virtual_address >= 0 && virtual_address < USER_TOP);
	e->__ptr_tws[entry_index].virtual_address = ROUNDDOWN(virtual_address,PAGE_SIZE*1024);
	e->__ptr_tws[entry_index].empty = 0;

	//e->__ptr_tws[entry_index].time_stamp = time;
	e->__ptr_tws[entry_index].time_stamp = 0x80000000;
	return;
}

inline void env_table_ws_clear_entry(struct Env* e, uint32 entry_index)
{
	assert(entry_index >= 0 && entry_index < __TWS_MAX_SIZE);
	e->__ptr_tws[entry_index].virtual_address = 0;
	e->__ptr_tws[entry_index].empty = 1;
	e->__ptr_tws[entry_index].time_stamp = 0;
}

inline uint32 env_table_ws_get_virtual_address(struct Env* e, uint32 entry_index)
{
	assert(entry_index >= 0 && entry_index < __TWS_MAX_SIZE);
	return ROUNDDOWN(e->__ptr_tws[entry_index].virtual_address,PAGE_SIZE*1024);
}


inline uint32 env_table_ws_get_time_stamp(struct Env* e, uint32 entry_index)
{
	assert(entry_index >= 0 && entry_index < __TWS_MAX_SIZE);
	return e->__ptr_tws[entry_index].time_stamp;
}

inline uint32 env_table_ws_is_entry_empty(struct Env* e, uint32 entry_index)
{
	return e->__ptr_tws[entry_index].empty;
}

///=================================================================================================
///=================================================================================================
///=================================================================================================
///=================================================================================================

