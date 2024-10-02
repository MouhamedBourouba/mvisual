#! /usr/bin/sh
CFLAGS="-Wall -Wextra -g"

clang $CFLAGS mvisual.c third-party/libraylib.a -lm
