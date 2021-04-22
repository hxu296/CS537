#include "types.h"
#include "user.h"

#define PGSIZE 4096

/*
 * To use this program, first add code to growproc() in proc.c
 * that prints the old and new values of "sz". Also add a print statement
 * to allocuvm every time a new page is allocated.
 *
 * Then observe the results when calling sbrk() with different values.
 */

int
main(int argc, char *argv[])
{
  printf(1, "\nsbrk(PGSIZE)\n");
  sbrk(PGSIZE);

  printf(1, "\nsbrk(PGSIZE / 2)\n");
  sbrk(PGSIZE / 2);

  printf(1, "\nsbrk(PGSIZE)\n");
  sbrk(PGSIZE);

  printf(1, "\nsbrk(1)\n");
  sbrk(1);

  printf(1, "\nsbrk((PGSIZE / 2) - 1)\n");
  sbrk((PGSIZE / 2) - 1);

  printf(1, "\n");
  exit();
  return 0;
}
