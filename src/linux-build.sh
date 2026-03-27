#!/bin/bash

gcc -o ../build/generate -g generate.c -lm && \
        ../build/generate lex.h ast.h && \
        gcc -o ../build/tst -g main.c -lm
