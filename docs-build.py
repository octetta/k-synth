#!/usr/bin/env python3
"""
docs-build.py — convert guide.md and readme.md to HTML using the doc-template.

Uses pandoc if available, otherwise falls back to the 'markdown' Python package.
Install the fallback with: pip install markdown

Usage: python3 docs-build.py
Output: guide.html, readme.html
"""

import sys
import os
import subprocess
import re

TEMPLATE = 'doc-template.html'

DOCS = [
    {
        'src':   'reference.md',
        'out':   'reference.html',
        'title': 'ksynth — verb reference',
        'nav':   '<a class="doc-nav-btn" href="index.html">&#8592; app</a>'
                 '<a class="doc-nav-btn" href="guide.html">guide</a>'
                 '<a class="doc-nav-btn" href="readme.html">readme</a>',
    },

    {
        'src':   'guide.md',
        'out':   'guide.html',
        'title': 'ksynth web — user guide',
        'nav':   '<a class="doc-nav-btn" href="index.html">&#8592; app</a>'
                 '<a class="doc-nav-btn" href="reference.html">reference</a>'
                 '<a class="doc-nav-btn" href="readme.html">readme</a>',
    },
    {
        'src':   'readme.md',
        'out':   'readme.html',
        'title': 'ksynth — readme',
        'nav':   '<a class="doc-nav-btn" href="index.html">&#8592; app</a>'
                 '<a class="doc-nav-btn" href="guide.html">guide</a>'
                 '<a class="doc-nav-btn" href="reference.html">reference</a>',
    },
]


def md_to_html_pandoc(src):
    result = subprocess.run(
        ['pandoc', '-f', 'markdown', '-t', 'html', '--no-highlight', src],
        capture_output=True, text=True, check=True
    )
    return result.stdout


def md_to_html_python(src):
    try:
        import markdown
    except ImportError:
        print('ERROR: pandoc not found and "markdown" package not installed.')
        print('Install with: pip install markdown')
        sys.exit(1)
    with open(src) as f:
        text = f.read()
    return markdown.markdown(
        text,
        extensions=['fenced_code', 'tables', 'toc'],
        output_format='html'
    )


def convert(src):
    """Try pandoc first, fall back to python-markdown."""
    try:
        subprocess.run(['pandoc', '--version'], capture_output=True, check=True)
        return md_to_html_pandoc(src)
    except (subprocess.CalledProcessError, FileNotFoundError):
        return md_to_html_python(src)


def build(doc, template):
    body = convert(doc['src'])
    html = template
    html = html.replace('{{TITLE}}', doc['title'])
    html = html.replace('{{NAV}}',   doc['nav'])
    html = html.replace('{{BODY}}',  body)
    with open(doc['out'], 'w') as f:
        f.write(html)
    print(f"  {doc['src']} -> {doc['out']}")


def main():
    if not os.path.exists(TEMPLATE):
        print(f'ERROR: {TEMPLATE} not found')
        sys.exit(1)
    with open(TEMPLATE) as f:
        template = f.read()
    print('Building docs...')
    for doc in DOCS:
        build(doc, template)
    print('Done.')


if __name__ == '__main__':
    main()
