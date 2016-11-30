#include "common_incl.c"

uint32_t errors = 0;

int
main(int argc, char *argv[])
{
  uint32_t ui1;

  if ((argc == 1) || (0 == argv[1]))
    ui1 = time(NULL);
  else
    ui1 = atoi(argv[1]);
  printf("seed : %d\n", ui1);
  srand(ui1);

  date_t rdate; s_time_t rtime;
  for (uint32_t loop=0; loop<1000; loop++) {
    RESET_TEST_KEYS;

    /* rand date */
    rdate.month = (rand() % 12)+1;
    rdate.year = 2000 + rand() % 100;
    rdate.day = (rand() % 28)+1;
    sprintf(inp, "%02d%02d%04d", rdate.day, rdate.month, rdate.year);
    //printf("inp:'%s'\n", inp);
    INIT_TEST_KEYS(inp);

    /* rand time */
    rtime.hour = rand() % 24;
    rtime.min = rand() % 60;
    rtime.sec = rand() % 60;
    sprintf(inp2, "%02d%02d%02d", rtime.hour, rtime.min, rtime.sec);
    //printf("inp2:'%s'\n", inp2);
    INIT_TEST_KEYS(inp2);

    /* */
    menuSetDateTime(0);
    assert(i2c_date.day == rdate.day);
    assert(i2c_date.month == rdate.month);
    assert(i2c_date.year == rdate.year);
    assert(i2c_time.hour == rtime.hour);
    assert(i2c_time.min == rtime.min);
    assert(i2c_time.sec == rtime.sec);
  }

  return errors;
}
