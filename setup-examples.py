#!/bin/python3
import subprocess
import argparse
import os

parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('--examples-path', type=str, default="examples", dest='examples_path', help='path to the examples repository')

args = parser.parse_args()
build_dir = os.path.join("build-examples")

#setup if not existing
if os.path.exists(args.examples_path) == False:
  if subprocess.call("git clone https://git.enlightenment.org/tools/examples.git "+args.examples_path, shell=True) != 0:
    exit(1)

if subprocess.call("cd "+args.examples_path+" && ./setup.py", shell=True) != 0:
  exit(1)

if os.path.exists(os.path.join(args.examples_path, "meson.build")) == False:
  print("meson build file not found")
  exit(1)
try:
  os.mkdir(build_dir)
except:
  pass

if subprocess.call("meson "+build_dir+" "+args.examples_path, shell=True) != 0:
  exit(1)
