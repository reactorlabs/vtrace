import os
import ycm_core

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )

def Settings( **kwargs ):
  return {
    'flags': [
      '-x',
      'c++',
      'std=gnu++17',
      '-I'+DirectoryOfThisScript()+'/../R-dyntrace/include',
      '-DNDEBUG',
      '-I'+DirectoryOfThisScript()+'/../instrumentr/inst/include',
      '-g3',
      '-O2',
      '-ggdb3',
      '-fpic',
      '-Wall',
      '-Wextra',
      '-fexceptions',
    ]
  }
