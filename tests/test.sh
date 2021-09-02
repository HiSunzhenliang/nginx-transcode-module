#!/bin/bash

sourcetypes=("pcm" "alaw" "ulaw" "amrnb" "amrwb" "mpeg")
outtypes=(wav,mp3)

sourcedir=/tmp/tmp_transcode_root
outdir=outdir
tmplist=tmp_list.txt
sourcefile=addf8-Alaw-GW.wav
host=127.0.0.1:8080

echo "Go tests:"

rm -rf $sourcedir $outdir $tmp_list $sourcefile

# make test audios
echo "1. make test audios"
wget -q http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Samples/Goldwave/addf8-Alaw-GW.wav
if [ $? -ne 0 ]; then
    echo "Get source file failed"
    exit -1
fi

mkdir -p $sourcedir
if [ $? -ne 0 ]; then
    echo "Mkdir source file dir failed"
    exit -1
fi

for i in ${sourcetypes[@]}; do
    if [ "$i" == "pcm" ]; then
        sox $sourcefile -e signed-integer $sourcedir/$i.wav
    elif [ "$i" == "alaw" ]; then
        sox $sourcefile -e a-law $sourcedir/$i.wav
    elif [ "$i" == "ulaw" ]; then
        sox $sourcefile -e u-law $sourcedir/$i.wav
    elif [ "$i" == "amrnb" ]; then
        sox $sourcefile -t amr-nb $sourcedir/$i.amr-nb
    elif [ "$i" == "amrwb" ]; then
        sox $sourcefile -r 16k -t amr-wb $sourcedir/$i.amr-wb
    elif [ "$i" == "mpeg" ]; then
        sox $sourcefile $sourcedir/$i.mp3
    fi
done

# make test list file
# needs combinations of formats
echo "2. make test list file"
ls $sourcedir > $tmplist


# check output dir
rm -rf $outdir
mkdir $outdir
if [ $? -ne 0 ]; then
    echo "mkdir $outdir failed, exit"
    exit -1
fi

# run test
for audio in $(cat $tmplist); do
    wget -q http://$host/$audio
    if [ $? -ne 0 ]; then
        echo "failed: $audio"
        rm $audio
        continue
    fi

    err=$(soxi $audio 2>&1)
    if [ $? -ne 0 ]; then
        echo "failed: $audio ($err)"
        rm $audio
        continue
    fi
    echo "succss: $audio"
    rm $audio
done

rm -rf $sourcedir $outdir $tmplist $sourcefile
echo "Tests done"
