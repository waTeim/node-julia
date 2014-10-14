import os
import sys
import subprocess


def julia_from_which_julia():
   path = "";
   which_julia = subprocess.Popen(["/usr/bin/which","julia"],stdout=subprocess.PIPE).communicate()[0];
   if len(which_julia) > 0:
      real_path = os.path.realpath(which_julia.split()[0])
      if real_path:
         head,tail = os.path.split(real_path)
         path,tail = os.path.split(head)
   return path 

def julia_from_home_directory():
   home = os.path.expanduser("~")
   julia_dir = os.path.join(home,"julia")
   if os.path.isdir(julia_dir): return julia_dir
   return "";

def julia_from_applications():
   julia_dir = "/Applications/Julia-0.3.0.app/Contents/Resources/julia/"
   if os.path.isdir(julia_dir): return julia_dir
   return ""

if sys.argv[1] == "mac" or sys.argv[1] == "linux": path = julia_from_which_julia()
if path == "": path = julia_from_home_directory()
if path == "" and len(sys.argv) > 1 and sys.argv[1] == "mac": path = julia_from_applications()

if not path == "": print path
