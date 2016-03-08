#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import urllib.parse
from datetime import datetime
from random import randint
from sql import *

HTML_DATE_FMT = "%Y-%m-%d"

SQL_CONFIG = {
    "host": "192.168.2.5",
    "user": "eamars",
    "password": "931105",
    "autocommit": True
}

# Database name in MySQL database
DB_NAME = "test"

# Table name in MySQL database
TABLE_NAME = "roster"

HEADER_TEMPLATE = \
"HTTP/1.1 200 OK\r\n" \
"Server: webhttpd/2.0\r\n" \
"Cache-Control: no-cache, no-store, must-revalidate\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: text/html; charset=utf-8\r\n" \
"Date: {}\r\n" \
"\r\n"

HEADER = """
<html>
    <head>
        <title>Roster</title>
    </head>
    <body>
        <h3>Roster</h3>
"""

DATETIME_PICKER = """
<form action="index" method="post" id="usrform">
    Select a date: 
    <input type="date" name="date" value="{}">
    <input type="submit">
</form>
"""

def isDate(string):
    try:
        datetime.strptime(string, HTML_DATE_FMT)
        return True
    except Exception as e:
        return False

def random_staff():
    staffList = ["Ran Bao", "Jamie Getty", "Brook Queree", "James Stewart"]

    # Pick chair first
    chair_index = randint(0, 3)
    chair = staffList[chair_index]
    staffList.pop(chair_index)

    # Pick minute then
    minute_index = randint(0, 2)
    minute = staffList[minute_index]
    staffList.pop(minute_index)

    return chair, minute

def pick_staff(connection, date):
    sql_query = "date='{}'".format(date)

    # The staff has already been generated
    if (entry_exists(connection, TABLE_NAME, sql_query)):
        chair, minute = fetch_entry(connection, TABLE_NAME, sql_query)
    else:
        # Generate something random and add to db
        chair, minute = random_staff()
        value = "(date, chair, minute) VALUES('{}', '{}', '{}')".format(date, chair, minute)
        insert_entry(connection, TABLE_NAME, value)

    sys.stdout.write("<b>{}<b><br><b>Chair: {}</b><br><b>Minute</b>: {}".format(date, chair, minute))



def handle_input(connection, query):
    query = urllib.parse.unquote_plus(query)

    date = ""
    splited = query.split("=")
    if (splited[0] == "date"):
        date = splited[1]
        if (isDate(date)):
            pick_staff(connection, date)

    



def main():
    get_query = urllib.parse.unquote(os.environ.get('GET_QUERY'))
    post_query = urllib.parse.unquote(os.environ.get('POST_QUERY'))


    sys.stdout.write(HEADER_TEMPLATE.format(datetime.now().strftime("%a, %d %b %Y %H:%M:%S GMT")))
    sys.stdout.write(HEADER)
    sys.stdout.write(DATETIME_PICKER.format(datetime.now().strftime(HTML_DATE_FMT)))

    # Connect to database
    connection = establish_connection(SQL_CONFIG)
    connect_database(connection, DB_NAME)

    entry_exists(connection, TABLE_NAME, "date=1970-01-01")

    # User input
    if get_query:
        handle_input(connection, get_query)
    if post_query:
        handle_input(connection, post_query)

    # Write terminator
    sys.stdout.write("</table></body></html>")
    sys.stdout.write("\r\n\r\n")


if __name__ == "__main__":
    main()
