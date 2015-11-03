#!/usr/bin/env python3
import sys
import os
import mysql.connector

import datetime

HEADER_TEMPLATE = \
"HTTP/1.1 200 OK\r\n" \
"Server: webhttpd/2.0\r\n" \
"Cache-Control: no-cache, no-store, must-revalidate\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: text/html; charset=utf-8\r\n" \
"Date: {}\r\n" \

# Connection configuration for MySQL Connection
SQL_CONFIG = {
    "host": "192.168.2.5",
    "user": "eamars",
    "password": "931105",
    "autocommit": True
}


COL = """
        <html>
            <body>
                <table border="1" bgcolor="D8D8D8">
                    <tr>
        """

def main():
    sys.stdout.write(HEADER_TEMPLATE.format(datetime.datetime.now().strftime("%a, %d-%b-%Y %H:%M:%S GMT")))

    # Connect to server
    connection = mysql.connector.connect(**SQL_CONFIG)

    # Connect to database
    connection.database = "`ZUOBIAO.ME`"

    # Write first part of html
    sys.stdout.write(COL)

    # Get col
    sql = "SELECT * FROM ZUOBIAO_COL"
    cursor = connection.cursor()
    cursor.execute(sql)

    for result in cursor:
        col = "<th>{}</th>".format(result[1])
        sys.stdout.write(col)
    sys.stdout.write("</tr>")

    sql = "SELECT * FROM ZUOBIAO where ID < 40"
    cursor = connection.cursor()
    cursor.execute(sql)

    for result in cursor:
        sys.stdout.write("<tr>")
        for row in result:
            sys.stdout.write("<td width=200>{}</td>".format(row))
        sys.stdout.write("</tr>\n")
        sys.stdout.flush()
    sys.stdout.write("</table></body></html>")


if __name__ == "__main__":
    main()
