#ifndef ERROR_H_
#define ERROR_H_


#define handle_error(msg) \
        do { perror(msg); fprintf(stderr, "FILE: [%s],  LINE: [%d]\n", __FILE__, __LINE__); exit(EXIT_FAILURE); } while (0)

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); fprintf(stderr, "FILE: [%s],  LINE: [%d]\n", __FILE__, __LINE__); exit(EXIT_FAILURE); } while (0)

#endif
