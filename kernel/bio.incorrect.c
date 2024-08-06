//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.



// NOTE: This method is wrong, see the other note 
// Also problem in locks: the buf may not always be in the 
// same bucket with this method.

// This defeates the purpose for bpin, bunpin and brelse 
// (have to make a global bcache_lock) which is used
// less but still is used concurrently, so contentions are there

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETS 13
#define NBUFS NBUF / BUCKETS

struct bcache {
  struct spinlock lock;
  struct buf buf[NBUFS];
};

struct bcache bcaches[BUCKETS];
struct spinlock bcache_lock;

void
binit(void)
{
  struct bcache *bcache;
  initlock(&bcache_lock, "bcache");
  for(int i = 0; i < BUCKETS; ++i) {
    bcache = &bcaches[i];
    initlock(&bcache->lock, "bcache.bucket");

    // Create linked list of buffers, remove LRU initz
    for(int j = 0; j < NBUFS; ++j){
      initsleeplock(&bcache->buf[j].lock, "buffer");
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  
  // printf("%d %d\n", dev, blockno);
  struct buf *b, *freebuf;
  struct bcache* bcache;
  
  for(int i = 0; i < BUCKETS; ++i){
    bcache = &bcaches[(blockno + i) % BUCKETS];
    acquire(&bcache->lock);

    // Is the block already cached?
    for(int j = 0; j < NBUFS; ++j) {
      b = &bcache->buf[j];
      if(b->dev == dev && b->blockno == blockno){
        b->refcnt++;
        release(&bcache->lock);
        acquiresleep(&b->lock);
        return b;
      }
      else if(b->refcnt == 0)
        freebuf = b;
    }

    // Not cached.
    // NOTE: This is wrong, in a later check it might find a freebuf in another bucket
    // and not come back to this very allocated position
    if(freebuf) {
      b = freebuf;
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache->lock);
      acquiresleep(&b->lock);
      return b;
    }
    release(&bcache->lock);
  }

  // Need to forcefully remove
  // b = &bcache->buf[0];
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache_lock);
  b->refcnt--;
  release(&bcache_lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache_lock);
  b->refcnt++;
  release(&bcache_lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache_lock);
  b->refcnt--;
  release(&bcache_lock);
}


