#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import urllib.parse
import datetime
from sql import *

SQL_CONFIG = {
    "host": "192.168.2.5",
    "user": "eamars",
    "password": "931105",
    "autocommit": True
}

# Database name in MySQL database
DB_NAME = "test"

# Table name in MySQL database
TABLE_NAME = "forum_posts"

HEADER_TEMPLATE = \
"HTTP/1.1 200 OK\r\n" \
"Server: webhttpd/2.0\r\n" \
"Cache-Control: no-cache, no-store, must-revalidate\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: text/html; charset=utf-8\r\n" \
"Date: {}\r\n" \
"\r\n"

HTML = """
<html>
    <head>
        <title>Forum</title>
            <style>
                table {
                    white-space: nowrap;
                    font-family: 'Arial';
                    margin: 25px auto;
                    border-collapse: collapse;
                    border: 1px solid #eee;
                    border-bottom: 2px solid #00cccc;
                    box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.1), 0px 10px 20px rgba(0, 0, 0, 0.05), 0px 20px 20px rgba(0, 0, 0, 0.05), 0px 30px 20px rgba(0, 0, 0, 0.05);
                }
                table tr:hover {
                    background: #f4f4f4;
                }
                table tr:hover td {
                    color: #555;
                }
                table th, table td {
                    color: #999;
                    border: 1px solid #eee;
                    padding: 12px 35px;
                    border-collapse: collapse;
                }
                table th {
                    background: #00cccc;
                    color: #fff;
                    text-transform: uppercase;
                    font-size: 12px;
                }
                table th.last {
                    border-right: none;
                }
                h3 {
                    font:1.2em normal Arial,sans-serif;
                    color:#34495E;
                    text-align:center;
                    letter-spacing:-2px;
                    font-size:2.5em;
                    margin:20px 0;
                }
                .form-style-6{
                    font: 95% Arial, Helvetica, sans-serif;
                    max-width: 400px;
                    margin: 10px auto;
                    padding: 16px;
                    border: 1px solid #eee;
                }
                .form-style-6 h1{
                    background: #43D1AF;
                    padding: 20px 0;
                    font-size: 140%;
                    font-weight: 300;
                    text-align: center;
                    color: #fff;
                    margin: -16px -16px 16px -16px;
                }
                .form-style-6 input[type="text"],
                .form-style-6 input[type="date"],
                .form-style-6 input[type="datetime"],
                .form-style-6 input[type="email"],
                .form-style-6 input[type="number"],
                .form-style-6 input[type="search"],
                .form-style-6 input[type="time"],
                .form-style-6 input[type="url"],
                .form-style-6 textarea,
                .form-style-6 select
                {
                    -webkit-transition: all 0.30s ease-in-out;
                    -moz-transition: all 0.30s ease-in-out;
                    -ms-transition: all 0.30s ease-in-out;
                    -o-transition: all 0.30s ease-in-out;
                    outline: none;
                    box-sizing: border-box;
                    -webkit-box-sizing: border-box;
                    -moz-box-sizing: border-box;
                    width: 100%;
                    background: #fff;
                    margin-bottom: 4%;
                    border: 1px solid #ccc;
                    padding: 3%;
                    color: #555;
                    font: 95% Arial, Helvetica, sans-serif;
                }
                .form-style-6 input[type="text"]:focus,
                .form-style-6 input[type="date"]:focus,
                .form-style-6 input[type="datetime"]:focus,
                .form-style-6 input[type="email"]:focus,
                .form-style-6 input[type="number"]:focus,
                .form-style-6 input[type="search"]:focus,
                .form-style-6 input[type="time"]:focus,
                .form-style-6 input[type="url"]:focus,
                .form-style-6 textarea:focus,
                .form-style-6 select:focus
                {
                    box-shadow: 0 0 5px #43D1AF;
                    padding: 3%;
                    border: 1px solid #43D1AF;
                }

                .form-style-6 input[type="submit"],
                .form-style-6 input[type="button"]{
                    box-sizing: border-box;
                    -webkit-box-sizing: border-box;
                    -moz-box-sizing: border-box;
                    width: 100%;
                    padding: 3%;
                    background: #43D1AF;
                    border-bottom: 2px solid #30C29E;
                    border-top-style: none;
                    border-right-style: none;
                    border-left-style: none;
                    color: #fff;
                }
                .form-style-6 input[type="submit"]:hover,
                .form-style-6 input[type="button"]:hover{
                    background: #2EBC99;
                }
                </style>
        </style>
    </head>
    <body>
        <h3>树洞</h3>
"""

POST = """
<div align="center" class="form-style-6">
<form action="index" method="post" id="usrform">
<input type="text" name="author" placeholder="Who are you?">
</form>
<textarea placeholder="Say something..." rows="4" cols="50" name="comment" form="usrform"></textarea>
<input type="submit">
</div>
"""

DISPLAY = """
<table>
    <tr>
        <th>ID</th>
        <th>Author</th>
        <th>Posts</th>
        <th>Commit Time</th>
    </tr>
"""

def handle_input(connection, query):
    query = urllib.parse.unquote_plus(query)
    splitted = query.split("&")
    author = ""
    comment = ""

    for field in splitted:
        pair = field.split("=")
        if pair[0] == "author":
            author = pair[1]
        if pair[0] == "comment":
            comment = pair[1]

    value = "(author, post, commit_time) VALUES ('{}', '{}', CURRENT_TIMESTAMP)".format(
        author,
        comment
    )
    insert_entry(connection, TABLE_NAME, value)



def main():
    get_query = urllib.parse.unquote(os.environ.get('GET_QUERY'))
    post_query = urllib.parse.unquote(os.environ.get('POST_QUERY'))


    sys.stdout.write(HEADER_TEMPLATE.format(datetime.datetime.now().strftime("%a, %d %b %Y %H:%M:%S GMT")))
    sys.stdout.write(HTML)
    sys.stdout.write(POST)
    sys.stdout.write(DISPLAY)


    # Connect to database
    connection = establish_connection(SQL_CONFIG)
    connect_database(connection, DB_NAME)

    entry_exists(connection, TABLE_NAME, "id=0")

    # User input
    if get_query:
        handle_input(connection, get_query)
    if post_query:
        handle_input(connection, post_query)

    # Get Content
    cursor = connection.cursor()
    cursor.execute("SELECT * FROM `forum_posts`")

    for result in cursor:
        row = "<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>\n".format(
            result[0],
            result[1],
            result[2],
            result[3]
        )
        sys.stdout.write(row)
    sys.stdout.write("</table></body></html>")



    sys.stdout.write("\r\n\r\n")


if __name__ == "__main__":
    main()
