#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 base;
  int len;
  uint64 mask;

  argaddr(0, &base);
  argint(1, &len);
  argaddr(2, &mask);

  vmprint(myproc()->pagetable);
  
  printf("%p, %d, %p\n", base, len, mask);

  if(len > 64)
    return -1;
  
  uint64 abits = 0;

  pagetable_t pagetable = myproc()->pagetable;

  for(int i = 0 ; i < len ; ++i) {
    uint64 curr_va = (base + PGSIZE * i);
    pte_t *pte = walk(pagetable, curr_va, 0);
    printf("At %p, itr %d\n", curr_va, i);
    if(*pte & PTE_A) {

      printf("%p is accessed\n", curr_va);
      abits = (uint64)abits | (1 << i);

      // unset after using
      *pte &= (~PTE_A);
    }
  }

  printf("%p\n", abits);
  return copyout(myproc()->pagetable, mask, (char *)&abits, sizeof(uint64));
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
