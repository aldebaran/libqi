# General information about the project.
project = u'LibQiType'
copyright = u'2012, platform@aldebaran-robotics.com'
version = release = '1.0'

htmlhelp_basename = 'LibQiTypedoc'

latex_documents = [
  ('index', 'LibQiType.tex', u'LibQiType Documentation',
   u'platform@aldebaran-robotics.com', 'manual'),
]

man_pages = [
    ('index', 'libqitype', u'LibQiType Documentation',
     [u'platform@aldebaran-robotics.com'], 1)
]

texinfo_documents = [
  ('index', 'LibQiType', u'LibQiType Documentation',
   u'platform@aldebaran-robotics.com', 'LibQiType', 'One line description of project.',
   'Miscellaneous'),
]

extensions.append('qiapidoc')

qiapidoc_srcs = os.path.realpath(os.path.join(
    os.path.dirname(__file__), '..', '..', 'build-doc', 'xml'
))
