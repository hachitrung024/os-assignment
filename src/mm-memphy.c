//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory physical module mm/mm-memphy.c
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
/*
 *  MEMPHY_mv_csr - move MEMPHY cursor (Di chuyen con tro csr trong bo nho vat ly den dia chi cu the)
 *  @mp: memphy struct 			(Dai dien cho bo nho vat ly)
 *  @offset: offset				(So buoc can di chuyen)
 */
int MEMPHY_mv_csr(struct memphy_struct *mp, int offset)
{
   int numstep = 0;

   mp->cursor = 0;
   while(numstep < offset && numstep < mp->maxsz){
     /* Traverse sequentially */
     mp->cursor = (mp->cursor + 1) % mp->maxsz;
     numstep++;
   }

   return 0;
}

/*
 *  MEMPHY_seq_read - read MEMPHY device	(Doc du lieu tu bo nho o che do truy cap tuan tu)
 *  @mp: memphy struct	
 *  @addr: address							(Dia chi can doc)
 *  @value: obtained value					(Con tro luu gia tri doc duoc)
 */
int MEMPHY_seq_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   if (!mp->rdmflg)
     return -1; /* Not compatible mode for sequential read */

   MEMPHY_mv_csr(mp, addr);
   *value = (BYTE) mp->storage[addr];

   return 0;
}

/*
 *  MEMPHY_read read MEMPHY device			(Doc du lieu tu bo nho vat ly)
 *  @mp: memphy struct						
 *  @addr: address							(Dia chi can doc)
 *  @value: obtained value
 */
int MEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   if (mp->rdmflg)
      *value = mp->storage[addr];
   else /* Sequential access device */
      return MEMPHY_seq_read(mp, addr, value);

   return 0;
}

/*
 *  MEMPHY_seq_write - write MEMPHY device				(Ghi du lieu vao bo nho o che do tuan tu)
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data									(Du lieu can ghi)
 */
int MEMPHY_seq_write(struct memphy_struct * mp, int addr, BYTE value)
{

   if (mp == NULL)
     return -1;

   if (!mp->rdmflg)
     return -1; /* Not compatible mode for sequential read */

   MEMPHY_mv_csr(mp, addr);
   mp->storage[addr] = value;

   return 0;
}

/*
 *  MEMPHY_write-write MEMPHY device					(Ghi du lieu vao bo nho)
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   if (mp->rdmflg)
      mp->storage[addr] = data;
   else /* Sequential access device */
      return MEMPHY_seq_write(mp, addr, data);

   return 0;
}

/*
 *  MEMPHY_format-format MEMPHY device			(Dinh dang lai bo nho vat ly, tao cac khung trong (free frame list))
 *  @mp: memphy struct
 */
int MEMPHY_format(struct memphy_struct *mp, int pagesz)
{
    /* This setting come with fixed constant PAGESZ */
    int numfp = mp->maxsz / pagesz;
    struct framephy_struct *newfst, *fst;
    int iter = 0;

    if (numfp <= 0)
      return -1;

    /* Init head of free framephy list */ 
    fst = malloc(sizeof(struct framephy_struct));
    fst->fpn = iter;
    mp->free_fp_list = fst;

    /* We have list with first element, fill in the rest num-1 element member*/
    for (iter = 1; iter < numfp ; iter++)
    {
       newfst =  malloc(sizeof(struct framephy_struct));
       newfst->fpn = iter;
       newfst->fp_next = NULL;
       fst->fp_next = newfst;
       fst = newfst;
    }

    return 0;
}

/*
 *  MEMPHY_get_freefp-get a free frame page			(Lay 1 khung trong tu danh sach)
 *  @mp: memphy struct
 *  @retfpn: return fpn of frame page
 */
int MEMPHY_get_freefp(struct memphy_struct *mp, int *retfpn)
{
   struct framephy_struct *fp = mp->free_fp_list;

   if (fp == NULL)
     return -1;

   *retfpn = fp->fpn;
   mp->free_fp_list = fp->fp_next;

   /* MEMPHY is iteratively used up until its exhausted
    * No garbage collector acting then it not been released
    */
   free(fp);

   return 0;
}
/*
 *  MEMPHY_dump-dump memphy content		(In ra man hinh noi dung bo nho vat ly)
 *  @mp: memphy struct
 */
int MEMPHY_dump(struct memphy_struct * mp)
{
    /*TODO dump memphy content mp->storage 
     *     for tracing the memory content
     */
   if(mp == NULL || mp->storage == NULL) return -1;
   printf("MEMPHY_dump:\n");
   // for(int i = 0; i< mp->maxsz; i++){
   //    printf("Address %d: %d\n", i, mp->storage[i]);
   // }
   return 0;
}

/*
	Dua mot khung tro lai danh sach khung trong
	fpn : Chi so khung can dua tro lanh danh sach khung trong
*/
int MEMPHY_put_freefp(struct memphy_struct *mp, int fpn)
{
   struct framephy_struct *fp = mp->free_fp_list;
   struct framephy_struct *newnode = malloc(sizeof(struct framephy_struct));

   /* Create new node with value fpn */
   newnode->fpn = fpn;
   newnode->fp_next = fp;
   mp->free_fp_list = newnode;

   return 0;
}


/*
 *  Init MEMPHY struct		(Khoi tao cau truc memphy_struct)
 mp : Cau truc can khoi tao
 max_size : Kich thuoc toi da cua bo nho
 randomflg: Co chi dinh che do truy cap (Ngau nhien hoac tuan tuS)
 */
int init_memphy(struct memphy_struct *mp, int max_size, int randomflg)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;

   MEMPHY_format(mp,PAGING_PAGESZ);

   mp->rdmflg = (randomflg != 0)?1:0;

   if (!mp->rdmflg )   /* Not Ramdom acess device, then it serial device*/
      mp->cursor = 0;

   return 0;
}

//#endif
