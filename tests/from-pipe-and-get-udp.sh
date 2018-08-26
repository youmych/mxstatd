#!/usr/bin/env bash

pipename=./mxstatd.pipe

rm -f ./mxstatd.pipe

../bin/mxstatd --pipe "$pipename" &
PID=$!

cat ../data/mtesrl_20150626_MD0000600002_stats.txt > "$pipename" &

for i in $(seq 1 5); do
    echo -n "ORDER" | nc -u4 -q1 localhost 9000
done

kill "$PID"

