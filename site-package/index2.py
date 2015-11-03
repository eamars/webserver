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
    # Connect to server
    connection = mysql.connector.connect(**SQL_CONFIG)

    # Connect to database
    connection.database = DB_NAME

    # Create sql
    sql = "SELECT * FROM `{}`".format(TABLE_NAME)

    # Execute sql
    cursor = connection.cursor()
    cursor.execute(sql)

    sys.stdout.write(COL)
    for result in cursor:
        row = "<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>\n".format(
            result[0],
            result[1],
            result[2],
            result[3],
            result[4]
        )
        sys.stdout.write(row)
    sys.stdout.write("</table></body></html>")




if __name__ == "__main__":
    main()
