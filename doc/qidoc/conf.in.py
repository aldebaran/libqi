project = u'qi Messaging'

#sys.path.insert(0, os.path.abspath('..'))
sys.path.insert(0, os.path.abspath('/home/ctaf/src/qi-naoqi2/lib/qimessaging/doc/../../../tools/qibuild/python/qiapidoc'))
extensions += [ 'extendcpp' ]
qiapidoc_srcs = [os.path.abspath('../libqimessaging/build-doc/xml')]
