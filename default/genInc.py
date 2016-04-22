#! python

from optparse import OptionParser
from ctypes import *
from optparse import SUPPRESS_HELP
import time
import re
from sys import exit

## Pin assignments
##   LPT has pins 1-25
##   D[7:0], S[7:0]
## LPT5, D3 - Serial program enable
## LPT6, D4 - SCLK
## LPT7, D5 - MOSI
## LPT9, D7 - RESET
## LPT10, S6 - MISO

class Atmega32:

  ## 32K with each page 64 words
  PAGE_SIZE = 128;
  NUM_PAGES = 32*8; ## 32K bytes (32*1024/128)

  def __init__(self):
    self.error = 0;

  def GenCpp(self, options):
    ## Generate Empty pages for holes
    holeStat = 0;
    for i in range(self.NUM_PAGES-1, -1, -1):
      if None == self.pages[i]:
        if (0 == (holeStat & 1)): continue;
        self.pages[i] = [0xFF] * dev.PAGE_SIZE;
      else:
        holeStat = 1;
    f1=open(options.output, 'w');
    print "Info: Generating CPP Header %s" % options.output,;
    f1.write("#define SPM_PAGESIZE      %d\n" % self.PAGE_SIZE)
    f1.write("#define NUM_PAGES         %d\n" % self.NUM_PAGES);
    f1.write("typedef uint32_t    address_t;\n");
    f1.write("typedef uint8_t     value_t;\n");
    f1.write("typedef	std::array<value_t, SPM_PAGESIZE> page_t;\n");
    f1.write("typedef	std::map<const address_t, const page_t> hexFile_t;\n");
    f1.write("\n");
    f1.write("\nhexFile_t hexFile = {\n");
    printStat = 0;
    for i in range(self.NUM_PAGES):
      if None != self.pages[i]:
        assert (i < (self.NUM_PAGES-4)); ## Last 4 pages reserved
        print ".",;
        if (printStat & 1):
          f1.write(",\n");
        printStat = 1;
        f1.write("{");
        f1.write("0x%x,\n{ " % (i << 7));
	for j in range(self.PAGE_SIZE):
          if (printStat & 2):
            f1.write(", ");
          printStat |= 2;
          f1.write("0x%x" % self.pages[i][j]);
        f1.write("}\n}");
    f1.write("\n};\n\n");
    f1.close();

##
def crc16(string, crc=0):
  for a in string:
    crc ^= a
    for i in range(8):
      if crc & 1:
        crc = (crc >> 1) ^ 0xA001
      else:
        crc = (crc >> 1)
  return crc

if "__main__" == __name__:
  ## Constants
  REC_DATA = 0;
  REC_EOF = 1;
  REC_EX_SEGADDR = 2;
  REC_ST_SEGADDR = 3;
  REC_EX_LIADDR  = 4;
  REC_ST_LIADDR  = 5;

  ##
  parser = OptionParser()
  parser.add_option("-f", "--file", dest="ihex", default="main.hex",
                  help="Default program file");
  parser.add_option("", "--mcu", dest="mcu", default="atmega32",
                  help=SUPPRESS_HELP); ## atmega1284p
  parser.add_option("-o", "--output", dest="output", default="lptprog.out",
                  help=SUPPRESS_HELP); ## atmega1284p
  (options, args) = parser.parse_args()

  ## modify certain default values
  if 'atmega32' == options.mcu:
    dev = Atmega32();
  else:
    assert(0);

  ##
  dev.pages = [None]*dev.NUM_PAGES;
  dev.error = 0;

  ##
  f = open(options.ihex);
  if not f:
    print "Error: Failed opening file";
    exit (1);

  ## Process file
  any_page_valid = False;
  valid_line = re.compile(r'^:(\w\w)(\w\w\w\w)(\w\w)(\w+)(\w\w)$');
  segment_addr = 0;
  while 1:
    l = f.readline();
##    print l;
    if not l: break;
    m = valid_line.match(l);
    if m:
      byte_count = int(m.group(1), 16);
      record_type = int(m.group(3), 16);
      address = segment_addr + int(m.group(2), 16);
##      print "address is ", address, " bytes:", byte_count;
      page = address >> 7; ## 128 bytes per page
      address &= 0x7F;
      ## Compute checksum
      checksum = 0;
      for i in range(1, len(l)-3, 2):
        checksum += int(l[i:(i+2)], 16);
      checksum = ((checksum ^ 0xFF) + 1) & 0xFF;
      if checksum != int(m.group(5), 16):
        print "Error: checksum:0x%x obtained:0x%x" % (checksum, int(m.group(5), 16));
        exit (4);
      ##
      if REC_DATA == record_type:
        for i in range(9, len(l)-3, 2):
          if address >= dev.PAGE_SIZE:
            page += 1;
            address = 0;
          if not dev.pages[page]:
            any_page_valid = True;
            dev.pages[page] = [0xFF] * dev.PAGE_SIZE;
          dev.pages[page][address] = int(l[i:(i+2)], 16);
##          print "%d = 0x%x" % (address, int(l[i:(i+2)], 16));
          address += 1;
      elif REC_EX_SEGADDR == record_type:
        segment_addr = int(m.group(4), 16) << 4;
      elif REC_ST_SEGADDR == record_type:
        segment_addr = int(m.group(4), 16) << 4;
      elif REC_EX_LIADDR == record_type:
        segment_addr = int(m.group(4), 16) << 16;
      elif REC_EOF == record_type:
        break;
      else:
        print "Error : Unknown record type: ", record_type;
        exit (3);

  ## Only program if valid lines are available in file
  dev.error = 0;
  if any_page_valid:
    dev.GenCpp(options);
  else:
    print "No valid programming found in hex file";

  exit (dev.error);
