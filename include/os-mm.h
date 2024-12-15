#include <stdint.h>
#ifndef OSMM_H
#define OSMM_H

#define MM_PAGING
#define PAGING_MAX_MMSWP 4 /* max number of supported swapped space */
#define PAGING_MAX_SYMTBL_SZ 30

typedef char BYTE;
typedef  uint32_t addr_t;
//typedef unsigned int uint32_t;

struct pgn_t{  // Quan ly danh sach cac so trang (Page number)
   int pgn;  // So trang hien tai
   struct pgn_t *pg_next; 	// Con tro den so trang tiep theo
};

/*
 *  Memory region struct	(Vung bo nho tu do)
 */
struct vm_rg_struct { 		
   int vmaid; // Id vung bo nho

   unsigned long rg_start; // Dia chi bat dau vung nho lien tuc
   unsigned long rg_end;	// Dia chi ket thuc

   struct vm_rg_struct *rg_next; // Con tro toi vung bo nho lien tuc tiep theo
};

/*
 *  Memory area struct		(Vung bo nho ao)
 */
struct vm_area_struct {
   unsigned long vm_id;		// ID cua vung bo nho ao
   unsigned long vm_start;	
   unsigned long vm_end;

   unsigned long sbrk;	// Con tro dieu khien su thay doi kich thuoc vung bo nho
/*
 * Derived field
 * unsigned long vm_limit = vm_end - vm_start
 */
   struct mm_struct *vm_mm;		// Tro toi cau truc quan ly bo nho cua tien trinh
   struct vm_rg_struct *vm_freerg_list;		// Danh sach quan ly cac vung bo nho tu do (vm_rg_struct) trong  vung bo nho ao nay 
   struct vm_area_struct *vm_next;		// Con tro toi vung bo nho ao tiep theo
};

/* 
 * Memory management struct			(Cau truc quan ly bo nho cua chuong trinh)
 */
struct mm_struct {
   uint32_t *pgd;  // Tro toi thu muc trang cua toi trinh
	// Thu muc trang(Page Directory) : Chinh la bang trang , noi chua cac muc (entry) anh xa dia chi ao thanh dia chi vat ly 
   struct vm_area_struct *mmap; // Danh sach cac vung bo nho ao (vm_area_struct) ma tien trinh  dang su dung
  
   /* Currently we support a fixed number of symbol */
   struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ];		//Bang bieu tuong dung de luu tru cac vung bo nho duoc danh dau trong chuong trinh

   /* list of free page */
   struct pgn_t *fifo_pgn;				// Danh sach cac so trang dang duoc su dung trong thuat toan thay the trang fifo
};

/*
 * FRAME/MEM PHY struct				(Khung bo nho vat ly)
 */
struct framephy_struct { 
   int fpn;				// So khung cua bo nho vat ly
   struct framephy_struct *fp_next;	//	Con tro toi khung bo nho tiep theo

   /* Resereed for tracking allocated framed */
   struct mm_struct* owner;	//Cau truc mm dang so huu khung nay
};

/*
	Bo nho vat ly
*/
struct memphy_struct {
   /* Basic field of data and size */
   BYTE *storage;  // Mang luu tru du lieu bo nho vat ly
   int maxsz;	//Kich thuoc toi da cua bo nho vat ly
   
   /* Sequential device fields */ 
   int rdmflg;	// Co chi dinh bo nho co ho tro tuy cap ngau nhien khong
   int cursor;	// Con tro dung de theo doi vi tri trong bo nho neu dang su dung truy cap tuan tu

   /* Management structure */
   struct framephy_struct *free_fp_list;	// Danh sach cac khung bo nho trong
   struct framephy_struct *used_fp_list;	// Danh sach cac khung bo nho da su dung
};

#endif
