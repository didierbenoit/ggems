# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'GGEMS'
copyright = '2025, GGEMS Team'
author = 'GGEMS Developers'
release = '1.3'
version = '1.3'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_title = 'GGEMS v1.3'
html_baseurl = 'https://doc.ggems.fr/v1.3'
html_logo = '_static/images/ggems_logo_128_128.png'
html_favicon = '_static/images/ggems_logo_32_32.png'
html_theme_options = {
    "sidebar_hide_name": True,
    "navigation_with_keys": True,
    "light_css_variables": {
        "color-brand-primary": "#319B54",
        "color-brand-content": "#319B54"
    },

    "dark_css_variables": {
        "color-brand-primary": "#8ADAA5",
        "color-brand-content": "#8ADAA5"
    },

    "footer_icons": [
        {
            "name": "GitHub",
            "url": "https://github.com/GGEMS/ggems",
            "html": """
                <svg stroke="currentColor" fill="currentColor" stroke-width="0" viewBox="0 0 24 24">
                <path d="M22,12.247a10,10,0,0,1-6.833,9.488c-.507.1-.687-.214-.687-.481,0-.328.012-1.407.012-2.743a2.386,2.386,0,0,0-.679-1.852c2.228-.248,4.566-1.093,4.566-4.935a3.859,3.859,0,0,0-1.028-2.683,3.591,3.591,0,0,0-.1-2.647s-.838-.269-2.747,1.025a9.495,9.495,0,0,0-5.007,0c-1.91-1.294-2.75-1.025-2.75-1.025a3.6,3.6,0,0,0-.1,2.647A3.864,3.864,0,0,0,5.62,11.724c0,3.832,2.334,4.69,4.555,4.942A2.137,2.137,0,0,0,9.54,18a2.128,2.128,0,0,1-2.91-.831A2.1,2.1,0,0,0,5.1,16.142s-.977-.013-.069.608A2.646,2.646,0,0,1,6.14,18.213s.586,1.944,3.368,1.34c.005.835.014,1.463.014,1.7,0,.265-.183.574-.683.482A10,10,0,1,1,22,12.247Z"/>
                </svg>
            """,
            "class": "",
        },
    ],
}
html_show_sphinx = False
html_search_language = 'en'
html_show_sourcelink = False

pygments_style = "solarized-light"
pygments_dark_style = "solarized-dark"

templates_path = ['_templates']
html_static_path = ['_static']
html_css_files = ['css/custom.css']
