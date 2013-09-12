project = u'qi Messaging'

#provided by qiapidoc extensions += ['mycpp' ]

extensions.extend(['sphinx.ext.autodoc', 'qiapidoc' ,'sphinx.ext.graphviz'])

templates_path = ['../source/_templates']

html_additional_pages = {
    'contents' : 'contents.html'
}

