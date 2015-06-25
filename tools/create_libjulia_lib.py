import re
import os.path 
import StringIO
import subprocess
import sys 
import _winreg 

# This script will create libjulia.lib from libjulia.dll using the C tools
# provided by Microsoft Visual Studio.

# Please note on 64-bit versions of Windows, registry operations to "HKLM\Software\..."
# are redirected to "HKLM\Software\Wow6432Node\..." 
# See https://msdn.microsoft.com/en-us/library/windows/desktop/aa384232%28v=vs.85%29.aspx 
VISUAL_STUDIO_2013_REGISTRY_PATH  = r"Software\Microsoft\VisualStudio\12.0" 
#VISUAL_STUDIO_2013_REGISTRY_PATH  = r"Software\Wow6432Node\Microsoft\VisualStudio\12.0" 
VISUAL_STUDIO_2012_REGISTRY_PATH  = r"Software\Microsoft\VisualStudio\11.0" 
#VISUAL_STUDIO_2012_REGISTRY_PATH  = r"Software\Wow6432Node\Microsoft\VisualStudio\11.0" 
VISUAL_STUDIO_SHELL_FOLDER_KEY    = "ShellFolder" 
VISUAL_C_BIN_FOLDER               = r"VC\bin"
LIBJULIA_DLL                      = "libjulia.dll"
LIBJULIA_DEF                      = "libjulia.def"
LIBJULIA_LIB                      = "libjulia.lib"
DUMPBIN_EXPORT_HEADER             = "ordinal hint"
DUMPBIN_EXPORT_FOOTER             = "Summary"

def usage():
   print "Usage: convert_libjulia <path_to_libjulia_bin_folder>"

def get_libjulia_path():
   if len(sys.argv) != 2:
      usage()
      sys.exit(1)
   julia_path = sys.argv[1]
   if not os.path.exists(julia_path): 
      print "ERROR: Unable to find " + julia_path 
      sys.exit(1)
   if not os.path.exists(os.path.join(julia_path, LIBJULIA_DLL)):
      print "ERROR: Unable to find " + LIBJULIA_DLL + " at " + julia_path 
      sys.exit(1)
   return julia_path

# Get a registry value from HKEY_LOCAL_MACHINE 
def get_hklm_value(reg_path, reg_key): 
   hKey = None 
   value = None 
   
   try: 
      hKey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, reg_path, _winreg.KEY_READ) 
      value, type = _winreg.QueryValueEx(hKey, reg_key) 
   except Exception, e: 
      print str(e) 
      pass 
   finally: 
      if hKey != None: 
         _winreg.CloseKey(hKey) 
   return value 
 
# Search the registry to find Visual Studio, giving preference to newer versions
def find_visual_studio(): 
   path = get_hklm_value(VISUAL_STUDIO_2013_REGISTRY_PATH, VISUAL_STUDIO_SHELL_FOLDER_KEY) 
   if path == None: 
      path = get_hklm_value(VISUAL_STUDIO_2012_REGISTRY_PATH, VISUAL_STUDIO_SHELL_FOLDER_KEY) 
   return path 

def get_symbol_name(temp):
   if "=" in temp:
      i = temp.index("=")
      sname = temp[0:i]
   else:
      sname = temp

   return sname

def create_def_file(vc_bin_path, libjulia_bin_path): 
   dumpbin_path = os.path.join(vc_bin_path, "dumpbin.exe")
   if not os.path.exists(dumpbin_path): 
      print "ERROR: Unable to find dumpbin at: " + dumpbin_path 
      sys.exit(1)
   dll_path = os.path.join(libjulia_bin_path, LIBJULIA_DLL)

   # Have dumpbin extract all the exports from the dll
   exports = subprocess.Popen([dumpbin_path, "/exports", dll_path],stdout=subprocess.PIPE).communicate()[0]
   #print exports

   def_file = os.path.join(libjulia_bin_path, LIBJULIA_DEF)
   fp = open(def_file,'w')
   fp.write('LIBRARY LIBJULIA\n')
   fp.write('EXPORTS\n')

   found_header = False
   for line in StringIO.StringIO(exports):
      if DUMPBIN_EXPORT_HEADER in line:
         found_header = True
         continue
      if DUMPBIN_EXPORT_FOOTER in line:
         break
      if found_header:
         values = line.split()
         if len(values) > 0:     # skip any blank lines
            symbol_name = get_symbol_name(values[3])
            fp.write(symbol_name + "\n")
   fp.close()
   return def_file

def create_lib(vc_bin_path, libjulia_bin_path): 
   lib_exe_path = os.path.join(vc_bin_path, "lib.exe")
   if not os.path.exists(lib_exe_path): 
      print "ERROR: Unable to find lib at: " + lib_exe_path 
      sys.exit(1)

   def_file_path = os.path.join(libjulia_bin_path, LIBJULIA_DEF)
   lib_file_path = os.path.join(libjulia_bin_path, LIBJULIA_LIB)

   results = subprocess.Popen([lib_exe_path, "/def:" + def_file_path, "/OUT:" + lib_file_path, "/machine:x64"],stdout=subprocess.PIPE).communicate()[0]
   #print results
   return lib_file_path

# Find libjulia
libjulia_bin_path = get_libjulia_path();

# Get the Visual Studio installation folder
vs_path = find_visual_studio() 
if vs_path == None: 
   print "ERROR: Unable to find Visual Studio installation folder." 
   sys.exit(1) 

# Make sure they installed Visual C++
vc_bin_path = os.path.join(vs_path, VISUAL_C_BIN_FOLDER) 
if not os.path.exists(vc_bin_path): 
   print "ERROR: Unable to find Visual C bin folder at: " + vc_bin_path 
   sys.exit(1) 

# Create our definition file
def_file_path = create_def_file(vc_bin_path, libjulia_bin_path) 
print "Created " + def_file_path

# Create libjulia.lib
lib_file_path = create_lib(vc_bin_path, libjulia_bin_path)
print "Created " + lib_file_path

