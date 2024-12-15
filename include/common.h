#ifndef COMMON_H
#define COMMON_H

/* Define structs and routine could be used by every source files */

#include <stdint.h>

#ifndef OSCFG_H
#include "os-cfg.h"
#endif

#ifndef OSMM_H
#include "os-mm.h"
#endif

#define ADDRESS_SIZE	20
#define OFFSET_LEN	10
#define FIRST_LV_LEN	5
#define SECOND_LV_LEN	5
#define SEGMENT_LEN     FIRST_LV_LEN
#define PAGE_LEN        SECOND_LV_LEN

#define NUM_PAGES	(1 << (ADDRESS_SIZE - OFFSET_LEN))
#define PAGE_SIZE	(1 << OFFSET_LEN)

enum ins_opcode_t {
	CALC,	// Just perform calculation, only use CPU
	ALLOC,	// Allocate memory
#ifdef MM_PAGING
	MALLOC, // Allocate dynamic memory
#endif
	FREE,	// Deallocated a memory block
	READ,	// Write data to a byte on memory
	WRITE	// Read data from a byte on memory
};

/* instructions executed by the CPU (Cau truc lenh) */
struct inst_t {
	enum ins_opcode_t opcode;  // Ma lenh , xac dinh thao tac ma CPU can thuc hien
	uint32_t arg_0; // Argument lists for instructions 
	uint32_t arg_1;
	uint32_t arg_2;
	// (arg : cac tham so lenh, tuy thuoc vao lenh co the co 2 den 3 tham so)
};

/* Phan doan ma */
struct code_seg_t { 
	struct inst_t * text;	// Con tro den mot mang cac lenh (inst_t)
	uint32_t size;			// So luong lenh trong text
};

struct trans_table_t { // Bang anh xa trang (Phan trang 2 lop)
	/* A row in the page table of the second layer */
	/* Bang chuyen doi tu dia chi ao sang dia chi vat ly */
	struct  {
		addr_t v_index; // The index of virtual address (Chi so cua trang ao)
		addr_t p_index; // The index of physical address (Chi so khung trang vat ly)
	} table[1 << SECOND_LV_LEN]; // Mot mang chua cac muc  trong bang anh xa, moi muc la mot chuyen doi tu bo nho ao sang bo nho vat ly
	int size; // So luong muc trong bang anh xa
};

/* Mapping virtual addresses and physical ones */
struct page_table_t {
	/* Translation table for the first layer */
	/* Bang Trang */
	struct {
		addr_t v_index;	// Virtual index (Chi so ao cua vung bo nho ,segment (VD : heap, stack , code))
		struct trans_table_t * next_lv;  // Con tro den bang anh xa cap thap hon (o day cap thap hon la mot trand_table_t).
	} table[1 << FIRST_LV_LEN]; // Mang chua cac muc, moi muc la mot con tro den cap thap hon (bang anh xa trans_table_t tuong ung voi segment)
	int size;	// Number of row in the first layer (So luong Segment)
};

/* PCB, describe information about a process  (Cau truc PCB)*/
struct pcb_t {
	uint32_t pid;	// PID	(ID cua tien trinh)
	uint32_t priority; // Default priority, this legacy (FIXED) value depend on process itself (Gia tri uu tien mac dinh cua tien trinh)
	// Gia tri uu tien mac dinh co the bi thay doi boi gia tri uu tien dong (prio)
	struct code_seg_t * code;	// Code segment (Con tro den phan doan ma cua tien trinh. Noi day chua ma thuc thi)
	addr_t regs[10]; // Registers, store address of allocated regions (Mang cac thanh ghi, luu tru dia chi cac vung bo nho duoc cap phat cho tien trinh)
	uint32_t pc; // Program pointer, point to the next instruction	(Con tro chuong trinh , tro toi lenh tiep theo ma CPU thuc thi)
#ifdef MLQ_SCHED
	// Priority on execution (if supported), on-fly aka. changeable
	// and this vale overwrites the default priority when it existed
	uint32_t prio;     // Gia tri uu tien dong cua tien trinh , gia tri nay co the thay doi gia tri uu tien mac dinh
#endif
#ifdef MM_PAGING
	struct mm_struct *mm; // Con tro den cau truc quan ly bo nho (mm_struct)
	struct memphy_struct *mram; // Con tro den memphystruct , dai dien bo nho vat ly cua he thong
	struct memphy_struct **mswp; //Mang con tro den memphystrcut , quan ly cac bo nho thay the (swap)
	struct memphy_struct *active_mswp; // Con tro den bo nho swap dang hoat dong
#ifdef MM_PAGING_HEAP_GODOWN
	uint32_t vmemsz;	// Kich thuoc bo nho ao cua tien trinh (Co the chi dai dien cho heap)
#endif
#endif
	struct page_table_t * page_table; // Page table (Con tro den cau truc quan ly bang trang cua tien trinh)
	uint32_t bp;	// Break pointer	(Quan ly bo nho heap cua tien trinh)

};

#endif

