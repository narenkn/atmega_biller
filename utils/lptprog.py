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

  p = WinDLL("io");
  def_val = 0x0;
  ## 32K with each page 64 words
  PAGE_SIZE = 128;
  NUM_PAGES = 32*8; ## 32K bytes (32*1024/128)

  def Out(self, byte):
    in_val = 0;
    for i in range(8):
      in_val <<= 1;
      self.p.PortOut(self.dat_reg, self.def_val|((byte>>(7-i)<<5)&0x20));
      self.p.PortOut(self.dat_reg, self.def_val|((byte>>(7-i)<<5)&0x20)|0x10);
      in_val |= (self.p.PortIn(self.sta_reg)>>6) & 0x1;
      self.p.PortOut(self.dat_reg, self.def_val);
    return (in_val & 0xFF);

  def RawOut(self, byte):
    self.p.PortOut(self.dat_reg, self.def_val|byte);
    return 0;

  def Reset(self):
    ## Simple delay to release the port
    self.def_val = 0x0;
    for i in range(8):
      self.RawOut(0x0);
    ## Enable the port
    self.def_val = 0x0;
    self.RawOut(0x0);
    ## Toggle Reset
    self.RawOut(0x80);
    self.RawOut(0x0);

  def Release(self):
    self.RawOut(0x0);
    self.def_val = 0xFF;
    self.RawOut(0xFF);
    self.p.PortOut(self.cnt_reg, 0x0);

  def __init__(self, dr):
    self.error = 0;
    self.dat_reg = dr;
    self.sta_reg = dr+1;
    self.cnt_reg = dr+2;
    self.def_val = 0x8;

  def TestPins(self):
    self.Reset();
    self.RawOut(0x0);

  def ProgEn(self, options):
    ## 1. Reset
    self.Reset();
    ## 2. Get acknowledgement of programming
    self.Out(0xAC);
    self.Out(0x53);
    in_val = self.Out(0x00);
    self.Out(0x00);
    ## 3. Check for ack before proceeding
    if (0x53 != in_val):
      print "Error: Couldn't communicate with device(0x%x), check cable" % in_val;
      return;
    else:
      print "Info: Detected device connected...";
    ## 4. Read signature byte of device
    self.Out(0x30);
    self.Out(0x00);
    self.Out(0x00);
    in_val = self.Out(0x00);
    if 0x1E != in_val:
      print "Error.sig1(0x%x): Not a compatible device" % in_val;
      return;
    self.Out(0x30);
    self.Out(0x00);
    self.Out(0x01);
    in_val = self.Out(0x00);
    if 0x95 != in_val:
      print "Error.sig2(0x%x): Not a compatible device" % in_val;
      return;
    self.Out(0x30);
    self.Out(0x00);
    self.Out(0x02);
    in_val = self.Out(0x00);
    if 0x02 != in_val:
      print "Error.sig3(0x%x): Not a compatible device" % in_val;
      return;
    else:
      print "Info: Compatible device...";
    ## Fuse programming
    if options.program_fuse:
      print "Info: Programming Fuse : ", ;
      ## Write low byte
      self.Out(0xAC);
      self.Out(0xA0);
      self.Out(0x0);
      self.Out(options.flb);
      ## Just wait for some time
      for j in range(300):
        addr = (j<<7);
        self.Out(0x20);
        self.Out((addr>>9)&0x3F);
        self.Out((addr>>1)&0xFF);
        self.Out(0x0);
      ## Write high byte
      self.Out(0xAC);
      self.Out(0xA8);
      self.Out(0x0);
      self.Out(options.fhb);
      ## Just wait for some time
      for j in range(300):
        addr = (j<<7);
        self.Out(0x20);
        self.Out((addr>>9)&0x3F);
        self.Out((addr>>1)&0xFF);
        self.Out(0x0);
      ## Read back & verify : low byte
      self.Out(0x50);
      self.Out(0x0);
      self.Out(0x0);
      in_val = self.Out(0x0);
      if options.flb != in_val:
        self.error += 1;
        print "Failed (0xFF Vs 0x%x)" % in_val;
      ## Read back & verify : high byte
      self.Out(0x58);
      self.Out(0x08);
      self.Out(0x0);
      in_val = self.Out(0x0);
      if options.fhb != in_val:
        self.error += 1;
        print "Failed (0x89 Vs 0x%x)" % in_val;
      if self.error:
        print "Failed";
      else:
        print "Wrote %x:%x Done" % (options.fhb, options.flb);
      return;
    ## 5. Chip erase
    if options.chip_erase:
      self.Out(0xAC);
      self.Out(0x80);
      self.Out(0x00);
      self.Out(0x00);
      for j in range(10000):
        range(j);
    ## 6. Program sector by sector
    print "Info: Writing & Checking",;
    for i in range(self.NUM_PAGES):
##    if 0:
      if None != self.pages[i]:
        print ".",;
	for j in range(0, self.PAGE_SIZE, 2):
          if (0xFF != self.pages[i][j]) or (0xFF != self.pages[i][j+1]):
            self.Out(0x40);
            self.Out(0x0);
            self.Out((j>>1)&0x3F);
            self.Out(self.pages[i][j]);
            self.Out(0x48);
            self.Out(0x0);
            self.Out((j>>1)&0x3F);
            self.Out(self.pages[i][j+1]);
        ## Initiate write of page
        self.Out(0x4C);
        self.Out((i>>2)&0x3F);
        self.Out((i<<6)&0xC0);
        self.Out(0x0);
        ## Just wait for some time
        for j in range(300):
          addr = (j<<7);
          self.Out(0x20);
          self.Out((addr>>9)&0x3F);
          self.Out((addr>>1)&0xFF);
          self.Out(0x0);
        ## 7. Read each byte and verify write
        error = False;
	for j in range(0, self.PAGE_SIZE, 2):
          addr = (i<<7)|j;
          self.Out(0x20);
          self.Out((addr>>9)&0x3F);
          self.Out((addr>>1)&0xFF);
          bo = self.Out(0x0);
          if bo != self.pages[i][j]:
##            print "Addr:0x%x Obt:0x%x Exp:0x%x" % (addr, bo, self.pages[i][j]);
            error = True;
            self.error += 1;
          self.Out(0x28);
          self.Out((addr>>9)&0x3F);
          self.Out((addr>>1)&0xFF);
          bo = self.Out(0x0);
          if bo != self.pages[i][j+1]:
##            print "Addr:0x%x Obt:0x%x Exp:0x%x" % (addr, bo, self.pages[i][j+1]);
            error = True;
            self.error += 1;
        if error: print "x",;
        else: print ".",;
    print "Complete";
    ## 8. Write lock bits
    if options.write_lock_bits:
      print "Info: Protecting the memory...",;
      self.Out(0xAC); self.Out(0xE0); self.Out(0x00); self.Out(0xCC);
      ## 8b. Read & check lock bits
      self.Out(0x58); self.Out(0x00); self.Out(0x00); in_val = self.Out(0x00);
      if (0x0C != (in_val&0x3F)):
        print " : Failed (Exp:0x0C Obt:0x%x)" % (in_val&0x3F);
        self.error += 1;
      else:
        print " : Done";
    ## 9. Write serial number
    if options.program_serial_no:
      print "Info: Writing serial number...",;
      self.Out(0xC0); self.Out(0x00); self.Out(0x0); self.Out(0xFA);
      print ".",;
      ## Just wait for some time
      for j in range(100):
        self.Out(0xA0);
        self.Out(0x00);
        self.Out(0x0);
        self.Out(0x0);
      self.Out(0xC0); self.Out(0x00); self.Out(0x1); self.Out(0xC7);
      print ".",;
      ## Just wait for some time
      for j in range(100):
        self.Out(0xA0);
        self.Out(0x00);
        self.Out(0x1);
        self.Out(0x0);
      self.Out(0xC0); self.Out(0x00); self.Out(0x2); self.Out(0x05);
      print ".",;
      ## Just wait for some time
      for j in range(100):
        self.Out(0xA0);
        self.Out(0x00);
        self.Out(0x2);
        self.Out(0x0);
      self.Out(0xC0); self.Out(0x00); self.Out(0x3); self.Out(0x1A);
      print ".",;
      ## Just wait for some time
      for j in range(100):
        self.Out(0xA0);
        self.Out(0x00);
        self.Out(0x3);
        self.Out(0x0);
      for i in range(16):
        print ".",;
        self.Out(0xC0); self.Out(0x00); self.Out(i+4); self.Out(ord(options.serial_no[i]));
        ## Just wait for some time
        for j in range(100):
          self.Out(0xA0);
          self.Out(0x00);
          self.Out(i+4);
          self.Out(0x0);
      self.Out(0xA0); self.Out(0x00); self.Out(0x0); in_val = self.Out(0);
      if (0xFA != in_val):
        print "x",;
        self.error += 1;
      self.Out(0xA0); self.Out(0x00); self.Out(0x1); in_val = self.Out(0);
      if (0xC7 != in_val):
        print "x",;
        self.error += 1;
      self.Out(0xA0); self.Out(0x00); self.Out(0x2); in_val = self.Out(0);
      if (0x05 != in_val):
        print "x",;
        self.error += 1;
      self.Out(0xA0); self.Out(0x00); self.Out(0x3); in_val = self.Out(0);
      if (0x1A != in_val):
        print "x",;
        self.error += 1;
      for i in range(16):
        print ".",;
        self.Out(0xA0); self.Out(0x00); self.Out(i+4); in_val = self.Out(0);
        if (ord(options.serial_no[i]) != in_val):
          print " : Serial # Failed (idx:%d Exp:0x%x Obt:0x%x)" % (i, ord(options.serial_no[i]), in_val&0xFF);
          self.error += 1;
          break;
      if (0 == self.error): print " : Done"

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
  parser.add_option("-c", "--test_cable",
                  action="store_true", dest="test_link", default=False,
                  help="Test the cable for connection with compatible target");
  parser.add_option("-f", "--file", dest="ihex", default="main.hex",
                  help="Default program file");
  parser.add_option("-p", "--lpt_addr", dest="lpt_addr", default=0x378,
                  help="Parallel port address on the device (def:0x378)");
  parser.add_option("", "--flb", dest="flb", default=0xE4, type="int",
                  help="Fuse low byte value");
  parser.add_option("", "--fhb", dest="fhb", default=(0x99|0x6)&~0x8, type="int",
                  help="Fuse high byte value");
  parser.add_option("", "--feb", dest="feb", default=0xFC, type="int",
                  help="Fuse extended byte value");
  parser.add_option("-r", "--release_bus", dest="release_bus", default=False,
                  action="store_true", help="release the drive");
  parser.add_option("-D", "--defaults", dest="defaults", default=False,
                  action="store_true", help="Use reset values wherever applicable");
  parser.add_option("-F", "--program_fuse", dest="program_fuse", default=False,
                  action="store_true", help="release the drive");
  parser.add_option("", "--program_serial_no", dest="program_serial_no", default=False,
                  action="store_true", help=SUPPRESS_HELP);
  parser.add_option("", "--chip_erase", dest="chip_erase", default=False,
                  action="store_true", help=SUPPRESS_HELP);
  parser.add_option("", "--cpph", dest="cpph", default=False,
                  action="store_true", help=SUPPRESS_HELP);
  parser.add_option("", "--write_lock_bits", dest="write_lock_bits", default=False,
                  action="store_true", help=SUPPRESS_HELP);
  parser.add_option("", "--serial_no", dest="serial_no", default="abcdef1234",
                  help=SUPPRESS_HELP);
  parser.add_option("", "--mcu", dest="mcu", default="atmega32",
                  help=SUPPRESS_HELP); ## atmega1284p
  parser.add_option("", "--device_id", dest="device_id", default="01",
                  help=SUPPRESS_HELP);
  parser.add_option("-o", "--output", dest="output", default="lptprog.out",
                  help=SUPPRESS_HELP); ## atmega1284p
  (options, args) = parser.parse_args()

  ## modify certain default values
  if 'atmega32' == options.mcu:
    dev = Atmega32(options.lpt_addr);
  else:
    assert(0);

  ##
  if options.release_bus:
    print "Causing reset & releasing the bus";
    dev.Reset();
    dev.Release();
    raw_input("Press Enter to continue...");
    exit(dev.error);

  ##
  dev.pages = [None]*dev.NUM_PAGES;
  dev.error = 0;

  ## 
  if options.program_serial_no:
    assert(10 == len(options.serial_no));
    options.serial_no += options.device_id
    assert(12 == len(options.serial_no));
    ui16 = crc16(bytearray(options.serial_no));
    options.serial_no += chr((ui16>>8)&0xFF)
    options.serial_no += chr(ui16&0xFF)
    assert(14 == len(options.serial_no));
    options.chip_erase = True;

  ##
  f = open(options.ihex);
  if not f:
    print "Error: Failed opening file";
    exit (1);

  ##
  if options.defaults:
    print "Using default fuse values";
    if 'atmega32' == options.mcu:
      options.flb = 0xE4;
      options.fhb = 0x99;
    else:
      assert(0);

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
    if options.cpph:
      dev.GenCpp(options);
    else:
      dev.ProgEn(options);
      dev.Release();
      raw_input("Errors:%d Press Enter to continue..." % dev.error);
    if dev.error: print "Programming Finished with %d errors" % dev.error;
  else:
    print "No valid programming found in hex file";

  exit (dev.error);
