import os
project_name = u"libqi api"
extensions.append('qiapidoc')
qiapidoc_srcs = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', 'build-doc', 'xml'))
qiapidoc_gen_skeleton = '/tmp/skeleton'
