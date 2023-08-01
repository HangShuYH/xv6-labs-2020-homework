// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define STEAL_PAGES 1
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for (int i = 0;i < NCPU;i++) {
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  int i = 0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE, i++)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off();
  int i = cpuid();
  acquire(&kmem[i].lock);
  r->next = kmem[i].freelist;
  kmem[i].freelist = r;
  release(&kmem[i].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int i = cpuid();
  acquire(&kmem[i].lock);
  r = kmem[i].freelist;
  if(r) {
    kmem[i].freelist = r->next;
    release(&kmem[i].lock);
  } else {
    release(&kmem[i].lock); 
    int id = 0;
    for (id = 0;id < NCPU; id++) {
      if (i == id) continue;
      acquire(&kmem[id].lock);
      if (!kmem[id].freelist) {
        release(&kmem[id].lock);
        continue;
      }
      r = kmem[id].freelist;
      kmem[id].freelist = kmem[id].freelist->next;

      struct run* steal_begin = kmem[id].freelist;
      struct run* steal_end = kmem[id].freelist;
      int cnt = 1;
      while(steal_end && cnt < STEAL_PAGES) {
        steal_end = steal_end->next;
        cnt++;
      }
      //steal_end is the end or cnt == STEAL_PAGES
      acquire(&kmem[i].lock);
      kmem[i].freelist = steal_begin;
      release(&kmem[i].lock);
      if (steal_end) {
        kmem[id].freelist = steal_end->next;
        steal_end->next = 0;
      } else {
        kmem[id].freelist = 0;
      }
      release(&kmem[id].lock);
      break;
    }
    
  }
  pop_off();
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
