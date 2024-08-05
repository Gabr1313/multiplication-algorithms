#!/usr/bin/env bash

./gen $1 $1 >in.gen.txt

./fft.bk <in.gen.txt >out.fft.bk.txt 2> time.fft.bk.txt              &
pid_fft_bk=$!

./fft <in.gen.txt >out.fft.txt 2> time.fft.txt                       &
pid_fft=$!

if [[ $1 -le 10000000 ]]; then
    ./karatsuba <in.gen.txt >out.karatsuba.txt 2> time.karatsuba.txt &
    pid_karatsuba=$!
fi

if [[ $1 -le 2000000 ]]; then
    ./naif      <in.gen.txt >out.naif.txt      2> time.naif.txt      &
    pid_naif=$!
fi

wait $pid_fft_bk
echo "Time fft.bk:    $(cat time.fft.bk.txt)"

wait $pid_fft
echo "Time fft:       $(cat time.fft.txt)"

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

