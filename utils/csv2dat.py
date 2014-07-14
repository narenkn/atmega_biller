from optparse import OptionParser
from optparse import SUPPRESS_HELP
#from ctypes import windll
import time
import re
import sys

class data_struct:
  error = 0
  def ds_print(self, cl):
    for const, cval in cl.consts.items():
      print "#define %25s %5d" % (const, cval)
    print ""
    print "struct %s {" % cl.name
    for t_var, tvars in cl.variables.items():
      for var, s_var in tvars.items():
        print "  %10s %25s%s;" % (t_var, var, s_var)
    print "} __attribute__((packed));"
    print "#define %s_SIZEOF sizeof(struct %s)" % (cl.name.upper(), cl.name)

class item:
  consts = {
    'ITEM_NAME_BYTEL':12,
    'ITEM_NAME_UNI_BYTEL':24,
    'ITEM_PROD_CODE_BYTEL':20,
    }
  variables = {
    'uint16_t' : {'id':'', 'cost':'', 'discout':''},
    'uint8_t'  : {
      'name':'[ITEM_NAME_BYTEL]',
      'prod_code':'[ITEM_PROD_CODE_BYTEL]',
      'name_unicode':'[ITEM_NAME_UNI_BYTEL]',
      'has_serv_tax':':1',
      'has_common_discount':':1',
      'has_cess1':':1',
      'has_cess2':':1',
      'has_weighing_mc':':1',
      'vat_sel':':3',
      'name_in_unicode':':1',
      'is_biller_item':':1',
      }
    }

  def __init__(self, name):
    self.name = name

class ep_store_layout:
  consts = {
    'HEADER_MAX_SZ_MAX' : 54,
    'SHOP_NAME_SZ_MAX' : 16,
    'FOOTER_MAX_SZ_MAX' : 24,
    'EPS_MAX_VAT_CHOICE' : 8,
    'EPS_MAX_USERS' : 15,
    'EPS_MAX_UNAME' : 8,
    }
  variables = {
    'uint16_t' : {
      'vat' : '[EPS_MAX_VAT_CHOICE]',
      'service_tax' : '',
      'cess1' : '',
      'cess2' : '',
      'round_off' : '',
      ## bits [15:4] : Unused
      ## bits [3:0] : Printing
      ## value 0b0000 : don't ask, 0b00xx : copies without confirmation,
      ## 0b01xx : confirm before print xx copies
      'property' : '',
      'passwds' : '[EPS_MAX_UNAME+1]',
      'shop_name_len' : '',
      'prn_header_len' : '',
      'prn_footer_len' : '',
      },
    'uint8_t' : {
      ## User options
      ## User 0 : is 'admin' + 15 usernames
      ## 16 passwords
      'users' : '[EPS_MAX_USERS][EPS_MAX_UNAME]',
      'denomination_text' : '[20]',
      'shop_name' : '[SHOP_NAME_SZ_MAX]',
      'prn_header' : '[HEADER_MAX_SZ_MAX]',
      'prn_footer' : '[FOOTER_MAX_SZ_MAX]',
      }
    }
  def __init__(self, name):
    self.name = name

if "__main__" == __name__:
  ## Constants
  error = 0

  ##
  parser = OptionParser()
  parser.add_option("", "--header",
                  action="store_true", dest="header", default=False,
                  help=SUPPRESS_HELP);
  parser.add_option("", "--settings",
                  action="store_true", dest="settings", default=False,
                  help="Create C Header file");
  parser.add_option("-f", "--file", dest="csv", default="settings.csv",
                  help="Settings file");
  (options, args) = parser.parse_args()

  ds = data_struct()

  if options.header:
    print "#ifndef EP_DS_H\n#define EP_DS_H\n"
    it = item('item')
    ds.ds_print(it)
    ep = ep_store_layout('ep_store_layout')
    ds.ds_print(ep)
    print "\n\n#endif"
  elif options.settings:
    pass;

  sys.exit (ds.error+error)
