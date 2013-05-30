import qi

class TestPython:
   def __init__(self):
      pass

   def get(self):
      return 42

class TestPython2:
   def __init__(self):
      pass

   def get(self):
      return 43

qi.registerObjectFactory("TestPython", TestPython)
qi.registerObjectFactory("TestPython2", TestPython2)
