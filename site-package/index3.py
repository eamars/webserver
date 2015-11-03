#!/usr/bin/env python3
import sys
import os
import mysql.connector

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

    sql = "SELECT * FROM ZUOBIAO where ID < 10"
    cursor = connection.cursor()
    cursor.execute(sql)

    for result in cursor:
        sys.stdout.write("<tr>")
        for row in result:
            sys.stdout.write("<td width=200>{}</td>".format(row))
        sys.stdout.write("</tr>\n")
    sys.stdout.write("</table></body></html>")


if __name__ == "__main__":
    main()
