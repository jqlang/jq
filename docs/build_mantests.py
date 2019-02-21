#!/usr/bin/env python3
import yaml

with open("content/manual/manual.yml") as f:
  manual = yaml.load(f)
  for section in manual.get('sections', []):
    for entry in section.get('entries', []):
      for example in entry.get('examples', []):
        print(example.get('program', '').replace('\n', ' '))
        print(example.get('input', ''))
        for s in example.get('output', []):
          print(s)
        print('')
