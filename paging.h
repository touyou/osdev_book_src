/*
 *            paging.h
 * written by Shinichi Awamoto, 2017
 *
 * ページング周りの定義
 */

#pragma once

#define PML4E_PRESENT_BIT          (1<<0)
#define PML4E_WRITE_BIT            (1<<1)
#define PML4E_USER_BIT             (1<<2)
#define PML4E_WRITETHROUGH_BIT     (1<<3)
#define PML4E_CACHEDISABLE_BIT     (1<<4)
#define PML4E_ACCESSED_BIT         (1<<5)

#define PDPTE_PRESENT_BIT          (1<<0)
#define PDPTE_WRITE_BIT            (1<<1)
#define PDPTE_USER_BIT             (1<<2)
#define PDPTE_WRITETHROUGH_BIT     (1<<3)
#define PDPTE_CACHEDISABLE_BIT     (1<<4)
#define PDPTE_ACCESSED_BIT         (1<<5)
#define PDPTE_DIRTY_BIT            (1<<6)  // 1GPAGE_BIT==1の時のみ
#define PDPTE_1GPAGE_BIT           (1<<7)
#define PDPTE_GLOBAL_BIT           (1<<8)  // 1GPAGE_BIT==1の時のみ

#define PDE_PRESENT_BIT            (1<<0)
#define PDE_WRITE_BIT              (1<<1)
#define PDE_USER_BIT               (1<<2)
#define PDE_WRITETHROUGH_BIT       (1<<3)
#define PDE_CACHEDISABLE_BIT       (1<<4)
#define PDE_ACCESSED_BIT           (1<<5)
#define PDE_DIRTY_BIT              (1<<6)  // 2MPAGE_BIT==1の時のみ
#define PDE_2MPAGE_BIT             (1<<7)
#define PDE_GLOBAL_BIT             (1<<8)  // 2MPAGE_BIT==1の時のみ

#define PTE_PRESENT_BIT            (1<<0)
#define PTE_WRITE_BIT              (1<<1)
#define PTE_USER_BIT               (1<<2)
#define PTE_WRITETHROUGH_BIT       (1<<3)
#define PTE_CACHEDISABLE_BIT       (1<<4)
#define PTE_ACCESSED_BIT           (1<<5)
#define PTE_DIRTY_BIT              (1<<6)
#define PTE_GLOBAL_BIT             (1<<8)

#ifndef ASM_FILE

#endif // ASM_FILE
