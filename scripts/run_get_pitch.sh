#!/bin/bash

# Put here the program (maybe with path)
GETF0="get_pitch"

for fwav in pitch_db/train/*.wav; do
    ff0=${fwav/.wav/.f0}
    echo "$GETF0 $fwav $ff0 ----"
    $GETF0 $@ $fwav $ff0 > /dev/null || (echo -e "Error in $GETF0 $fwav $ff0\n"; exit 1)
done

exit 0
