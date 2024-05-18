## Compilação
```
./build.sh
```
## Teste
```
./test.sh
```
## Uso
Para codificar:
```
./main < arquivo.txt
```
Gera um arquivo "output.huff" como resultado.

Para decodificar:
```
./main output.huff
```
## Formatação
Formatação dos arquivos gerados:
| Tamanho (em bytes) | Uso                                                                                                                 |
|--------------------|---------------------------------------------------------------------------------------------------------------------|
| 4                  | magic number. diz se arquivo foi comprimido com a nossa codificação                                                 |
| 4                  | tamanho da tabela de frequência (N)                                                                                 |
| 8                  | tamanho em bytes do arquivo original, descomprimido                                                                 |
| (1 + 4) * N        | tabela de frequência. char + int * tamanho da tabela de frequência, onde char é o caractere e int a frequência dele |
| variado            | bits comprimidos                                                                                                    |
