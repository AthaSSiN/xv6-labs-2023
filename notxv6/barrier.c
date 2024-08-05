#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

static int nthread = 1;
static int round = 0;

struct barrier {
  pthread_mutex_t barrier_mutex;
  pthread_cond_t barrier_cond;
  int nthread;      // Number of threads that have reached this round of the barrier
  int round;     // Barrier round
} bstate;

static void
barrier_init(void)
{
  assert(pthread_mutex_init(&bstate.barrier_mutex, NULL) == 0);
  assert(pthread_cond_init(&bstate.barrier_cond, NULL) == 0);
  bstate.nthread = 0;
  bstate.round = 0; // is this needed?
}

static void 
barrier()
{
  // YOUR CODE HERE
  //
  // Block until all threads have called barrier() and
  // then increment bstate.round.
  //

  pthread_mutex_lock(&bstate.barrier_mutex);
  
  // helps hold a thread which has raced to this point, as we wait till nthread is zero again 
  // (first check is for round 0 and if someone passes through, for others to wakeup and pass)
  while(bstate.round > round && bstate.nthread > 0) 
    pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);

  // locks here so that only one thread at a time can check and update
  // putting locks inside might cause multple incorect increaments in all cases
  // I don't think it will be wrong in this case though, will see
  
  // WRONG: No need for separate locks, cond_wait anyway releases locks held earlier
  if(bstate.nthread == 0 && bstate.round > round){
    round++;
    pthread_cond_broadcast(&bstate.barrier_cond); // as there are two sleeps, we will need two wakeups
  }
  // update the nthread
  bstate.nthread++;
  
  // second statement is to wait till nthreads are the same
  // first statement to let the woken up processes pass through
  while(bstate.round <= round && bstate.nthread < nthread)
    pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);


  if(bstate.nthread == nthread) {
    // can start next round now
    bstate.round++;
    pthread_cond_broadcast(&bstate.barrier_cond);
    // bstate.nthread = 0;
  }
  bstate.nthread--;
  
  // if (++bstate.nthread < nthread) {
  //   pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
  // } else {
  //   bstate.nthread = 0;
  //   bstate.round++;
  //   pthread_cond_broadcast(&bstate.barrier_cond);
  // }
  
  pthread_mutex_unlock(&bstate.barrier_mutex);
  
}

static void *
thread(void *xa)
{
  long n = (long) xa;
  long delay;
  int i;

  for (i = 0; i < 20000; i++) {
    int t = bstate.round;
    assert (i == t);
    barrier();
    // printf("%ld: %d, %d\n", n, i, bstate.nthread);
    usleep(random() % 100);
  }

  return 0;
}

int
main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2) {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);

  barrier_init();

  for(i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, thread, (void *) i) == 0);
  }
  for(i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  printf("OK; passed\n");
}
