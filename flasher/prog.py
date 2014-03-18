from optparse import OptionParser
from ctypes import windll
import time
import re

class AT89S52:

  p = windll.inpout32;
  def_val = 0x0;
  ## 8K with each page 256 bytes
  PAGE_SIZE = 256;
  NUM_PAGES = 8*4;

  def Out1(self, byte):
    in_val = 0;
    for i in range(8):
      in_val <<= 1;
      self.p.Out32(self.dat_reg, self.def_val|((byte>>(7-i)<<5)&0x20));
      self.p.Out32(self.dat_reg, self.def_val|0x10);
      in_val |= (self.p.Inp32(self.sta_reg)>>6) & 0x1;
      self.p.Out32(self.dat_reg, self.def_val);
    return (in_val & 0xFF);

  def Out(self, byte):
    print "Programming byte: 0x%x" % byte;
    return 0;

  def Reset(self):
    for i in range(8):
      self.Out(0x0);
    self.Out(0x10);
    self.def_val = 0x10;

  def __init__(self, dr):
    self.dat_reg = dr;
    self.sta_reg = dr+1;
    self.Out(0);

  def ProgEn(self):
    ## 1. Reset
    self.Reset();
    ## 2. Get acknowledgement of programming
    self.Out(0xAC);
    self.Out(0x53);
    self.Out(0x00);
    in_val = self.Out(0x00);
##    print "0x%x" % in_val;
    ## 3. Check for ack before proceeding
    if (0x69 != in_val):
      print "Error: Couldn't communicate with device, check cable";
      return;
    else:
      print "Info: Detected device connected...";
    ## 4. Read signature byte for 89S52
    self.Out(0x28);
    self.Out(0x01);
    self.Out(0x00);
    in_val = self.Out(0x00);
    if 0x52 != in_val:
      print "Error: Not a compatible device";
      return;
    else:
      print "Info: Compatible device...";
    ## 5. Chip erase
    self.Out(0xAC);
    self.Out(0x80);
    self.Out(0x00);
    self.Out(0x00);
    ## 6. Program sector by sector
    print "Info: Writing memory",;
    for i in range(self.NUM_PAGES):
      if None != self.pages[i]:
        print ".",;
        self.Out(0x50);
        self.Out(i);
        for b in self.pages[i]:
          self.Out(b);
    print "";
    ## 7. Read each sectors and verify write
    print "Info: Reading back and verifying memory";
    for i in range(self.NUM_PAGES):
      if None != self.pages[i]:
        self.Out(0x50);
        self.Out(i);
        error = False;
        for b in self.pages[i]:
          bo = self.Out(0x0);
          if bo != b: error = True;
        if error: print "x",;
        else: print ".",;
    print "";
    ## 8. Write lock bits
    print "Info: Protecting the memory...";
    self.Out(0xAC); self.Out(0xE0); self.Out(0x00); self.Out(0x00);
    self.Out(0xAC); self.Out(0xE1); self.Out(0x00); self.Out(0x00);
    self.Out(0xAC); self.Out(0xE2); self.Out(0x00); self.Out(0x00);
    self.Out(0xAC); self.Out(0xE3); self.Out(0x00); self.Out(0x00);
    ## 8b. Read & check lock bits
    self.Out(0x24); self.Out(0x00); self.Out(0x00); in_val = self.Out(0x00);
    if (0x1C != (in_val&0x1C)):
      print "Failure to protect the memory...";
      return;
    else:
      print "Info: Protected memory...";

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
  parser.add_option("-f", "--file", dest="ihex", default="default.ihx",
                  help="Default program file");
  parser.add_option("-p", "--lpt_addr", dest="lpt_addr", default=0x3BC,
                  help="Parallel port address on the device (def:0x3BC)");
  (options, args) = parser.parse_args()

  dev = AT89S52(options.lpt_addr);
  dev.pages = ['']*dev.NUM_PAGES;

  ##
  for ui in range(dev.NUM_PAGES):
    dev.pages[ui] = None;

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
      page = address >> 8; ## 256 bytes per page
      address &= 0xFF;
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
        if not dev.pages[page]:
          any_page_valid = True;
          dev.pages[page] = [0xFF] * dev.PAGE_SIZE;
        for i in range(9, len(l)-3, 2):
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
  dev.ProgEn();

