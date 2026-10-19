#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void emitError(const char *module, const char *fmt, ...);
void emitWarning(const char *module, const char *fmt, ...);

#endif
