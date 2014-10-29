import os
import re
import subprocess

def node_version():
   version = subprocess.Popen(["node","--version"],stdout=subprocess.PIPE).communicate()[0];
   if len(version) > 0:
      version = re.sub("^v([0-9]*\.[0-9]*\.)([0-9]*).*$", "\g<1>x", version).rstrip(os.linesep)
   return version

version = node_version()
print version
