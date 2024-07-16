#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int g(int x) {
  return x+3;
}

int f(int x) {
  return g(x);
}

void main(void) {
  int a[5];
  a[0] = 3;
  for(int j = 1; j < 5; j++)
    a[j] = a[j - 1] * 2; 
  printf("%d", a);
  printf("x=%d y=%d", 3);
  unsigned int i = 0x00646c72;
	printf("H%x Wo%s", 57616, &i);
  printf("%d %d\n", f(8)+1, 13);
  exit(0);
}
