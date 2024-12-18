#ifndef MM_PAGING
#define MM_PAGING
#endif
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

/*enlist_vm_freerg_list - add new rg to freerg_list (Them mot vung nho moi da duoc giai phong vao danh sach vung nho tu do cua cau truc quan ly bo nho)
 *@mm: memory region
 *@rg_elmt: new region
 *
rg_elmt : Vung nho duoc giai phong
mm_struct: Cau truc quan ly bo nho
mm_struct->mmap->vm_freerg_list: Danh sach cac vung nho tu do
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct * rg_elmt) 
{
  struct vm_area_struct * cur_vma = get_vma_by_num(mm, rg_elmt->vmaid);
  struct vm_rg_struct *rg_node = cur_vma->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;
  while(rg_node != NULL){
    if(rg_node->rg_start == rg_elmt->rg_end + 1){
      rg_node->rg_start = rg_elmt->rg_start;
      free(rg_elmt);
      struct vm_rg_struct * current = rg_node->rg_next,* prev = rg_node;
      while(current != NULL){
        if(rg_node->rg_start == current->rg_end + 1){
          rg_node->rg_start = current->rg_start;
          prev->rg_next = current->rg_next;
          free(current);
          break;
        }
        prev = current;
        current = current->rg_next;
      }
      return 0;
    }
    if(rg_node->rg_end + 1== rg_elmt->rg_start){
      rg_node->rg_end = rg_elmt->rg_end;
      free(rg_elmt);
      struct vm_rg_struct * current = rg_node->rg_next,* prev = rg_node;
      while(current != NULL){
        if(rg_node->rg_end + 1 == current->rg_start ){
          rg_node->rg_end = current->rg_end;
          prev->rg_next = current->rg_next;
          free(current);
          break;
        }
        prev = current;
        current = current->rg_next;
      }
      return 0;
    }
    rg_node = rg_node->rg_next;
  }
  /* Enlist the new region */
  rg_elmt->rg_next = cur_vma->vm_freerg_list;
  cur_vma->vm_freerg_list = rg_elmt;

  return 0;
}

/*get_vma_by_num - get vm area by numID		(Truy xuat mot vung bo nho ao dua tren so thu tu vmaid tu danh sach vung bo nho ao mmap trong mm_struct)
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma= mm->mmap;

  if(mm->mmap == NULL)
    return NULL;

  int vmait = 0;
  
  while (vmait < vmaid)
  {
    if(pvma == NULL)
	  return NULL;

    vmait++;
    pvma = pvma->vm_next;
  }

  return pvma;
}

/*get_symrg_byid - get mem region by region ID (Truy xuat mot vung nho trong bang ky hieu (symgtbl) dua tren ID vung nho (rgid))
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory (Cap phat mot vung nho trong khong gian bo nho ao cua mot tien trinh)
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_rg_struct rgnode;
  /* TODO: commit the vmaid */
  rgnode.vmaid = vmaid;

  // Kiem tra xem vmrg co du lon hay khong
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    // Cap nhat bang ky hieu voi thong tin vmrg moi
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;
    *alloc_addr = rgnode.rg_start;

    #ifdef VMDBG
    int size = rgnode.rg_end - rgnode.rg_start + 1;
    printf("alloc %d rgid=%d [%ld->%ld<at>vma%d]\nfreelist:",size ,rgid, rgnode.rg_start, rgnode.rg_end, rgnode.vmaid);
    print_list_rg(cur_vma->vm_freerg_list);

    #endif //VMDBG

    return 0;
  }

  /* TODO: get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  // Co gang tang kich thuoc cua vmarea

  
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int inc_limit_ret;

  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  int old_sbrk = cur_vma->sbrk;
  if(vmaid == 1){
    cur_vma->sbrk -= inc_sz;
  }
  /* TODO INCREASE THE LIMIT
   * inc_vma_limit(caller, vmaid, inc_sz)
   */
  if(inc_vma_limit(caller, vmaid, inc_sz, &inc_limit_ret) < 0){
    cur_vma->sbrk = old_sbrk;
    return -1;
  }
  /* TODO: commit the limit increment */
  if(vmaid == 1){
    cur_vma->vm_start = cur_vma->sbrk;
  }else {
    cur_vma->vm_end += inc_limit_ret;
    cur_vma->sbrk += inc_limit_ret;
  }
  /* TODO: commit the allocation address 
  // *alloc_addr = ...
  */
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    // Cap nhat bang ky hieu voi thong tin vmrg moi
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;
    *alloc_addr = rgnode.rg_start;

  #ifdef VMDBG
  printf("not enough space, increase vma more %d\n", inc_sz);
  int size = rgnode.rg_end - rgnode.rg_start + 1;
  printf("alloc %d rgid=%d [%ld->%ld<at>vma%d]\nfreelist:",size ,rgid, rgnode.rg_start, rgnode.rg_end, rgnode.vmaid);
  print_list_rg(cur_vma->vm_freerg_list);
  print_pgtbl(caller, 0,-1);
  #endif //VMDBG
    return 0;
  }

  return -1;
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __free(struct pcb_t *caller, int rgid)
{
  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  struct vm_rg_struct * rgnode = malloc(sizeof(struct vm_rg_struct));
  rgnode->rg_start = caller->mm->symrgtbl[rgid].rg_start;
  rgnode->rg_end =caller->mm->symrgtbl[rgid].rg_end;
  rgnode->vmaid = caller->mm->symrgtbl[rgid].vmaid;

  #ifdef VMDBG
  int size = rgnode->rg_end - rgnode->rg_start + 1;
  struct vm_area_struct * cur_vma = get_vma_by_num(caller->mm, rgnode->vmaid);
  printf("free rgid=%d [%ld->%ld(%d)<at>vma%d]\nfreelist:",rgid, rgnode->rg_start, rgnode->rg_end,size, rgnode->vmaid);
  #endif //VMDBG
  
  /* TODO: Manage the collect freed region to freerg_list */
  //Cap nhat thong tin ve vung nho da giai phong
  caller->mm->symrgtbl[rgid].rg_start = -1;
  caller->mm->symrgtbl[rgid].rg_end = -1;
  caller->mm->symrgtbl[rgid].vmaid = -1;

  /*enlist the obsoleted memory region */
  // Tra vung nho nay ve freerg list
  enlist_vm_freerg_list(caller->mm, rgnode);
  #ifdef VMDBG
  print_list_rg(cur_vma->vm_freerg_list);
  #endif //VMDBG
  return 0;
}

/*pgalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 0 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*pgmalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify vaiable in symbole table)
 */
int pgmalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 1 */
  return __alloc(proc, 1, reg_index, size, &addr);
}

/*pgfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
{
   return __free(proc, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];// page table entry tu bang trang
 
  if (!PAGING_PTE_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    // Trang khong co trong RAM -> tim trong SWAP
    if(!PAGING_PTE_PAGE_SWAPPED(pte)) return -1; // Trang nay khong co trong SWAP
    int vicpgn, swpfpn; // so trang cua nan nhan va 1 trang trong trong swap

    int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    find_victim_page(caller->mm, &vicpgn);

    /* Get free frame in MEMSWP */
    //Tim 1 trang trong trong SWAP
    MEMPHY_get_freefp(caller->active_mswp, &swpfpn);


    /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
    /* Copy victim frame to swap */
    // Chuyen nan nhan vao trong SWAP
    __swap_cp_page(caller->mram, vicpgn, caller->active_mswp, swpfpn);
    /* Copy target frame from swap to mem */
    // Chuyen target vao trong RAM
    __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicpgn);

    /* Update page table */
    // Cap nhat bang phan trang
    pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);

    /* Update its online status of the target page */
    pte_set_fpn(&pte, vicpgn);
    // Them trang vao hang doi fifo
    enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }

  *fpn = PAGING_PTE_FPN(pte);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess 
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram,phyaddr, data);

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess 
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_write(caller->mram,phyaddr, value);

   return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region 
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __read(struct pcb_t *caller, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}


/*pgwrite - PAGING-based read a region memory */
int pgread(
		struct pcb_t * proc, // Process executing the instruction
		uint32_t source, // Index of source register
		uint32_t offset, // Source address = [source] + [offset]
		uint32_t destination) 
{
  BYTE data;
  int val = __read(proc, source, offset, &data);

  destination = (uint32_t) data;
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif //PAGETBL_DUMP
#ifdef MEMPHY_DUMP
  MEMPHY_dump(proc->mram);
#endif // MEMPHY_DUMP
#endif // IODUMP

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region 
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __write(struct pcb_t *caller, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  
  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*pgwrite - PAGING-based write a region memory */
int pgwrite(
		struct pcb_t * proc, // Process executing the instruction
		BYTE data, // Data to be wrttien into memory
		uint32_t destination, // Index of destination register
		uint32_t offset)
{
  int res = __write(proc, destination, offset, data);
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif //PAGETBL_DUMP
#ifdef MEMPHY_DUMP
  MEMPHY_dump(proc->mram);
#endif // MEMPHY_DUMP
#endif // IODUMP
  return res;
}


/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PTE_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}

/*get_vm_area_node_at_brk - get new area from sbrk of another area
 *@caller: caller
 *@vmaid: ID vm area to get sbrk
 *@size: size of area
 *@alignedsz:aligned size
 *
 */
struct vm_rg_struct* get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct * newrg;

  newrg = malloc(sizeof(struct vm_rg_struct));
  if(newrg == NULL) return NULL;
  struct vm_area_struct * cur_vma = get_vma_by_num(caller->mm, vmaid);
  // TODO: update the newrg boundary
  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + alignedsz -1 ;
  newrg->vmaid = vmaid;

  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  struct vm_area_struct *vma = caller->mm->mmap;

  /* TODO validate the planned memory area is not overlapped */
  while (vma != NULL){
    if(vma->vm_id != vmaid){
      if(OVERLAP(vma->vm_start, vma->vm_end, vmastart, vmaend)) return -1;
      if(INCLUDE(vma->vm_start, vma->vm_end, vmastart, vmaend)) return -1;
    }
    //Di chuyen qua vma tiep theo
    vma = vma->vm_next;

  }

  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size 
 *@inc_limit_ret: increment limit return
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz, int* inc_limit_ret)
{
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz); // Can chinh cho dung dia chi trang
  int incnumpage =  inc_amt / PAGING_PAGESZ; // So trang nho can cap phat
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);

  int old_end = area->rg_start;

  /*Validate overlap of obtained region */
  /*Kiem tra trung lap vung nho */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    return -1; /*Overlap and failed allocation */

  /* TODO: Obtain the new vm area based on vmaid */
  /* Cap nhat gioi han moi va kich thuoc vung cap phat da mo rong*/
  * inc_limit_ret = inc_amt;
  /* Anh xa vung nho moi vao RAM*/
  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                    old_end, incnumpage , newrg) < 0)
    return -1; /* Map the memory to MEMRAM */
  enlist_vm_freerg_list(caller->mm, area);
  return 0;

}

/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn) 
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */
  if(pg == NULL){
    return -1;
  }
  // Tai vi fifo them dau nen phai xoa cuoi 
  struct pgn_t * prev = NULL;
  while (pg->pg_next != NULL){
    prev = pg;
    pg = pg->pg_next;
  }
  if(prev != NULL){
    prev->pg_next = NULL;
  }

  *retpgn = pg->pgn;
  free(pg);
  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size 
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* Traverse on list of free vm region to find a fit space */
  while (rgit != NULL && rgit->vmaid == vmaid)
  {
    if (rgit->rg_end - rgit->rg_start + 1 >= size)
    { /* Current region has enough space */
      if(vmaid == 1){
        newrg->rg_end = rgit->rg_end;
        newrg->rg_start = rgit->rg_end - size + 1;
      }else {
        newrg->rg_start = rgit->rg_start;
        newrg->rg_end = newrg->rg_start + size - 1;
      }

      /* Update left space in chosen region */
      if (rgit->rg_end - rgit->rg_start + 1 > size)
      {
        if(vmaid == 1){
          rgit->rg_end = rgit->rg_end - size;
        }else {
          rgit->rg_start = rgit->rg_start + size;
        }
      }
      else
      { /*Use up all space, remove current node */
        /*Clone next rg node */
        struct vm_rg_struct *nextrg = rgit->rg_next;

        /*Cloning */
        if (nextrg != NULL)
        {
          rgit->rg_start = nextrg->rg_start;
          rgit->rg_end = nextrg->rg_end;

          rgit->rg_next = nextrg->rg_next;

          free(nextrg);
        }
        else
        { /*End of free list */
          rgit->rg_start = rgit->rg_end;	//dummy, size 0 region
          rgit->rg_next = NULL;
        }
      }
      break;
    }
    else
    {
      rgit = rgit->rg_next;	// Traverse next rg
    }
  }

 if(newrg->rg_start == -1) // new region not found
   return -1;

 return 0;
}

// #endif
