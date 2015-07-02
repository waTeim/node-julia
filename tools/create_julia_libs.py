import argparse
import os.path 
import StringIO
import subprocess
import sys 
import _winreg 

# This script will create a .lib from a given .dll using the C tools provided
# by Microsoft Visual Studio.

# Please note on 64-bit versions of Windows, registry operations to "HKLM\Software\..."
# are redirected to "HKLM\Software\Wow6432Node\..." 
# See https://msdn.microsoft.com/en-us/library/windows/desktop/aa384232%28v=vs.85%29.aspx 
VISUAL_STUDIO_2013_REGISTRY_PATH  = r"Software\Microsoft\VisualStudio\12.0" 
#VISUAL_STUDIO_2013_REGISTRY_PATH  = r"Software\Wow6432Node\Microsoft\VisualStudio\12.0" 
VISUAL_STUDIO_2012_REGISTRY_PATH  = r"Software\Microsoft\VisualStudio\11.0" 
#VISUAL_STUDIO_2012_REGISTRY_PATH  = r"Software\Wow6432Node\Microsoft\VisualStudio\11.0" 
VISUAL_STUDIO_SHELL_FOLDER_KEY    = "ShellFolder" 
VISUAL_C_BIN_FOLDER               = r"VC\bin"
DUMPBIN_EXPORT_HEADER             = "ordinal hint"
DUMPBIN_EXPORT_FOOTER             = "Summary"

# Override argparse's default error handler so that it displays the "full" help, rather
# than an abbreviated one on errors.
class MyParser(argparse.ArgumentParser): 
   def error(self, message):
      sys.stderr.write('Error: %s\n' % message)
      self.print_help()
      sys.exit(1)

def parse_command_line():
   try:
      parser = MyParser()
	  
	  # Add the required arguments to a separate group.  If we don't do this, print_help will
	  # show them under "optional arguments", even though they are marked as required.
      required_args = parser.add_argument_group('required arguments')
      required_args.add_argument('--bin', dest = 'bin_folder', required=True, help = 'path to the Julia bin folder')
      required_args.add_argument('--files', dest = 'files', required=True, nargs = '+', help = 'list of dll files to process')

      args = parser.parse_args()
   except Exception, e: 
#      print str(e) 
      parser.print_help()
      pass 
      sys.exit(1)

   if not os.path.exists(args.bin_folder): 
      print "ERROR: Unable to find " + args.bin_folder 
      sys.exit(1)
   for dll_name in args.files:
      dll_file_path = os.path.join(args.bin_folder, dll_name)
      if not os.path.exists(dll_file_path):
         print "ERROR: Unable to find " + dll_file_path
         sys.exit(1)
   return args.bin_folder, args.files

# Get a registry value from HKEY_LOCAL_MACHINE 
def get_hklm_value(reg_path, reg_key): 
   hKey = None 
   value = None 
   
   try: 
      hKey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, reg_path, _winreg.KEY_READ) 
      value, type = _winreg.QueryValueEx(hKey, reg_key) 
   except Exception, e: 
      #print str(e) 
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

def create_def_file(dll_file_name, libjulia_bin_path, vc_bin_path): 
   dumpbin_path = os.path.join(vc_bin_path, "dumpbin.exe")
   if not os.path.exists(dumpbin_path): 
      print "ERROR: Unable to find dumpbin at: " + dumpbin_path 
      sys.exit(1)
   dll_basename = os.path.splitext(dll_file_name)[0]
   dll_path = os.path.join(libjulia_bin_path, dll_file_name)

   # Have dumpbin extract all the exports from the dll
   exports = subprocess.Popen([dumpbin_path, "/exports", dll_path],stdout=subprocess.PIPE).communicate()[0]
   #print exports

   def_file = os.path.join(libjulia_bin_path, dll_basename + '.def')
   fp = open(def_file,'w')
   fp.write('LIBRARY ' + dll_basename.upper() + '\n')
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

def create_lib(dll_file_name, libjulia_bin_path, vc_bin_path): 
   lib_exe_path = os.path.join(vc_bin_path, "lib.exe")
   if not os.path.exists(lib_exe_path): 
      print "ERROR: Unable to find lib at: " + lib_exe_path 
      sys.exit(1)

   dll_basename = os.path.splitext(dll_file_name)[0]
   def_file_path = os.path.join(libjulia_bin_path, dll_basename + '.def')
   lib_file_path = os.path.join(libjulia_bin_path, dll_basename + '.lib')

   results = subprocess.Popen([lib_exe_path, "/def:" + def_file_path, "/out:" + lib_file_path, "/machine:x64"],stdout=subprocess.PIPE).communicate()[0]
   #print results
   return lib_file_path

# Get our Julia bin path and a list of dlls from the command line
libjulia_bin_path, dll_list = parse_command_line()

# Get the Visual Studio installation folder
vs_path = find_visual_studio() 
if vs_path == None: 
   print "ERROR: Unable to find the Visual Studio installation folder." 
   sys.exit(1) 

# Make sure they installed Visual C++
vc_bin_path = os.path.join(vs_path, VISUAL_C_BIN_FOLDER) 
if not os.path.exists(vc_bin_path): 
   print "ERROR: Unable to find Visual C bin folder at: " + vc_bin_path 
   sys.exit(1) 

# Create our definition files
for dll_file_name in dll_list:
   def_file_path = create_def_file(dll_file_name, libjulia_bin_path, vc_bin_path) 
   print "Created " + def_file_path

   # Create our .lib file
   lib_file_path = create_lib(dll_file_name, libjulia_bin_path, vc_bin_path)
   print "Created " + lib_file_path