def load_shlib(name, sdkDir, verbose=False):
  """ Load a shared library, try to handle as many cases as possible
  """
  import ctypes
  import os
  import sys
  if verbose:
    print("PATH: " + os.environ.get('PATH', ''))
    print("LD_LIBRARY_PATH: " + os.environ.get('LD_LIBRARY_PATH', ''))
    print("DYLD_LIBRARY_PATH: " + os.environ.get('DYLD_LIBRARY_PATH', ''))
  if sys.platform.startswith('linux'):
    soname = [".so"]
    prefix = "lib"
    paths = ['', '../lib', os.path.join(sdkDir, 'lib')]
  elif sys.platform.startswith('darwin'):
    soname = [".dylib"]
    prefix = "lib"
    paths = ['', '../lib', os.path.join(sdkDir, 'lib')]
  else:
    # Windows has no RPATH equivalent, but allows for changing the
    # search path within the process.
    ctypes.windll.kernel32.SetDllDirectoryA(os.path.join(sdkDir, 'bin'))
    soname = [".dll", "_d.dll"]
    prefix = ''
    paths = ['', '.', os.path.join(sdkDir, 'lib'), os.path.join(sdkDir, 'bin')]

  if verbose:
    print("Loading " + name)
  handle = None
  for s in soname:
    for p in paths:
      path = os.path.join(p, prefix + name + s)
      try:
        handle = ctypes.cdll.LoadLibrary(path)
        break
      except Exception as e:
        if verbose:
          print(path + " : " + str(e))
  return handle
