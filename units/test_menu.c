#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "kbd.c"
#include "menu.c"

#define KBD_SHIFT                0x80
#define KBD_KEY_TIMES(N)   ((N-1)<<4)
#define KBD_KEY(N)                  N

void *
f1(void *x)
{
  while (1) {
    get_key();
  }
  pthread_exit(0); 
}

void *
f2(void *x)
{
  menu_main();
  pthread_exit(0); 
}

int
main(void)
{
  pthread_t f2_thread, f1_thread; 

  pthread_create(&f1_thread,NULL,f1,NULL);
  pthread_create(&f2_thread,NULL,f2,NULL);

  pthread_join(f1_thread,NULL);
  pthread_join(f2_thread,NULL);

  return 0;
}

void
get_key(void)
{
  uint32_t val;

  printf(" 1   2   3  10\n");
  printf(" 4   5   6  Sh\n");
  printf(" 7   8   9  13\n");
  printf("11   0  12  13\n");
  scanf("%d", &val);
  if KBD_NOT_HIT {
    printf("Processing Kbd key %d\n", val);
    uint32_t v1 = val%100, v2=0;
    do {
      val /= 100;
      if ((v1 & 0xF) == (val%100)) v1 += 0x10;
      if (64 == (val%100)) v1 |= 0x80;
    } while (val);
    KbdData = v1;
  }
}
