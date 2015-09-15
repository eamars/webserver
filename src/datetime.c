#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "datetime.h"

int get_datetime(char *buffer)
{
    time_t current_time;
    struct tm *ptm;

    // obtain current as second elapsed since the epoch
    time(&current_time);

    // get time in GMT
    ptm = gmtime(&current_time);
    if (ptm == NULL)
    {
        return -1;
    }

    // Tue, 15 Sep 2015 06:09:14 GMT
    strftime (buffer, MAX_DATETIME_LENGTH, "%a, %d %b %G %T GMT", ptm);

    return 0;
}
