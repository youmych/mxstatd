#!/usr/bin/env bash

../bin/mxstatd --socket 8000 &
PID=$!

cat ../data/mtesrl_20150626_MD0000600002_stats.txt | nc -q1 localhost 8000 &

for i in $(seq 1 5); do
    kill -USR1 "$PID"
    sleep 1
done

kill "$PID"

