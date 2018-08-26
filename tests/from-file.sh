#!/usr/bin/env bash

../bin/mxstatd --file=../data/mtesrl_20150626_MD0000600002_stats.txt &
pid=$!
sleep 1
echo -n "ORDER" | nc -u -q1 localhost 9000

kill "$pid"

