project = u'libqi API documentation'

extensions.append('qiapidoc')

import os

qiapidoc_srcs = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', 'build-doc', 'xml'))
qiapidoc_gen_skeleton = '/tmp/skeleton'
