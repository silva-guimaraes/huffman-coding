## Compilação
```
./build.sh
```
## Teste
```
./test.sh
```
## Visualização
Requer uma ferramenta de visualização chamada [graphviz](https://graphviz.org/) como dependência e que seja definido o seguinte macro diretamente no código:
```c
#define GRAPH 1
```
Após isso, gere a árvore da codificação com:
```
./graph.sh arquivo.txt
```
Gera uma imagem "output.png" do seu diretório atual.
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
