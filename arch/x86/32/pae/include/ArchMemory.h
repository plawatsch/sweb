/**
 * @file ArchMemory.h
 *
 */

#ifdef CMAKE_X86_32_PAE
#ifndef _ARCH_MEMORY_H_
#define _ARCH_MEMORY_H_

#include "types.h"
#include "paging-definitions.h"

#define RESOLVEMAPPING(pdpt,vpage) ;\
  uint32 pdpte_vpn = vpage / (PAGE_TABLE_ENTRIES * PAGE_DIRECTORY_ENTRIES);\
  PageDirEntry* page_directory = (PageDirEntry*) ArchMemory::getIdentAddressOfPPN(pdpt[pdpte_vpn].page_directory_ppn);\
  uint32 pde_vpn = (vpage % (PAGE_TABLE_ENTRIES * PAGE_DIRECTORY_ENTRIES)) / PAGE_TABLE_ENTRIES;\
  uint32 pte_vpn = (vpage % (PAGE_TABLE_ENTRIES * PAGE_DIRECTORY_ENTRIES)) % PAGE_TABLE_ENTRIES;

/**
 *
 * Collection of architecture dependant functions concerning Memory and Pages
 *
 */
class ArchMemory
{
public:

/**
 * constructor: initializes a new pdpt
 * @param pdpt
 */
  ArchMemory();

/** 
 *
 * maps a virtual page to a physical page (pde and pte need to be set up first)
 *
 * @param physical_page_directory_page Real Page where the PDE to work on resides
 * @param virtual_page 
 * @param physical_page
 * @param user_access PTE User/Supervisor Flag, governing the binary Paging
 * Privilege Mechanism
 * @param page_size Optional, defaults to 4k pages, but you ned to set it to
 * 1024*4096 if you want to map a 4m page
 */
  void mapPage(uint32 virtual_page, uint32 physical_page, uint32 user_access, uint32 page_size=PAGE_SIZE);

/**
 * removes the mapping to a virtual_page by marking its PTE Entry as non valid
 *
 * @param physical_page_directory_page Real Page where the PDE to work on resides
 * @param virtual_page which will be invalidated
 */
  void unmapPage(uint32 virtual_page);

  /**
   * Destructor. Recursively deletes the page directory and all page tables
   *
   */
    ~ArchMemory();

  /**
   * recursively remove a PageDirectoryEntry and all its Pages and PageTables
   *
   * @param physical_page_directory_page of PDE to remove
   */
    void freePageDirectory(uint32 physical_page_directory_page);

/**
 * Takes a Physical Page Number in Real Memory and returns a virtual address than
 * can be used to access given page
 * @param ppn Physical Page Number
 * @param page_size Optional, defaults to 4k pages, but you ned to set it to
 * 1024*4096 if you have a 4m page number
 * @return Virtual Address above 3GB pointing to the start of a memory segment that
 * is mapped to the physical page given
 */
  static pointer getIdentAddressOfPPN(uint32 ppn, uint32 page_size=PAGE_SIZE)
  {
    return (3U*1024U*1024U*1024U) + (ppn * page_size);
  }

/**
 * Checks if a given Virtual Address is valid and is mapped to real memory
 * @param pdpt page dir pointer table
 * @param vaddress_to_check Virtual Address we want to check
 * @return true: if mapping exists\nfalse: if the given virtual address is unmapped
 * and accessing it would result in a pageFault
 */
  bool checkAddressValid(uint32 vaddress_to_check);

/**
 * Takes a virtual_page and search through the pageTable and pageDirectory for the
 * physical_page it refers to
 * to get a physical address (which you can only use by adding 3gb to it) multiply
 * the &physical_page with the return value
 * @param virtual_page virtual Page to look up
 * @param *physical_page Pointer to the result
 * @param *physical_pte_page optional: Pointer to physical page number of used PTE, or unchanged if 4MiB Page
 * @return 0: if the virtual page doesn't map to any physical page\notherwise
 * returns the page size in byte (4096 for 4KiB pages or 4096*1024 for 4MiB pages)
 */
  static uint32 get_PPN_Of_VPN_In_KernelMapping(uint32 virtual_page, size_t *physical_page, uint32 *physical_pte_page=0);
  static uint32 get_PAddr_Of_VAddr_In_KernelMapping(uint32 virtual_addr);
  PageDirPointerTableEntry* page_dir_pointer_table_;

  PageDirPointerTableEntry* getRootOfPagingStructure();

  static const size_t RESERVED_START = 0x80000ULL;
  static const size_t RESERVED_END = 0xC0000ULL;

private:

  void insertPD(uint32 pdpt_vpn, uint32 physical_page_directory_page);
/** 
 * Adds a page directory entry to the given page directory.
 * (In other words, adds the reference to a new page table to a given
 * page directory.)
 *
 * @param physical_page_directory_page physical page containing the target PD.
 * @param pde_vpn Index of the PDE (i.e. the page table) in the PD.
 * @param physical_page_table_page physical page of the new page table.
 */
  void insertPT(PageDirEntry* page_directory, uint32 pde_vpn, uint32 physical_page_table_page);

/**
 * Removes a page directory entry from a given page directory if it is present
 * in the first place. Futhermore, the target page table is assured to be
 * empty.
 *
 * @param physical_page_directory_page physical page containing the target PD.
 * @param pde_vpn Index of the PDE (i.e. the page table) in the PD.
 */
  void checkAndRemovePT(uint32 physical_page_directory_page, uint32 pde_vpn);

  PageDirPointerTableEntry page_dir_pointer_table_space_[2 * PAGE_DIRECTORY_POINTER_TABLE_ENTRIES];
  // why 2* ? this is a hack because this table has to be aligned to its own
  // size 0x20... this way we allow to set an aligned pointer in the constructor.
  // but why cant we just use __attribute__((aligned(0x20))) ? i'm not sure...
  // i think because were in a class and the object will be allocated by the
  // KMM which will just return some not-aligned block, thus this aligned block
  // gets not-aligned in memory -- DG
};

#endif
#endif
