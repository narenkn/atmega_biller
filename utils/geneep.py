#! python

from optparse import OptionParser
from ctypes import *
from optparse import SUPPRESS_HELP
import time
import re
from sys import exit

## Constants
REC_DATA = 0;
REC_EOF = 1;
REC_EX_SEGADDR = 2;
REC_ST_SEGADDR = 3;
REC_EX_LIADDR  = 4;
REC_ST_LIADDR  = 5;

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

class hexLine:
  def __init__(self, l=0, a=0, rt=REC_DATA):
    self.len = l;
    self.addr = a;
    self.data = '';
    self.recType = rt;

  ##   (nBytes)(  addr)(record)(dat)(crc )
  ## r'^:(\w\w)(\w\w\w\w)(\w\w)(\w*)(\w\w)$'
  def __str__(self):
    str = "%02x%04x%02x" % (self.len, self.addr, self.recType);
    if (len(self.data)):
      str += self.data;
    c16 = crc16(str);
    str += "%02x" % c16;
    assert(len(str) == (10+len(self.data)));
    return str;

class hexFile:
  def __init__(self):
    self.lines = {};

  def __str__(self):
    s = '';
    for a in sorted(self.lines):
      s += str(self.lines[a]);
      s += "\n";
    s += ":00000001FF\n";
    return s;

if "__main__" == __name__:
  ##
  parser = OptionParser()
  parser.add_option("-f", "--file", dest="ihex", default="main.hex",
                  help="EEPROM file to create");
  parser.add_option("-D", "--defaults", dest="defaults", default=False,
                  action="store_true", help="Use reset values wherever applicable");
  parser.add_option("", "--serial_no", dest="serial_no", default="abcdef1234",
                  help=SUPPRESS_HELP);
  parser.add_option("", "--mcu", dest="mcu", default="atmega32",
                  help=SUPPRESS_HELP); ## atmega1284p
  parser.add_option("", "--device_id", dest="device_id", default="01",
                  help=SUPPRESS_HELP);
  parser.add_option("-o", "--output", dest="output", default="lptprog.out",
                  help=SUPPRESS_HELP); ## atmega1284p
  (options, args) = parser.parse_args()

  ## Init
  error = 0;

  ## modify certain default values

  ## 
  assert(10 == len(options.serial_no));
  options.serial_no += options.device_id
  assert(12 == len(options.serial_no));
  ui16 = crc16(bytearray(options.serial_no));
  options.serial_no += chr((ui16>>8)&0xFF)
  options.serial_no += chr(ui16&0xFF)
  assert(14 == len(options.serial_no));

  ##
  f = open(options.ihex, "w");
  if not f:
    print "Error: Failed opening file";
    exit (1);

  ## Create the hex File
  hfile = hexFile();
  hfile[0] = options.serial_no;
  hfile[0] = options.serial_no;
  f.write(str(hfile));

  exit (error);
