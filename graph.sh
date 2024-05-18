#!/usr/bin/sh

./build.sh \
    && ./main < $1 > graph.dot \
    && dot -Tpng graph.dot > output.png
