#!/usr/bin/env bash


./gen $1 $1 >in.gen.txt

./fft3 <in.gen.txt >out.fft3.txt 2> time.fft3.txt # &
echo "Time fft3: $(cat time.fft3.txt)"

./fft4 <in.gen.txt >out.fft4.txt 2> time.fft4.txt # &
echo "Time fft4: $(cat time.fft4.txt)"
diff -q out.fft3.txt  out.fft4.txt

./fft5 <in.gen.txt >out.fft5.txt 2> time.fft5.txt # &
echo "Time fft5: $(cat time.fft5.txt)"
diff -q out.fft4.txt  out.fft5.txt

# ./gen $1 $1 >in.gen.txt
#
# ./fft3 <in.gen.txt >out.fft3.txt 2> time.fft3.txt                   &
# pid_fft3=$!
#
# ./fft4 <in.gen.txt >out.fft4.txt 2> time.fft4.txt                   &
# pid_fft4=$!
#
# ./fft5 <in.gen.txt >out.fft5.txt 2> time.fft5.txt                   &
# pid_fft5=$!
#
# ./fft6 <in.gen.txt >out.fft6.txt 2> time.fft6.txt                   &
# pid_fft6=$!
#
# wait $pid_fft3
# echo "Time fft3: $(cat time.fft3.txt)"
#
# wait $pid_fft4
# echo "Time fft4: $(cat time.fft4.txt)"
#
# diff -q out.fft3.txt  out.fft4.txt
#
# wait $pid_fft5
# echo "Time fft5: $(cat time.fft5.txt)"
#
# diff -q out.fft4.txt  out.fft5.txt

