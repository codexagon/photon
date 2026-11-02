#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void emitError(const char *type, const char *fmt, ...);
void emitWarning(const char *type, const char *fmt, ...);
void emitInfo(const char *type, const char *fmt, ...);

#endif
