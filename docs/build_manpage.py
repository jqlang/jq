#!/usr/bin/env python3
from datetime import date
from io import StringIO
from lxml import etree
import markdown
from markdown.extensions import Extension
import re
import sys
import yaml


# Prevent our markdown parser from trying to help by interpreting things in angle brackets as HTML tags.
class EscapeHtml(Extension):

    def extendMarkdown(self, md):
        md.preprocessors.deregister('html_block')
        md.inlinePatterns.deregister('html')


class RoffWalker(object):

    def __init__(self, tree, output=sys.stdout):
        self.tree = tree
        self.target = output
        self.f = StringIO()

    def walk(self):
        self._walk(self.tree, parent_tag=None)
        # We don't want to start lines with \. because that can confuse man
        # For lines that start with \., we need to prefix them with \& so it
        # knows not to treat that line as a directive
        data = re.sub(r'^\\\.', r'\&.', self.f.getvalue(), flags=re.MULTILINE)
        self.target.write(data)

    def _ul_is_special(self, root):
        if len(root) != 1:
            return False
        child = root[0]
        if child.tag != 'li':
            return False
        msg = ''.join(child.itertext()).strip()
        return msg.endswith(':')

    def _walk_child(self, root):
        if len(root) > 0:
            self._walk(root[0], parent_tag=root.tag)

    def _write_element(self, root, ensure_newline=True):
        if root.text is not None:
            text = self._sanitize(root.text)
            self.__write_raw(text)
        self._walk_child(root)
        self._write_tail(root, ensure_newline=ensure_newline)

    def _write_tail(self, root, ensure_newline=False, inline=False):
        if root.tail is not None:
            if inline or root.tail != '\n':
                text = self._sanitize(root.tail)
                if text.endswith('\n'):
                    ensure_newline = False
                self.__write_raw(text)
        if ensure_newline:
            self.__write_raw('\n')

    def _walk(self, root, parent_tag=None):
        last_tag = None
        while root is not None:
            if root.tag == 'h1':
                self.__write_cmd('.TH "JQ" "1" "{}" "" ""'.format(
                    date.today().strftime('%B %Y')))
                self.__write_cmd('.SH "NAME"')
                # TODO: properly parse this
                self.__write_raw(r'\fBjq\fR \- Command\-line JSON processor' +
                                 "\n")

            elif root.tag == 'h2':
                self.__write_cmd('.SH "{}"'.format(''.join(
                    root.itertext()).strip()))

            elif root.tag == 'h3':
                text = ''.join(root.itertext()).strip()
                self.__write_cmd('.SS "{}"'.format(self._h3_sanitize(text)))

            elif root.tag == 'p':
                if last_tag not in ['h2', 'h3'] and parent_tag not in ['li']:
                    self.__write_cmd('.P')
                self._write_element(root, ensure_newline=(parent_tag != 'li'))

            elif root.tag == 'a':
                self._write_element(root, ensure_newline=(parent_tag != 'li'))

            elif root.tag == 'ul':
                if self._ul_is_special(root):
                    li = root[0]
                    self.__write_cmd('.TP')
                    self._write_element(li)
                    next = root.getnext()
                    while next is not None and next.tag == 'p':
                        if next.getnext() is not None and next.getnext(
                        ).tag == 'pre':
                            # we don't want to .IP these, because it'll look funny with the code indent
                            break
                        self.__write_cmd('.IP')
                        self._write_element(next)
                        root = next
                        next = root.getnext()
                else:
                    self._walk_child(root)
                    self._write_tail(root)
                    # A pre tag after the end of a list doesn't want two of the indentation commands
                    if root.getnext() is None or root.getnext().tag != 'pre':
                        self.__write_cmd('.IP "" 0')

            elif root.tag == 'li':
                self.__write_cmd(r'.IP "\(bu" 4')
                if root.text is not None and root.text.strip() != '':
                    text = self._sanitize(root.text)
                    self.__write_raw(text)
                self._walk_child(root)
                self._write_tail(root, ensure_newline=True)

            elif root.tag == 'strong':
                if root.text is not None:
                    text = self._sanitize(root.text)
                    self.__write_raw('\\fB{}\\fR'.format(text))

                self._write_tail(root, inline=True)

            elif root.tag == 'em':
                if root.text is not None:
                    text = self._sanitize(root.text)
                    self.__write_raw('\\fI{}\\fR'.format(text))
                self._write_tail(root, inline=True)

            elif root.tag == 'code':
                if root.text is not None:
                    text = self._code_sanitize(root.text)
                    self.__write_raw('\\fB{}\\fR'.format(text))
                self._write_tail(root, inline=True)

            elif root.tag == 'pre':
                self.__write_cmd('.IP "" 4')
                self.__write_cmd('.nf\n')  # extra newline for spacing reasons
                next = root
                first = True
                while next is not None and next.tag == 'pre':
                    if not first:
                        self.__write_raw('\n')
                    text = ''.join(next.itertext(with_tail=False))
                    self.__write_raw(self._pre_sanitize(text))
                    first = False
                    root = next
                    next = next.getnext()
                self.__write_cmd('.fi')
                self.__write_cmd('.IP "" 0')

            else:
                self._walk_child(root)

            last_tag = root.tag
            root = root.getnext()

    def _base_sanitize(self, text):
        text = re.sub(r'\\', r'\\e', text)
        text = re.sub(r'\.', r'\\.', text)
        text = re.sub("'", r"\'", text)
        text = re.sub('-', r'\-', text)
        return text

    def _pre_sanitize(self, text):
        return self._base_sanitize(text)

    def _code_sanitize(self, text):
        text = self._base_sanitize(text)
        text = re.sub(r'\s', ' ', text)
        return text

    def _h3_sanitize(self, text):
        text = self._base_sanitize(text)
        text = re.sub(' \n|\n ', ' ', text)
        text = re.sub('\n', ' ', text)
        return text

    def _sanitize(self, text):
        text = self._base_sanitize(text)
        text = re.sub(r'<([^>]+)>', r'\\fI\1\\fR', text)
        text = re.sub(r' +', ' ', text)
        text = re.sub('\n', ' ', text)
        return text

    def __write_cmd(self, dat):
        print('.', dat, sep='\n', file=self.f)
        pass

    def __write_raw(self, dat):
        print(dat, sep='', end='', file=self.f)
        pass


def load_yml_file(fn):
    with open(fn) as f:
        return yaml.safe_load(f)


def dedent_body(body):
    lines = [re.sub(r'^  (\S)', r'\1', l) for l in body.split('\n')]
    return '\n'.join(lines)


def convert_manual_to_markdown():
    f = StringIO()
    manual = load_yml_file("content/manual/manual.yml")
    f.write(manual.get('manpage_intro', '\n'))
    f.write(dedent_body(manual.get('body', '\n')))
    for section in manual.get('sections', []):
        f.write('## {}\n'.format(section.get('title', '').upper()))
        f.write(dedent_body(section.get('body', '\n')))
        f.write('\n')
        for entry in section.get('entries', []):
            f.write('### {}\n'.format(entry.get('title', '')))
            f.write(dedent_body(entry.get('body', '\n')))
            f.write('\n')
            if entry.get('examples') is not None:
                f.write("~~~~\n")
                first = True
                for example in entry.get('examples'):
                    if not first:
                        f.write('\n')
                    f.write("jq '{}'\n".format(example.get('program', '')))
                    f.write("   {}\n".format(example.get('input', '')))
                    output = [str(x) for x in example.get('output', [])]
                    f.write("=> {}\n".format(', '.join(output)))
                    first = False
                f.write("~~~~\n")
        f.write('\n')
    f.write(manual.get('manpage_epilogue', ''))
    return f.getvalue()


# Convert manual.yml to our special markdown format
markdown_data = convert_manual_to_markdown()

# Convert markdown to html
html_data = markdown.markdown(markdown_data,
                              extensions=[EscapeHtml(), 'fenced_code'])

# Parse the html into a tree so we can walk it
tr = etree.HTML(html_data, etree.HTMLParser())

# Convert the markdown to ROFF
RoffWalker(tr).walk()
