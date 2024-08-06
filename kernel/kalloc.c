// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);
void kfree2(void* pa, int cpu);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem {
  struct spinlock lock;
  struct run *freelist;
};

struct kmem kmem[NCPU];

void
kinit()
{
  for(int i = 0; i < NCPU; ++i) {
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);

  uint64 total_mem_per_cpu = ((uint64)pa_end - (uint64)pa_start) / NCPU;

  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    // not actually needed this way but i prefer equitable distrib here
    kfree2(p, (int)(((uint64)p - (uint64)pa_start) / total_mem_per_cpu));
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // calling and using cpuid needs interrups off
  // internet solution hasn't used this and it breaks test3
  // it passes but there is a usertrap
  push_off();
  int cpu = cpuid();
  kfree2(pa, cpu);
  pop_off();
}

void
kfree2(void* pa, int cpu)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  push_off();
  int cpu = cpuid();
  
  struct run *r;

  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;

  if(r)
    kmem[cpu].freelist = r->next;
  release(&kmem[cpu].lock);

  if(!r) {
    for(int i = cpu + 1;; ++i) { 
      if(i == NCPU)
        i = 0;
      // printf("%d %d\n", i, cpu);
      if(i == cpu)
        break;
      // steal free pages
      acquire(&kmem[i].lock);
      r = kmem[i].freelist;
      if(r) {
        kmem[i].freelist = r->next;
        r->next = 0; // not necessary but makes it clearer
        release(&kmem[i].lock);     
        break;
      }
      release(&kmem[i].lock);     
    }
  }
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
