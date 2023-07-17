#!/usr/bin/env python3
import yaml
import re

regex_program_pattern = re.compile(
    r'\b(?:test|match|capture|scan|split|splits|sub|gsub)\s*\(')

with open('content/manual/manual.yml') as source, \
        open('../tests/man.test', 'w') as man, \
        open('../tests/manonig.test', 'w') as manonig:
    manual = yaml.safe_load(source)
    for section in manual.get('sections', []):
        for entry in section.get('entries', []):
            for example in entry.get('examples', []):
                program = example.get('program', '').replace('\n', ' ')
                out = manonig if regex_program_pattern.search(program) else man
                print(program, file=out)
                print(example.get('input', ''), file=out)
                for s in example.get('output', []):
                    print(s, file=out)
                print('', file=out)
