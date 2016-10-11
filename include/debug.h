#ifndef DEBUG_H
#define DEBUG_H

#if ENABLE_DEBUG
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

#endif /* DEBUG_H */
