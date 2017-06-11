def checksum(l):
  ## Compute checksum
  checksum = 0;
  for i in range(0, len(l)-1, 2):
    checksum += int(l[i:(i+2)], 16);
  checksum &= 0xFF
  checksum = (checksum ^ 0xFF) + 1;
  return checksum;

##
def crc16(string, crc=0):
  for a in string:
    crc ^= ord(a);
    for i in range(8):
      if crc & 1:
        crc = (crc >> 1) ^ 0xA001;
      else:
        crc = (crc >> 1);
  return crc

def toHsig(str):
  hstr = '';
  for a in str:
    hstr += "%02x" % ord(a);
  return hstr;

##   (nBytes)(  addr)(record)(dat)(crc )
## r'^:(\w\w)(\w\w\w\w)(\w\w)(\w*)(\w\w)$'
sig = "ABCDEF789001";
hsig = toHsig(sig);
l1 = "10" + "0000" + "00" + "FAC7051A" + hsig;
print ":%s%02x" % (l1, checksum(l1));
l2 = "02" + "0010" + "00" + ("%04x" % crc16(sig));
print ":%s%02x" % (l2, checksum(l2));
l3 = "00" + "0000" + "01";
print ":%s%02x" % (l3, checksum(l3));
