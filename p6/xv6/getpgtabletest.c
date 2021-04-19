#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "ptentry.h"

#define PGSIZE 4096

void print_pgtable(struct pt_entry* pt_entries, int pages_num, int wset){
    int retval = getpgtable(pt_entries, pages_num, wset);
    if(retval == -1) {
        printf(1,"ERROR");
    }
    printf(1,"*****************\n");
    for (int i = 0; i < retval; i++) 
          printf(1, "XV6_TEST_OUTPUT Index %d: pdx: 0x%x, ptx: 0x%x, writable bit: %d, encrypted: %d, ref: %d\n", 
              i,
              pt_entries[i].pdx,
              pt_entries[i].ptx,
              pt_entries[i].writable,
              pt_entries[i].encrypted,
              pt_entries[i].ref
    );
    printf(1,"*****************\n");
}
int main(void) {
    char *ptr = sbrk(PGSIZE); // Allocate one page
    struct pt_entry pt_entry;
    // Get the page table information for newly allocated page
    // and the page should be encrypted at this point
    print_pgtable(&pt_entry, 1, 0);
    ptr[0] = 0x0;
    // The page should now be decrypted and put into the clock queue
    print_pgtable(&pt_entry, 1, 1);
    exit();
}