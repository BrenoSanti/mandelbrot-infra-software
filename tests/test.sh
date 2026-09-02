#!/bin/sh
set -eu

PROGRAM=./mandelbrot
OUTPUTS="mandelbrot_bgs_serial.pgm mandelbrot_bgs_openmp.pgm mandelbrot_bgs_pthreads1.pgm mandelbrot_bgs_pthreads2.pgm"

fail() {
    echo "FALHA: $1" >&2
    exit 1
}

expect_failure() {
    if "$PROGRAM" "$@" >/dev/null 2>/dev/null; then
        fail "argumentos invalidos foram aceitos: $*"
    fi
}

check_run() {
    stdout_file=$(mktemp)
    if ! "$PROGRAM" "$@" >"$stdout_file"; then
        rm -f "$stdout_file"
        fail "execucao valida falhou: $*"
    fi
    if [ -s "$stdout_file" ]; then
        rm -f "$stdout_file"
        fail "execucao valida produziu stdout: $*"
    fi
    rm -f "$stdout_file"

    for output in $OUTPUTS; do
        [ -f "$output" ] || fail "$output nao foi criado"
    done
    cmp mandelbrot_bgs_serial.pgm mandelbrot_bgs_openmp.pgm
    cmp mandelbrot_bgs_serial.pgm mandelbrot_bgs_pthreads1.pgm
    cmp mandelbrot_bgs_serial.pgm mandelbrot_bgs_pthreads2.pgm
    [ -f times.txt ] || fail "times.txt nao foi criado"
    [ "$(wc -l < times.txt)" -eq 4 ] || fail "times.txt nao contem quatro linhas"
    awk 'NF != 2 || $1 !~ /^(serial|openmp|pthreads1|pthreads2)$/ || $2 !~ /^[0-9]+([.][0-9]+)?$/ { exit 1 }' times.txt ||
        fail "formato invalido em times.txt"
}

check_run 20 20 100 2
check_run 23 17 100 4
check_run 20 20 100 1

expect_failure
expect_failure 20 20 100
expect_failure 20 20 100 2 extra
expect_failure 0 20 100 2
expect_failure -1 20 100 2
expect_failure 20 0 100 2
expect_failure 20 20 0 2
expect_failure 20 20 100 0
expect_failure texto 20 100 2
expect_failure 20xyz 20 100 2
expect_failure 999999999999999999999999 20 100 2

echo "Todos os testes passaram."
