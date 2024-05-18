#!/usr/bin/sh

gcc main.c -lm -o main -DGRAPH \
    && ./main < $1 > graph.dot \
    && dot -Tpng graph.dot > output.png
