#!/bin/sh

run_get_pitch $@ > /dev/null && pitch_evaluate pitch_db/train/*.f0ref | tail -n 3 | head -n 1 | cut -f3 | cut -d " " -f1