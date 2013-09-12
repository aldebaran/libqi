project = u'qi SDK Documentation'

extensions.extend(['sphinx.ext.autodoc',
                   'qiapidoc',
                   'sphinx.ext.graphviz'])

extensions.extend(['sphinx.ext.autodoc', 'qiapidoc' ,'sphinx.ext.graphviz'])

templates_path = ['../source/_templates']

html_additional_pages = {
    'contents' : 'contents.html'
}

html_theme_path = ["../source/_themes"]
html_theme = "alcodedocs"
