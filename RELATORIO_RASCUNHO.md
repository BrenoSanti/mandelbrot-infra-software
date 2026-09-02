# 1. Objetivo

O projeto solicitado calcula o conjunto de Mandelbrot em uma versão serial e
três versões paralelas. Foram entregues implementações OpenMP e Pthreads com
duas divisões de trabalho, além da geração das imagens e do registro de tempos.
O projeto inclui compilação automatizada, testes e documentação de reprodução.

# 2. Checklist de Requisitos

| Requisito do enunciado | Status | Como testar / evidência |
|---|---|---|
| Cálculo serial | [CONFIRMAR] | `make test` e inspeção de `src/mandelbrot.c` |
| Paralelismo OpenMP com número solicitado de threads | [CONFIRMAR] | Inspeção do `parallel for` e execução do teste |
| Pthreads 1 com blocos contíguos | [CONFIRMAR] | Inspeção do particionamento e caso 23 × 17 |
| Pthreads 2 com linhas cíclicas | [CONFIRMAR] | Inspeção do particionamento e caso 23 × 17 |
| Mesma matemática e arquivos byte a byte iguais | [CONFIRMAR] | Comandos `cmp` de `tests/test.sh` |
| Arquivos sem cabeçalho PGM | [CONFIRMAR] | `head mandelbrot_bgs_serial.pgm` |
| Tempos referentes principalmente ao cálculo | [CONFIRMAR] | Inspeção dos limites de `clock_gettime` |
| Validação robusta de argumentos e overflow | [CONFIRMAR] | Casos inválidos de `tests/test.sh` |
| Execução normal sem stdout | [CONFIRMAR] | Captura e verificação feita por `tests/test.sh` |
| Compilação sem warnings com as flags exigidas | [CONFIRMAR] | `make clean && make` |
| Ausência de vazamentos/erros de memória | [CONFIRMAR] | Executar Valgrind conforme a seção 6 |

# 3. Como Reproduzir

No diretório raiz do projeto, em Linux/Ubuntu:

```sh
make clean
make
./mandelbrot 800 600 1000 4
make test
```

# 4. Arquitetura

- `src/main.c` — valida argumentos, aloca memória, mede os cálculos e grava as saídas.
- `src/mandelbrot.h` — declara a configuração e a interface das quatro versões.
- `src/mandelbrot.c` — contém a matemática compartilhada e as estratégias de execução.
- `tests/test.sh` — testa casos válidos, saídas idênticas e entradas inválidas.
- `Makefile` — automatiza compilação, limpeza e testes.
- `README.md` — documenta uso, dependências, estratégias e testes.
- `RELATORIO_RASCUNHO.md` — organiza o relatório a ser completado com evidências reais.
- `.gitignore` — exclui da versão os artefatos gerados.

Decisão → Buffer linear único reutilizado sequencialmente → Reduzir memória e
manter indexação uniforme → Cada resultado precisa ser gravado antes do próximo cálculo.

Decisão → Uma única função de cálculo de pixel → Garantir a mesma matemática nas
quatro versões → Diferenças ficam restritas à distribuição das linhas.

Decisão → Medir criação e sincronização das threads, mas não escrita → Representar
o custo real de cada estratégia computacional → Os tempos paralelos incluem seu overhead.

# 5. Estratégias e Diário de Desenvolvimento

## 5.1 Estratégias da semana

| Estratégia | Nome curto | Contexto | Motivo da troca (se houve) |
|---|---|---|---|
| Percurso sequencial por linhas | Serial | Referência determinística | Não houve troca registrada |
| Linhas com escalonamento estático | OpenMP | Paralelismo por diretiva | Não houve troca registrada |
| Blocos contíguos balanceados | Pthreads 1 | Divisão explícita | Não houve troca registrada |
| Linhas intercaladas | Pthreads 2 | Divisão explícita alternativa | Não houve troca registrada |

## 5.2 Diário de Tentativas

| # | Estrat. | O que tentei | Resultado | Hipótese/Causa (se falhou) | Quando | Evidência |
|---|---|---|---|---|---|---|
| 1 | [OBTER DO EVIDENCIAS.LOG] | [OBTER DO EVIDENCIAS.LOG] | [OBTER DO EVIDENCIAS.LOG] | [OBTER DO EVIDENCIAS.LOG] | [OBTER DO EVIDENCIAS.LOG] | [OBTER DO EVIDENCIAS.LOG] |

# 6. Evidências

No início da sessão manual de validação, executar:

```sh
script -a evidencias.log
date
whoami
pwd
```

Depois, executar os comandos da seção 3, os `cmp` e, se disponível:

```sh
valgrind --leak-check=full --error-exitcode=1 ./mandelbrot 23 17 100 4
```

Encerrar a captura com `exit` e incluir `evidencias.log` no arquivo `.tar` final.

[INSERIR PRINT REAL]

[INSERIR EVIDÊNCIA REAL]

[OBTER DO EVIDENCIAS.LOG]

# 7. Uso de IA

## Onde usei IA

Foi utilizada IA como auxílio na implementação, revisão, criação dos testes,
diagnóstico e documentação do projeto.

## Prompts principais

- Solicitação de implementação integral conforme o enunciado da disciplina.
- [ADICIONAR OUTROS PROMPTS REALMENTE UTILIZADOS, SE HOUVER]

## O que validei manualmente e como

[CONFIRMAR APÓS A SESSÃO MANUAL, CITANDO COMANDOS E EVIDÊNCIAS REAIS]

# 8. Reflexão Final

O projeto evidencia que a mesma operação pode receber distribuições de trabalho
distintas sem mudar o resultado. A função compartilhada de pixel reduz o risco
de divergência entre as versões. A divisão por blocos favorece acesso contíguo,
enquanto a divisão cíclica espalha linhas potencialmente mais custosas entre as
threads. O custo de criar e sincronizar threads também faz parte da comparação.
A validação de entradas evita que dimensões inválidas se transformem em acessos
incorretos à memória. Os resultados de desempenho ainda devem ser interpretados
com medições reais e repetidas na máquina usada na entrega.

# 9. Checklist Final de Entrega

- [CONFIRMAR] compilei do zero seguindo apenas o relatório
- [CONFIRMAR] rodei testes e `evidencias.log` foi gerado
- [CONFIRMAR] tenho prints obrigatórios
- [CONFIRMAR] testei caso limite
- [CONFIRMAR] testei caso inválido
- [CONFIRMAR] preenchi uso de IA
- [CONFIRMAR] removi frases genéricas

# 10. Se eu tivesse mais 2 horas

Eu repetiria cada medição várias vezes e registraria mediana e dispersão.
Compararia speedup e eficiência para diferentes quantidades de threads.
Executaria os testes em mais de uma versão do GCC no Ubuntu.
Acrescentaria um teste independente com pixels esperados calculados à mão.
Usaria sanitizadores de endereço e comportamento indefinido além do Valgrind.
Documentaria as características da CPU e a carga do sistema durante as medições.
Essas ações produziriam uma análise de desempenho mais reprodutível e verificável.
