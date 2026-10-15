#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define emitError(module, msg) fprintf(stderr, "\033[38;5;160m[ERROR] %s: %s\033[0m\n", module, msg)
#define emitWarning(module, msg) fprintf(stderr, "\033[38;5;214m[WARN] %s: %s\033[0m\n", module, msg)

#endif
