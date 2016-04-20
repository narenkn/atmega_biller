#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/*
** Translation Table as described in RFC1113
*/
static const uint8_t cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const uint8_t cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void
b64encode( uint8_t *in, uint8_t *out)
{
  out[3] = cb64[ (int)(in[2] & 0x3f) ];
  out[2] = cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ];
  out[1] = cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
  out[0] = cb64[ (int)(in[0] >> 2) ];
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
static void
encode( uint8_t *in, uint8_t *out )
{
  uint8_t len, in_idx, out_idx;

  for (len=12, in_idx=12, out_idx=16; len;
       len-=3, in_idx-=3, out_idx-=4) {
    b64encode (in+in_idx-3, out+out_idx-4);
  }
}

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void
b64decode( uint8_t *in, uint8_t *out )
{   
  out[ 0 ] = (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
static void
decode( uint8_t *in, uint8_t *out )
{
  uint8_t len, in_idx, out_idx;
  uint8_t ui1, v;

  for (len=0, in_idx=out_idx=0; len<12;
       len+=3, in_idx+=4, out_idx+=3) {
    for (ui1=0; ui1<4; ui1++) {
      v = in[in_idx+ui1];
      v = ((v < 43 || v > 122) ? (uint8_t) 0 : cd64[ v - 43 ]);
      if (v != 0) {
	v = ((v == (uint8_t)'$') ? 0 : v - 61);
	in[in_idx+ui1] = (uint8_t) (v - 1);
      } else {
	in[in_idx+ui1] = (uint8_t) 0;
      }
    }

    b64decode (in+in_idx, out+out_idx);
  }
}

int
main()
{
  uint8_t in1[12], out1[16], out2[12];
  uint8_t ui1;
  int     ui;
  uint32_t loop, seed;

  seed = time(NULL);
  srand(seed);

  printf("seed:0x%x\n", seed);
  for (loop=0; loop<1000000; loop++) {
    for (ui1=0; ui1<12; ui1++)
      in1[ui1] = rand();
    encode(in1, out1);
    decode(out1, out1);

    ui = strncmp(in1, out1, 12);
    if (0 != ui)
      printf("Error @loop:%d \n", loop);
      //      printf("'%s' vs '%s': %d\n", in1, out2, ui);
  }

  return 0;
}
