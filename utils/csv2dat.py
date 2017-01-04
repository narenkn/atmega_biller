from optparse import OptionParser
from optparse import SUPPRESS_HELP
#from ctypes import windll
import time
import re
import sys
import csv
import os

TYPE_NONE    = 0
TYPE_UINT8   = 1
TYPE_UINT16  = 2
TYPE_UINT32  = 4
TYPE_STRING  = 5
TYPE_ARR     = 7
TYPE_BIT     = 8
TYPE_STRUCT  = 9

class data_struct:
  error = 0
  def ds_print(self, cl):
    for const, cval in cl.consts.items():
      print "#define %25s %5d" % (const, cval)
    print ""
    print "struct %s {" % cl.name
    for t_var, tvars in sorted(cl.variables.items()):
      for var, s_var in sorted(tvars.items(), key=lambda x: x[1], reverse=True):
        print "  %15s %25s%s;" % (t_var, var.replace(' ','').replace('%',''), s_var[0])
    print "} __attribute__((packed));"
    print "#define %s_SIZEOF sizeof(struct %s)" % (cl.name.upper(), cl.name)

  def get_type(self, type_val):
    if TYPE_NONE == type_val:
      return "TYPE_NONE"
    elif TYPE_UINT8 == type_val:
      return "TYPE_UINT8"
    elif TYPE_UINT16 == type_val:
      return "TYPE_UINT16"
    elif TYPE_UINT32 == type_val:
      return "TYPE_UINT32"
    elif TYPE_STRING == type_val:
      return "TYPE_STRING"
    elif TYPE_BIT == type_val:
      return "TYPE_BIT"
    elif TYPE_ARR == type_val:
      return "TYPE_ARR"
    assert 0, "Can't match types.."

  def menu_var_table(self, cl):
    re_1 = re.compile(r'unused')
    print '#define TYPE_NONE    ', TYPE_NONE
    print '#define TYPE_UINT8   ', TYPE_UINT8
    print '#define TYPE_UINT16  ', TYPE_UINT16
    print '#define TYPE_UINT32  ', TYPE_UINT32
    print '#define TYPE_STRING  ', TYPE_STRING
    print '#define TYPE_BIT     ', TYPE_BIT
    print '#define TYPE_ARR  ', TYPE_ARR
    print '#define SETTING_VAR_TABLE \\'
    goffset = 0
    num_vars = 0
    for t_var, tvars in sorted(cl.variables.items()):
      for var, s_var in sorted(tvars.items(), key=lambda x: x[1], reverse=True):
        if re_1.match(var) or ('passwds' == var) or ('users' == var):
          if int(((goffset+s_var[1]) * 10) % 10) > 8:
            goffset += 1
            goffset = int(goffset)
          goffset += s_var[1]
          continue
        if TYPE_ARR == s_var[2]:
          for idx in range(0, s_var[1], s_var[3]):
            num_vars += 1;
            print "\t{%s, %d, %d, \"%s[%d]\", offsetof(struct ep_store_layout, %s)+(%d*%d)}, \\" % (self.get_type(s_var[3]), s_var[3], int((s_var[1]*10)%10), var, idx/s_var[3], var.replace(' ','').replace('%',''), idx/s_var[3], s_var[3])
        elif TYPE_BIT == s_var[2]:
          num_vars += 1;
          print "\t{%s, %d, %d, \"%s\", %d}, \\" % (self.get_type(s_var[2]), int((s_var[1]*10)%10), int((goffset*10)%10), var, int(goffset))
        else:
          num_vars += 1;
          print "\t{%s, %d, %d, \"%s\", offsetof(struct ep_store_layout, %s)}, \\" % (self.get_type(s_var[2]), int(s_var[1]), int((s_var[1]*10)%10), var, var.replace(' ','').replace('%',''))
        assert (s_var[1]<256), "size holder overflow"
        if int(((goffset+s_var[1]) * 10) % 10) > 8:
          goffset += 1
          goffset = int(goffset)
        goffset += s_var[1]
    print ''
    print '#define SETTING_VARS_SIZE ', num_vars

class csv2dat:
  error = 0

  def __init__(self, o):
    self.options = o

  def crc16(self, string, crc=0):
    for a in string:
      crc ^= a
      for i in range(8):
        if crc & 1:
          crc = (crc >> 1) ^ 0xA001
        else:
          crc = (crc >> 1)
    return crc

  def sizeof(self, cl):
    goffset = 0
    for t_var, tvars in sorted(cl.variables.items()):
      for var, s_var in sorted(tvars.items(), key=lambda x: x[1][0], reverse=True):
        if int(((goffset+s_var[1]) * 10) % 10) > 8:
          goffset += 1
          goffset = int(goffset)
        goffset += s_var[1]
    return goffset

  def make_dat(self, cl):
    csvfile = open(self.options.in_file, 'rb')
    re_1 = re.compile(r'unused')
    csv_lines = 0
    data = {}
    if csvfile:
      reader = csv.reader(csvfile, delimiter=',', quotechar="'")
      re_int = re.compile(r'\d+')
      re_comment_line = re.compile(r'^#')
      headers = reader.next()
      for h in headers:
        data[h] = []
      for row in reader:
        if not row: continue
        if re_comment_line.match(row[0]): continue
        csv_lines += 1

#	print "row is '%s'" % row
        for h, v in zip(headers, row):
          data[h].append(v.strip())
      ##
      vardict = {}
      vardict_keys = []
      goffset = 0
      for t_var, tvars in sorted(cl.variables.items()):
        for var, s_var in sorted(tvars.items(), key=lambda x: x[1][0], reverse=True):
          if re_1.match(var):
            pass
          elif var not in data:
            print "column '%s' not found in csv(%s)" % (var, csvfile)
            self.error += 1
            return
          if 0 == s_var[1]: s_var[1] = offsetof(cl);
          if int(((goffset+s_var[1]) * 10) % 10) > 8:
            goffset += 1
            goffset = int(goffset)
          s_var.append(goffset)
          assert var not in vardict
          vardict[var] = s_var
          vardict_keys.append(var)
          goffset += s_var[1]
##          print "var:%s goffset:%f" % (var, goffset)
      ## Sanity check is complete, things should work from here
      outfile = open(self.options.out_file, 'wb')
      signature = 0
      assert(outfile)
      ## Write CRC string
      tt = os.popen('git log --pretty=format:"%h"').readline()
      for i in range(7):
        outfile.write(tt[i])
        signature = self.crc16(bytearray(tt[i]), signature)
#      pbytes = []
#      for i in range(1, -1, -1):
#        pbytes.append((csv_lines >> (i*8)) & 0xFF);
#      outfile.write(bytearray(pbytes))
#      signature = self.crc16(bytearray(pbytes), signature)
      re_graph = re.compile(r'[\x20-\x7E]')
      re_hexdigit = re.compile(r'[\x30-\x39\x41-\x46\x61-\x66]')
      goffset += 0.9
      for li in range(csv_lines):
        pbytes = [0] * int(goffset)
        for var in vardict_keys:
          offset = int(vardict[var][-1])
          if options.debug:
            print "%s (TYPE:%d), offset : %f" % (var, vardict[var][2], offset),
            if var in data: print data[var]
            else: print ""
          if var not in data:
            pass
          elif (TYPE_UINT16 == vardict[var][2]) or (TYPE_UINT32 == vardict[var][2]):
            for i in range(vardict[var][1]): ## each bytes
              if '' != data[var][li]:
                pbytes[offset+i] = ((int(data[var][li]) >> (i*8)) & 0xFF)
          elif (TYPE_ARR == vardict[var][2]) and (TYPE_UINT8 == vardict[var][3]):
            i,j,k,l = 0,0,0,0
            while (i < vardict[var][1]):
              if (j < len(data[var][li])): ## each bytes
                c = data[var][li][j]
                if re_hexdigit.match(c):
                  k <<= 4
                  k |= int(c, 16)
              else:
                k = 0x20 ## space
              if (1 == (l&1)): ## assign
                pbytes[offset+i] = k & 0xFF
                i += 1
#                print "0x%x" % (k&0xFF)
              l += 1
              j += 1
          elif (TYPE_ARR == vardict[var][2]) and ((TYPE_UINT16 == vardict[var][3]) or (TYPE_UINT32 == vardict[var][3])):
            ## Expect a ';' seperated list of values
            i=0
            for s_data in data[var][li].split(';'):
              k = int(s_data)
              for j in reversed(range(vardict[var][3])):
                pbytes[(vardict[var][4]+(i*vardict[var][3]))+j] = (k>>(j*vardict[var][3]*8)) & 0xFF
##                print pbytes[vardict[var][4]+j]
              i+=1
          elif (TYPE_ARR == vardict[var][2]):
            assert(0) ## Not yet implemented
          elif TYPE_STRING == vardict[var][2]:
            for i in range(vardict[var][1]): ## each bytes
              if (i >= len(data[var][li])) or (not re_graph.match(data[var][li][i])):
                pbytes[offset+i] = ' '
              else:
                pbytes[offset+i] = data[var][li][i].upper()
          elif TYPE_BIT == vardict[var][2]:
            if '' != data[var][li]:
              pbytes[offset] |= int(data[var][li]) << (int((vardict[var][-1]*10)%10))
          elif TYPE_UINT8 == vardict[var][2]:
            if '' != data[var][li]:
              if (0 != (offset*10)%10): offset += 1
              pbytes[offset] = int(data[var][li])
          else: assert(0), "Field '%s' is not type matched ;%s;" % (var, data[var])
        if options.debug: print "pbytes:", pbytes
        outfile.write(bytearray(pbytes))
        signature = self.crc16(bytearray(pbytes), signature)
      pbytes = []
      for i in range(1, -1, -1):
        pbytes.append((signature >> (i*8)) & 0xFF);
      outfile.write(bytearray(pbytes))
      outfile.close()
      csvfile.close()

  def make_csv(self, cl):
    datfile = open("%sdat" % self.file[:-3], 'rb')
    csvfile = open(self.file, 'wb')
    if datfile and csvfile:
      writer = csv.writer(csvfile, delimiter=',', quotechar="'")
      assert(0)
      datfile.close()
      csvfile.close()

class item:
  consts = {
    'ITEM_NAME_BYTEL':23,
    'ITEM_PROD_CODE_BYTEL':22,
    }
  variables = {
    'uint32_t' : {
      'cost':['', 4, TYPE_UINT32],
      'discount':['', 4, TYPE_UINT32],
      },
    'uint16_t' : {
      'id': ['', 2, TYPE_UINT16],
      'Vat' : ['', 2, TYPE_UINT16],
      'Tax1' : ['', 2, TYPE_UINT16],
      'Tax2' : ['', 2, TYPE_UINT16],
      'Tax3' : ['', 2, TYPE_UINT16],
      },
    'uint8_t'  : {
      'name':['[ITEM_NAME_BYTEL]', 1*15, TYPE_STRING],
      'prod_code':['[ITEM_PROD_CODE_BYTEL]', 1*16, TYPE_STRING],
##      'name_unicode':['[ITEM_NAME_UNI_BYTEL]', 1*16, TYPE_ARR, TYPE_UINT8],
      'unused_crc':['', 1, TYPE_UINT8],
      'unused_crc_invert':['', 1, TYPE_UINT8],
      'has_vat':[':1', 0.1, TYPE_BIT],
      'has_tax1':[':1', 0.1, TYPE_BIT],
      'has_tax2':[':1', 0.1, TYPE_BIT],
      'has_tax3':[':1', 0.1, TYPE_BIT],
      'has_weighing_mc':[':1', 0.1, TYPE_BIT],
      'is_disabled':[':1', 0.1, TYPE_BIT],
      'has_common_discount':[':1', 0.1, TYPE_BIT],
      'is_reverse_tax':[':1', 0.1, TYPE_BIT],
      }
    }

  def __init__(self, name):
    self.name = name

  def change_const(self, id, val):
    self.consts[id] = val;

class ep_store_layout:
  consts = {
    'HEADER_SZ_MAX' : 32*4,
    'SHOP_NAME_SZ_MAX' : 16*2,
    'COMPANY_NAME_MAX' : 16,
    'FOOTER_SZ_MAX' : 32*2,
    'SERIAL_NO_MAX' : 14, ## 12 + 2CRC
    'SCRATCH_MAX' : 16,
    'EPS_CAPTION_SZ_MAX' : 10,
    'EPS_MAX_USERS' : 4,
    'EPS_MAX_UNAME' : 8,
    'EPS_WORD_LEN' : 8,
    'ITEM_MAX' : 2048,
    'NUM_HOT_KEYS' : 16*3,
    }
  variables = {
    'uint16_t' : {
      'unused_passwds' : ['[EPS_MAX_USERS+1]', 2*16, TYPE_UINT16],
      'Common Disc%' : ['', 2, TYPE_UINT16],
      'unused_DiagStat' : ['', 2, TYPE_UINT16],
      'unused_LastBillId' : ['', 2, TYPE_UINT16],
      'unused_ItemLastUsed' : ['', 2, TYPE_UINT16],
      'Round Off' : ['', 2, TYPE_UINT16],
      'unused_nextBillAddr' : ['', 2, TYPE_UINT16],
      'unused_todayStartAddr' : ['', 2, TYPE_UINT16],
      'unused_HotKey' : ['[NUM_HOT_KEYS]', 16*3, TYPE_UINT16],
      },
    'uint8_t' : {
      ## User options
      ## User 0 : is 'admin' + 15 usernames
      ## 5 passwords
      'unused_users' : ['[EPS_MAX_USERS+1][EPS_MAX_UNAME]', 5*8, TYPE_STRING],
      'unused_itIdxName' : ['[ITEM_MAX]', 1260, TYPE_UINT8],
      'unused_crc_prod_code' : ['[ITEM_MAX]', 1260, TYPE_UINT8],
      'unused_serial_no' : ['[SERIAL_NO_MAX]', 14, TYPE_STRING],
      'unused_scratch' : ['[SCRATCH_MAX]', 16, TYPE_STRING],
      'Currency' : ['[EPS_WORD_LEN]', 8, TYPE_STRING],
      'Bill Prefix' : ['[EPS_WORD_LEN]', 8, TYPE_STRING],
      'Caption' : ['[EPS_CAPTION_SZ_MAX]', 10, TYPE_STRING],
      'Shop Name' : ['[SHOP_NAME_SZ_MAX]', 16*2, TYPE_STRING],
      'unused_comp_name' : ['[COMPANY_NAME_MAX]', 16, TYPE_STRING],
      'Bill Header' : ['[HEADER_SZ_MAX]', 32*4, TYPE_STRING],
      'Bill Footer' : ['[FOOTER_SZ_MAX]', 32*2, TYPE_STRING],
      'Key Beep On':['', 1, TYPE_UINT8],
    }
  }
  def __init__(self, name):
    self.name = name

class dat2bill:
  MAX_ITEMS_IN_BILL = 16
  SALE_INFO_ITEMS_NBITS = 4
  sale_item = [
    ['ep_item_ptr', TYPE_UINT16, 2],
    ['quantity', TYPE_UINT16, 2],
    ['cost', TYPE_UINT32, 4],
    ['discount', TYPE_UINT32, 4],
    ['unused', TYPE_BIT, 5],
    ['has_serv_tax', TYPE_BIT, 1],
    ['vat_sel', TYPE_BIT, 2],
  ]
  sale_info = [
    ['n_items', TYPE_BIT, SALE_INFO_ITEMS_NBITS],
    ['property', TYPE_BIT, 8-SALE_INFO_ITEMS_NBITS],
    ['date_yy', TYPE_BIT, 7],
    ['date_mm', TYPE_BIT, 4],
    ['date_dd', TYPE_BIT, 5],
    ['time_hh', TYPE_BIT, 5],
    ['time_mm', TYPE_BIT, 6],
    ['time_ss', TYPE_BIT, 5],
  ]
  sale_end = [
    ['taxable_total', TYPE_UINT32, 4],
    ['non_taxable_total', TYPE_UINT32, 4],
    ['discount', TYPE_UINT32, 4],
    ['total', TYPE_UINT32, 4],
  ]

  def __init__(self, name):
    self.name = name
    self.sale = sale_info + (sale_item * MAX_ITEMS_IN_BILL) + sale_end

if "__main__" == __name__:
  ## Constants
  error = 0
  options = None

  ##
  parser = OptionParser()
  parser.add_option("", "--header",
                  action="store_true", dest="header", default=False,
                  help=SUPPRESS_HELP);
  parser.add_option("", "--debug",
                  action="store_true", dest="debug", default=False,
                  help=SUPPRESS_HELP);
  parser.add_option("", "--items",
                  action="store_true", dest="items", default=False,
                  help="process items");
  parser.add_option("", "--settings",
                  action="store_true", dest="settings", default=False,
                  help="process settings");
  parser.add_option("", "--crc16",
                  action="store_true", dest="crc16", default=False,
                  help=SUPPRESS_HELP);
  parser.add_option("", "--to_dat",
                  action="store_true", dest="to_dat", default=False,
                  help="Create csv file from dat");
  parser.add_option("", "--to_csv",
                  action="store_true", dest="to_csv", default=False,
                  help="Create dat file from csv");
  parser.add_option("-i", "--in_file", dest="in_file", default="settings.csv",
                  help="Input file");
  parser.add_option("", "--dat2bill", dest="dat2bill", default=None,
                  help="Convert billed data to xls");
  parser.add_option("", "--from_date", dest="from_date", default=None,
                  help="Starting date to analyze from");
  parser.add_option("", "--to_date", dest="to_date", default=None,
                  help="Ending date to analyze upto");
  parser.add_option("-o", "--out_file", dest="out_file", default=None,
                  help="Output file");
  parser.add_option("", "--device_id", dest="device_id", default=1024>>10,
                  help=SUPPRESS_HELP);
  (options, args) = parser.parse_args()

  it = item('item')
  ep = ep_store_layout('ep_store_layout')

  if options.crc16:
    cd = csv2dat(None)
    print "#define GIT_HASH_CRC 0x%04x" % cd.crc16(bytearray(args[0]))
  elif options.header:
    print "#ifndef EP_DS_H\n#define EP_DS_H\n\n"
    ds = data_struct()
    ds.ds_print(it)
    ds.ds_print(ep)
    ds.menu_var_table(ep)
    print "\n\n#endif"
    error += ds.error
  elif options.to_dat:
    cd = csv2dat(options)
    if not options.out_file:
      (options.out_file, ext) = os.path.splitext(options.in_file)
      options.out_file += '.dat'
    if options.items: cd.make_dat(it)
    elif options.settings: cd.make_dat(ep)
    else: assert(0), "either --items or --settings needs to be provided"
    error += cd.error
  elif options.to_csv:
    cd = csv2dat(options)
    cd.make_csv(it)
    error += cd.error
  elif options.dat2bill:
    pass
  else: error += 1

  sys.exit (error)
