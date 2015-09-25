/**
 * Write and read the current pid of process to file
 *
 * Version 1.1/webhttpd
 */

#ifndef PID_LOCK_H
#define PID_LOCK_H

/**
 * Write process id to routerX.pid file
 * @param  pid process id
 * @return     negative if failed, 0 for succeed
 */
int write_pid(char *path, int pid);

/**
 * Read process from routerX.pid file
 * @return     negative if failed, positive for any process id
 */
int read_pid(char *path);

/**
 * Remove the routerX.pid file
 * @param rid router id
 */
void remove_pid(char *path);

#endif
