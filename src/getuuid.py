#!/usr/bin/python3
from uuid import uuid4

def getuuid():
	return str(uuid4()).encode("UTF8")