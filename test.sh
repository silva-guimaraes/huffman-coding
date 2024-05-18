#!/usr/bin/sh

set -e

./build.sh

echo 'biblia versão king james: "biblia"'
./main < ./input/biblia.txt
./main ./output.huff > foobar
diff ./input/biblia.txt foobar

printf '\n'
echo 'documentação do ffmpeg: "teste.txt"'
./main < ./input/teste.txt
./main ./output.huff > foobar
diff ./input/teste.txt foobar

printf '\n'
echo 'a.txt: "a.txt.txt"'
./main < ./input/a.txt
./main ./output.huff > foobar
diff ./input/a.txt foobar

printf '\n'
echo 'rance X: "rance10_script.txt"'
./main < ./input/rance10_script.txt
./main ./output.huff > foobar
diff ./input/rance10_script.txt foobar
