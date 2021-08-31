#!/bin/bash
# ./test.sh host listfile outputdir

if [ $# -ne 3 ]; then
    echo "Params error"
    echo "Example: ./test.sh 127.0.0.1:8000 test_audios.txt out"
    exit
else
    echo "Go tests:"
fi

host=$1
listfile=$2
outdir=$3

if [ -d "$outdir" ]; then
    echo "$outdir exists, exit"
    exit
else
    mkdir $outdir
    if [ $? -ne 0 ]; then
        echo "mkdir $outdir failed, exit"
        exit
    fi
fi

for audio in $(cat $listfile); do
    cd $outdir
    wget -q http://$host/$audio
    if [ $? -ne 0 ]; then
        echo "failed: $audio"
        cd - > /dev/null
        continue
    fi

    cd - > /dev/null
    err=`soxi $outdir/$audio 2>&1`
    if [ $? -ne 0 ]; then
        echo "failed: $audio ($err)"
        continue
    fi
    echo "succss: $audio"
done

echo "Tests done"
