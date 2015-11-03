#!/usr/bin/env python3
import sys
import os

fp = open(os.environ['WORKING_DIR'] + "/index_files/scale.fix.js", "rb")
sys.stdout.buffer.write(fp.read())
