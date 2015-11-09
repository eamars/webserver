#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import datetime

HEADER_TEMPLATE = \
"HTTP/1.1 200 OK\r\n" \
"Server: webhttpd/3.0\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: text/html; charset=utf-8\r\n" \
"Date: {}\r\n" \
"\r\n"

HTML = \
"""
<!DOCTYPE html>
<html>
<body>

<form action="post" method="post">
  First name: <input type="text" name="fname"><br>
  Last name: <input type="text" name="lname"><br>
  <input type="submit" value="Submit">
</form>

<p>Click on the submit button, and the input will be sent to a page on the server called "demo_form_method_post.asp".</p>

</body>
</html>
"""


def main():
    sys.stdout.write(HEADER_TEMPLATE.format(datetime.datetime.now().strftime("%a, %d %b %Y %H:%M:%S GMT")))
    sys.stdout.write(HTML)
    sys.stdout.write("\r\n\r\n")


if __name__ == "__main__":
    main()
