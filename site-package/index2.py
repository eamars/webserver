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
"\r\n\r\n"

# Connection configuration for MySQL Connection
SQL_CONFIG = {
    "host": "192.168.2.5",
    "user": "eamars",
    "password": "931105",
    "autocommit": True
}

DB_NAME = "JAV"
TABLE_NAME = "GENRE_INDEX_TABLE"


COL = """
        <html>
            <body>
                <table border="1" bgcolor="D8D8D8 ">
                    <tr>
                        <th>GENRE_ID</th>
                        <th>GENRE_NAME_EN</th>
                        <th>GENRE_NAME_CN</th>
                        <th>GENRE_NAME_TW</th>
                        <th>GENRE_NAME_JA</th>
                    </tr>
        """

def main():
    HTML = ""

    # Connect to server
    connection = mysql.connector.connect(**SQL_CONFIG)

    # Connect to database
    connection.database = DB_NAME

    # Create sql
    sql = "SELECT * FROM `{}`".format(TABLE_NAME)

    # Execute sql
    cursor = connection.cursor()
    cursor.execute(sql)

    HTML += COL
    for result in cursor:
        row = "<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>\n".format(
            result[0],
            result[1],
            result[2],
            result[3],
            result[4]
        )
        HTML += row
    HTML += "</table></body></html>"

    HEADER = HEADER_TEMPLATE.format(datetime.datetime.now().strftime("%a, %d-%b-%Y %H:%M:%S GMT"))


    sys.stdout.write(HEADER + HTML)




if __name__ == "__main__":
    main()
