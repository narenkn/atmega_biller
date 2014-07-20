from distutils.core import setup
import py2exe
import sys

prog = sys.argv.pop()
sys.argv.append('py2exe')

setup(
  options={'py2exe': dict(bundle_files=1, optimize=2)},
  console=[prog],
  zipfile=None,
  )
