void
error(void)
{

  printf("error: TWI status %#x\n", twst);
  exit(0);
}

FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void
main(void)
{
  uint16_t a;
  int rv;
  uint8_t b[16];
  uint8_t x;

  ep_store_init();

  stdout = &mystdout;

  for (a = 0; a < 256;)
    {
      printf("%#04x: ", a);
      rv = ee24xx_read_bytes(a, 16, b);
      if (rv <= 0)
	error();
      if (rv < 16)
	printf("warning: short read %d\n", rv);
      a += rv;
      for (x = 0; x < rv; x++)
	printf("%02x ", b[x]);
      putchar('\n');
    }
#define EE_WRITE(addr, str) ee24xx_write_bytes(addr, sizeof(str)-1, str)
  rv = EE_WRITE(55, "The quick brown fox jumps over the lazy dog.");
  if (rv < 0)
    error();
  printf("Wrote %d bytes.\n", rv);
  for (a = 0; a < 256;)
    {
      printf("%#04x: ", a);
      rv = ee24xx_read_bytes(a, 16, b);
      if (rv <= 0)
	error();
      if (rv < 16)
	printf("warning: short read %d\n", rv);
      a += rv;
      for (x = 0; x < rv; x++)
	printf("%02x ", b[x]);
      putchar('\n');
    }

  printf("done.\n");

}
