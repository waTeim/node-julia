import os
import sys
import subprocess


def julia_from_which_julia():
   path = '';
   which_julia = subprocess.Popen(["/usr/bin/which","julia"],stdout=subprocess.PIPE).communicate()[0].split()[0]
   if which_julia:
      real_path = os.path.realpath(which_julia)
      if real_path:
         path = '';
         for component in real_path.split("/")[1:-2]:
            path += "/" + component
   return path

path = julia_from_which_julia()

if path: print path
