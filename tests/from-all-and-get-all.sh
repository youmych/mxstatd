#!/usr/bin/env bash

pipe=./mxstatd.pipe
infile=../data/mtesrl_20150626_MD0000600002_stats.txt
outfile=./out.txt


rm -f "$pipe"

../bin/mxstatd --pipe "$pipe" --socket 8000 --file "$infile" --outfile "$outfile" &
PID=$!

if [ -p "$pipe" ]; then
    cat "$infile" > "$pipe" &
fi
cat "$infile" | nc -t4 -q1 localhost 8000 &

for i in $(seq 1 5); do
    kill -USR1 "$PID"
    echo -n "ORDER" | nc -u4 -q1 localhost 9000
done

sleep 5

kill "$PID"

