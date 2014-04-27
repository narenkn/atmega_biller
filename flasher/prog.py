from optparse import OptionParser
from ctypes import windll
import time
import re

## Pin assignments
##   LPT has pins 1-25
##   D[7:0], S[7:0]
## LPT5, D3 - Serial program enable
## LPT6, D4 - SCLK
## LPT7, D5 - MOSI
## LPT9, D7 - RESET
## LPT10, S6 - MISO

class Atmega32:

  p = windll.inpout32;
  def_val = 0x0;
  ## 32K with each page 64 words
  PAGE_SIZE = 128;
  NUM_PAGES = 32*8; ## 32K bytes (32*1024/128)

  def Out(self, byte):
    in_val = 0;
    for i in range(8):
      in_val <<= 1;
      self.p.Out32(self.dat_reg, self.def_val|((byte>>(7-i)<<5)&0x20));
      self.p.Out32(self.dat_reg, self.def_val|((byte>>(7-i)<<5)&0x20)|0x10);
      in_val |= (self.p.Inp32(self.sta_reg)>>6) & 0x1;
      self.p.Out32(self.dat_reg, self.def_val);
    return (in_val & 0xFF);

  def RawOut(self, byte):
    self.p.Out32(self.dat_reg, self.def_val|byte);
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
    self.def_val = 0x8;
    self.RawOut(0x8);
    self.p.Out32(self.cnt_reg, 0x0);

  def __init__(self, dr):
    self.error = 0;
    self.dat_reg = dr;
    self.sta_reg = dr+1;
    self.cnt_reg = dr+2;
    self.def_val = 0x8;
    self.RawOut(0);

  def TestPins(self):
    self.Reset();
    self.RawOut(0x0);

  def ProgEn(self):
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
      print "Error.sig1: Not a compatible device";
      return;
    self.Out(0x30);
    self.Out(0x00);
    self.Out(0x01);
    in_val = self.Out(0x00);
    if 0x95 != in_val:
      print "Error.sig2: Not a compatible device";
      return;
    self.Out(0x30);
    self.Out(0x00);
    self.Out(0x02);
    in_val = self.Out(0x00);
    if 0x02 != in_val:
      print "Error.sig3: Not a compatible device";
      return;
    else:
      print "Info: Compatible device...";
    ## 5. Chip erase
    self.Out(0xAC);
    self.Out(0x80);
    self.Out(0x00);
    self.Out(0x00);
    for j in range(10000):
      range(j);
    ## 6. Program sector by sector
    print "Info: Writing & Checking",;
    for i in range(self.NUM_PAGES):
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
            print "Addr:0x%x Obt:0x%x Exp:0x%x" % (addr, bo, self.pages[i][j]);
            error = True;
            self.error += 1;
          self.Out(0x28);
          self.Out((addr>>9)&0x3F);
          self.Out((addr>>1)&0xFF);
          bo = self.Out(0x0);
          if bo != self.pages[i][j+1]:
            print "Addr:0x%x Obt:0x%x Exp:0x%x" % (addr, bo, self.pages[i][j+1]);
            error = True;
            self.error += 1;
        if error: print "x",;
        else: print ".",;
    print "Complete";
    ## 8. Write lock bits
##    print "Info: Protecting the memory...",;
##    self.Out(0xAC); self.Out(0xE0); self.Out(0x00); self.Out(0xCC);
##    ## 8b. Read & check lock bits
##    self.Out(0x58); self.Out(0x00); self.Out(0x00); in_val = self.Out(0x00);
##    if (0x0C != (in_val&0x3F)):
##      print " : Failed (Exp:0x0C Obt:0x%x)" % (in_val&0x3F);
##      self.error += 1;
##    else:
##      print " : Done";

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
  parser.add_option("-p", "--lpt_addr", dest="lpt_addr", default=0x3BC,
                  help="Parallel port address on the device (def:0x3BC)");
  (options, args) = parser.parse_args()

  dev = Atmega32(options.lpt_addr);
  dev.pages = [None]*dev.NUM_PAGES;

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
      elif REC_EX_LIADDR == record_type:
        segment_addr = int(m.group(4), 16) << 16;
      elif REC_EOF == record_type:
        break;
      else:
        print "Error : Unknown record type: ", record_type;
        exit (3);

  ## Only program if valid lines are available in file
  if any_page_valid:
    dev.error = 0;
    dev.ProgEn();
    dev.Release();
    if dev.error: print "Programming Finished with %d errors" % dev.error;
  else:
    print "No valid programming found in hex file";

  exit (dev.error);
