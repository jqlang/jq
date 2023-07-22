#!/usr/bin/env python3
import jsonschema
import sys
import yaml

if len(sys.argv) < 2:
    sys.exit("Usage: {} MANUAL_FILE ...".format(sys.argv[0]))

with open("manual_schema.yml", "r") as schema_file:
    manual_schema = yaml.safe_load(schema_file)

for path in sys.argv[1:]:
    with open(path, "r") as manual_file:
        manual_data = yaml.safe_load(manual_file)
    try:
        jsonschema.validate(instance=manual_data, schema=manual_schema)
    except jsonschema.exceptions.ValidationError as e:
        print("Failed to validate:", path, file=sys.stderr)
        sys.exit(e)
