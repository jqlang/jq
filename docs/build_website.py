#!/usr/bin/env python3
import argparse
import glob
import itertools
from jinja2 import Environment, FileSystemLoader, select_autoescape, pass_context
from markdown import markdown
from markupsafe import Markup
import os
import os.path
import re
import shutil
import yaml

parser = argparse.ArgumentParser()
parser.add_argument('--root', default='/jq')
args = parser.parse_args()

env = Environment(
    loader=FileSystemLoader('templates'),
    autoescape=select_autoescape(['html.j2']),
)


def load_yml_file(fn):
    with open(fn) as f:
        return yaml.safe_load(f)


env.globals['url'] = 'https://jqlang.github.io/jq'
env.globals['root'] = args.root

env.filters['search_id'] = lambda input: input.replace(r'`', '')
env.filters['section_id'] = lambda input: re.sub(
    r'[^-a-zA-Z0-9_]', '', input.replace(' ', '-')).lower()
env.filters['entry_id'] = lambda input: re.sub(
    r'^(split|first-last-nth)$',
    r'\1' + ('-1' if ';' not in input else '-2'),  # avoid id conflict
    re.sub(
        r'\b([^-]+)(?:-\1)+\b',
        r'\1',  # e.g. range-range-range -> range
        re.sub(r' ?/ ?|,? ', '-',
               re.sub(r'[`;]|: .*|\(.*?\)| \[.+\]', '', input)))).lower()
env.filters['markdownify'] = lambda input: Markup(markdown(input))
env.filters['no_paragraph'] = lambda input: Markup(re.sub(r'</?p>', '', input))

env.globals['unique_id'] = pass_context(
    lambda ctx: str(next(ctx['unique_ctr'])))


def raise_handler(message):
    raise Exception(message)


env.globals['raise'] = raise_handler


def generate_file(env, fname):
    path, base = os.path.split(fname)
    path = os.path.relpath(path, 'content')
    if path == '.':
        path = ''
        permalink = ''
    else:
        permalink = path + '/'

    output_dir = os.path.join('output', path)
    output_path = os.path.join(output_dir, 'index.html')

    template_name = re.sub(r'.yml$', '.html.j2', base)

    ctx = load_yml_file(fname)
    ctx.update(unique_ctr=itertools.count(1),
               permalink=permalink,
               navitem=path)
    os.makedirs(output_dir, exist_ok=True)
    env.get_template(template_name).stream(ctx).dump(output_path,
                                                     encoding='utf-8')


def copy_public_files(root=''):
    for f in os.scandir(os.path.join('public', root)):
        src = os.path.join(root, f.name)
        dst = os.path.join('output', src)
        if f.is_dir():
            os.makedirs(dst, exist_ok=True)
            copy_public_files(src)
        else:
            shutil.copyfile(f.path, dst)


os.makedirs('output', exist_ok=True)
copy_public_files()

for fn in glob.glob('content/**/*.yml', recursive=True):
    generate_file(env, fn)
