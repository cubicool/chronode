#!/usr/bin/env python3
#vimrun! ./test-json.py test5.json

import sys
import json

import IPython

def print_node(n, d=0):
	print(f"{'   ' * d}{n['name']}: {n['start']} -> +{n['stop'] - n['start']}")

	for c in n["children"]:
		print_node(c, d + 1)

if __name__ == "__main__":
	d = json.loads(open(sys.argv[1]).read())

	for timer in d["data"]:
		print_node(timer)

	# IPython.embed()
