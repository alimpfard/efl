#!/bin/python3

import platform
import os
import sys

default={}
default['linux']= ''
default['bsd']='-Dsystemd=false -Davahi=false -Deeze=false -Dx11=false -Dcocoa=true -Demotion-loaders-disabler=gstreamer,gstreamer1,libvlc,xine'
default['darwin']='-Dsystemd=false -Davahi=false -Deeze=false -Dx11=false -Dcocoa=true -Demotion-loaders-disabler=gstreamer,gstreamer1,libvlc,xine'

passed_args = sys.argv
passed_args.pop(0)

if '-h' in passed_args or '--help' in passed_args:
  print("Run ./setup.py [...]")
  print(" The additional arguments are passed to the meson build")
  print(" The rest of the build options are decided based on the platform")
  exit(1)

try:
    os.stat('./build')
except:
    os.mkdir('./build')

cmd = "meson build "+default[platform.system().lower()]+" "+(''.join(passed_args))

print("Executing build command: "+cmd)

os.system(cmd)
