#!/usr/bin/sh

set -e


./build.sh

test () {
    echo '=================='
    echo $1
    time ./main < $2
    printf '\ndecodificação:\n'
    time ./main ./output.huff > foobar
    echo
    wc -l $2
    wc -c $2
    du -h --apparent-size $2
    du -h --apparent-size ./output.huff
    du -h --apparent-size foobar
    diff $2 foobar
    echo
}

test 'huffman_wiki.txt' ./input/huffman_wiki.txt

test 'documentação do ffmpeg: "ffmpeg_doc.txt"' ./input/ffmpeg_doc.txt

test 'biblia versão king james: "biblia"' ./input/biblia.txt


test 'a.txt: "a.txt"' ./input/a.txt

test 'rance X: "rance10_script.txt"' ./input/rance10_script.txt
