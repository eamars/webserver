#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_DATETIME_LENGTH 36

int get_datetime(char *buffer)
{
    time_t current_time;
    struct tm *ptm;

    // obtain current as second elapsed since the epoch
    time(&current_time);

    // get time in GMT
    ptm = gmtime(&current_time);

    strftime (buffer, MAX_DATETIME_LENGTH, "%a, %d %b %G %T GMT", ptm);

    return 0;
}


int main(void)
{
    char buffer[MAX_DATETIME_LENGTH];
    get_datetime(buffer);
    return 0;
}
