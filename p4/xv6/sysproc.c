#include "pstat.h"
#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork2(myproc()->timeslice);
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_setslice()
{
    int pid;
    int timeslice;

    if(argint(0, &pid) < 0 || argint(1, &timeslice) < 0 || timeslice < 1)
        return -1;

    return setslice(pid, timeslice);
}

int sys_getslice()
{
    int pid;

    if(argint(0, &pid) < 0)
        return -1;

    return getslice(pid);
}

int sys_getpinfo(){
    struct pstat *pst;

    if(argptr(0, (void*)&pst , sizeof(*pst)) < 0 || pst == 0)
        return -1;

    return getpinfo(pst);
}

int sys_fork2(){
    int timeslice;

    if(argint(0, &timeslice) < 0 || timeslice < 1)
        return -1;

    return fork2(timeslice);
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;

  if(argint(0, &n) < 0)
    return -1;

  //cprintf("sysproc: %d sleeps for %d\n", myproc()->pid, n);

  acquire(&tickslock);

  if(myproc()->killed){
    release(&tickslock);
    return -1;
  }

  sleep2(&ticks, &tickslock, n);

  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
