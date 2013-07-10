project = u'libqi API documentation'

extensions.extend(['qiapidoc', 'sphinx.ext.graphviz'])

import os

qiapidoc_srcs = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', 'build-doc', 'xml'))
