#!/usr/bin/env python3
import sys
import os
import datetime

HEADER_TEMPLATE = \
"HTTP/1.1 200 OK\r\n" \
"Server: webhttpd/2.0\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: image/svg+xml\r\n" \
"Content-Length: {}\r\n" \
"Date: {}\r\n" \
"\r\n\r\n"

fp = open(os.environ['WORKING_DIR'] + "/index_files/webserver.svg", "rb")
binary = fp.read()

sys.stdout.write(HEADER_TEMPLATE.format(len(binary), datetime.datetime.now().strftime("%a, %d %b %Y %H:%M:%S GMT")))
sys.stdout.buffer.write(binary)

fp.close()
