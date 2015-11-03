#!/usr/bin/env python3
import sys
import os

fp = open(os.environ['WORKING_DIR'] + "/favicon.ico", "rb")
sys.stdout.buffer.write(fp.read())
