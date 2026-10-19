project = 'C Programming Mastery'
author = 'Curriculum contributors'
copyright = '2026, curriculum contributors'
extensions = ['myst_parser', 'sphinx.ext.mathjax']
source_suffix = {'.md': 'markdown'}
root_doc = 'index'
html_theme = 'sphinx_book_theme'
html_title = 'C Mastery · Learn by building'
html_theme_options = {'show_toc_level': 2, 'navigation_with_keys': False, 'use_download_button': False, 'use_fullscreen_button': False,
                      # Omit unrelated sidebar subtrees; retain every page and section index.
                      'collapse_navbar': True}
html_static_path = ['_static']
html_css_files = ['curriculum.css']
myst_enable_extensions = ['dollarmath', 'colon_fence', 'deflist']
nitpicky = True
smartquotes = False
# Sphinx prefixes relative JavaScript paths with _static automatically.
mathjax_path = 'mathjax/tex-mml-chtml.js'
html_show_sourcelink = False
html_copy_source = False
html_last_updated_fmt = None
# Raw learner downloads remain assets, including Markdown originals. They are
# not handbook documents (also for builders without HTML asset discovery).
exclude_patterns = ['fixtures/**', '_static/learner_sources/**']
