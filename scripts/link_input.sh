#! /usr/bin/env bash

cd "$(dirname ${BASH_SOURCE[0]})"
source preamble.sh
source setup_machine.sh

time=${1}
module use ../modulefiles > /dev/null 2>&1
module load run.${MACHINE} > /dev/null 2>&1

./combine_input.py ${time}
./link_input.py ${time}
