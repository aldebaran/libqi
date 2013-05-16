#! /usr/bin/env python

## Copyright (c) 2013 Aldebaran Robotics. All rights reserved.



import sys
import os

def main(args):
  if len(args) != 3:
    print('Usage: %s URL DIR\n\t create AL specialized proxy for each live AL module, in DIR.' % args[0])
    return
  sd_url = args[1]
  prefix = args[2]
  abver = '.'.join(sys.version.split('.', 2)[0:2])
  me = os.path.dirname(os.path.abspath(__file__))
  lpath = me + '/../lib/python'+ abver + '/site-packages'
  sys.path.append(lpath)
  from qi import Session
  session = Session()
  session.connect(sd_url)
  services = session.services(0)
  names = map(lambda x: x[0], services)
  print(names)
  for s in names:
    if s[0] == '_':
      continue
    fname = s.lower() + 'proxy'
    pathes = os.path.join(prefix, "alproxies", fname + '.h') + ',' + os.path.join(prefix, "src", fname + '.cpp')
    cmd = ['idl.py', sd_url + '/' + s, '--output-mode=alproxy', '-o', pathes]
    os.system(' '.join(cmd))

main(sys.argv)
