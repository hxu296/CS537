#include "types.h"
#include "stat.h"
#include "user.h"
#include "ptentry.h"

void
printpginfo(struct pt_entry *entries, int len){
    printf(1, "getpginfo.c: len: %d\n", len);
    for(int i = 0; i < len; i++){
        printf(1, "%d: pdx: %d, ptx: %d, ppage: %d, present: %d, "
                  "writable: %d, encrypted: %d\n", i, entries[i].pdx, entries[i].ptx,
                  entries[i].ppage, entries[i].present, entries[i].writable, entries[i].encrypted);
    }
}
int
main(void)
{
    char *ptr = sbrk(4096); // Allocate one page
    if(mencrypt(ptr, 1) == -1) {
        printf(2, "memcrypt failed\n");
        exit();
    }
    struct pt_entry entries;
    int len;
    if((len = getpgtable(&entries, 10)) == 0){
        printf(2, "getpgtable failed\n");
        exit();
    }
    printpginfo(&entries, len);
    exit();
}
