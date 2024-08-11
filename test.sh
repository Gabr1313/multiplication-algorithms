#!/usr/bin/env bash

./gen $1 $1 >in.gen.txt

./fft       <in.gen.txt >out.fft.txt 2> time.fft.txt &
pid_fft=$!

./fft.mt <in.gen.txt >out.fft.mt.txt 2> time.fft.mt.txt  &
pid_fft_mt=$!

./fft.simd <in.gen.txt >out.fft.simd.txt 2> time.fft.simd.txt  &
pid_fft_simd=$!

if [[ $1 -le 10000000 ]]; then
    ./karatsuba <in.gen.txt >out.karatsuba.txt 2> time.karatsuba.txt &
    pid_karatsuba=$!
fi

if [[ $1 -le 2000000 ]]; then
    ./naif      <in.gen.txt >out.naif.txt      2> time.naif.txt      &
    pid_naif=$!
fi

wait $pid_fft
echo "Time fft:       $(cat time.fft.txt)"

wait $pid_fft_mt
echo "Time fft.mt:    $(cat time.fft.mt.txt)"
    diff -q out.fft.txt  out.fft.mt.txt

wait $pid_fft_simd
echo "Time fft.simd:  $(cat time.fft.simd.txt)"
    diff -q out.fft.txt  out.fft.simd.txt

if [[ $1 -le 10000000 ]]; then
    wait $pid_karatsuba
    echo "Time karatsuba: $(cat time.karatsuba.txt)"

    diff -q out.fft.txt  out.karatsuba.txt
fi

if [[ $1 -le 2000000 ]]; then
    wait $pid_naif
    echo "Time naif:      $(cat time.naif.txt     )"

    diff -q out.naif.txt out.fft.txt
    diff -q out.naif.txt out.karatsuba.txt
fi

