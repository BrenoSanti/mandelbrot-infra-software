# Mandelbrot: versões serial, OpenMP e Pthreads

Projeto em C que calcula o conjunto de Mandelbrot na região real `[-2, 1]` e
imaginária `[-1.5, 1.5]`. A mesma imagem é produzida por uma versão serial e
por três versões paralelas, permitindo comparar seus tempos de cálculo.

## Dependências

- Linux/Ubuntu
- GCC com suporte a OpenMP
- GNU Make
- implementação POSIX de Pthreads
- utilitários de shell `sh`, `cmp`, `awk`, `wc` e `mktemp` para os testes

Em Ubuntu, GCC e Make podem ser instalados pelo pacote `build-essential`.

## Compilação

```sh
make
```

O executável gerado se chama `mandelbrot`. Para remover executável, objetos e
saídas geradas, use `make clean`.

## Execução

```sh
./mandelbrot largura altura max_iteracoes num_threads
./mandelbrot 800 600 1000 4
```

Todos os quatro argumentos devem ser inteiros positivos. `largura` e `altura`
definem a quantidade de pixels, `max_iteracoes` limita o cálculo de escape e
`num_threads` controla OpenMP e Pthreads. Em caso de erro, a mensagem é enviada
para stderr; uma execução bem-sucedida não escreve em stdout.

## Arquivos gerados

- `mandelbrot_bgs_serial.pgm`
- `mandelbrot_bgs_openmp.pgm`
- `mandelbrot_bgs_pthreads1.pgm`
- `mandelbrot_bgs_pthreads2.pgm`
- `times.txt`

Apesar da extensão `.pgm`, os arquivos de imagem não possuem cabeçalho: contêm
somente intensidades de 0 a 255 separadas por espaços, uma linha da imagem por
linha. As quatro imagens devem ser byte a byte idênticas. `times.txt` registra,
em segundos, apenas o período de cálculo de cada implementação (incluindo a
gestão das threads, mas não a escrita dos arquivos).

## Estratégias

- **Serial:** percorre todas as linhas e colunas sequencialmente.
- **OpenMP:** distribui as linhas com um `parallel for` de escalonamento estático
  e usa exatamente a quantidade de threads solicitada.
- **Pthreads 1:** reparte blocos contíguos de linhas; as primeiras threads
  recebem uma linha adicional quando a altura não é divisível igualmente.
- **Pthreads 2:** reparte linhas ciclicamente; a thread `i` calcula as linhas
  `i`, `i + num_threads`, `i + 2 * num_threads` e assim por diante.

Todas chamam as mesmas funções internas de cálculo de pixel e linha. Como cada
thread escreve em linhas exclusivas de um buffer linear, não há escrita
concorrente na mesma posição.

## Testes

```sh
make test
```

O script executa casos regulares, divisão não exata e uma única thread; compara
as quatro imagens com `cmp`; confere `times.txt` e ausência de stdout; e rejeita
argumentos ausentes, extras, não numéricos, parciais, não positivos e com
overflow de conversão.
