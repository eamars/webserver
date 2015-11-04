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

DB_NAME = "Syslog"
TABLE_NAME = "SystemEvents"


HTML = """
<html>
    <head>
        <title>Localhost</title>
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
                text-transform:uppercase;
                text-align:center;
                letter-spacing:-2px;
                font-size:2.5em;
                margin:20px 0;
            }
    </style>
    </head>
    <body>
        <h3>%d log entries from 192.168.2.5</h1>
        <table>
            <tr>
                <th>ID</th>
                <th>CustomerID</th>
                <th>ReceivedAt</th>
                <th>DeviceReportedTime</th>
                <th>Facility</th>
                <th>Priority</th>
                <th>FromHost</th>
                <th>Message</th>
                <th>NTSeverity</th>
                <th>Importance</th>
                <th>EventSource</th>
                <th>EventUser</th>
                <th>EventCategory</th>
                <th>EventID</th>
                <th>EventBinaryData</th>
                <th>MaxAvailable</th>
                <th>CurrUsage</th>
                <th>MinUsage</th>
                <th>MaxUsage</th>
                <th>InfoUnitID</th>
                <th>SysLogTag</th>
                <th>EventLogType</th>
                <th>GenericFileName</th>
                <th>SystemID</th>
            </tr>
"""

def main():
    sys.stdout.write(HEADER_TEMPLATE.format(datetime.datetime.now().strftime("%a, %d %b %Y %H:%M:%S GMT")))

    # Connect to server
    connection = mysql.connector.connect(**SQL_CONFIG)

    # Connect to database
    connection.database = DB_NAME

    # Get mysql cursor
    cursor = connection.cursor()

    sql = "SELECT COUNT(*) FROM `{}`".format(TABLE_NAME)
    cursor.execute(sql)

    for result in cursor:
        sys.stdout.write(HTML % result[0]) # old style substituding numbers into string
        break

    sql = "SELECT * FROM `{}`".format(TABLE_NAME)
    cursor.execute(sql)

    for result in cursor:
        sys.stdout.write("<tr>")
        for row in result:
            sys.stdout.write("<td>{}</td>".format(row))
        sys.stdout.write("</tr>\n")
    sys.stdout.write("</table></body></html>")


if __name__ == "__main__":
    main()
