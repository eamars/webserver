#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os

fp = open(os.environ['WORKING_DIR'] + "/robots.txt", "rb")
sys.stdout.buffer.write(fp.read())
