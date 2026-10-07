#! /usr/bin/env bash

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)
cd "${script_dir}"
source preamble.sh
source setup_machine.sh

module use ../modulefiles > /dev/null 2>&1
module load run.${MACHINE} > /dev/null 2>&1
module load rocoto.${MACHINE} > /dev/null 2>&1

time=${1}
day=${time:0:8}
cyc=${time:8:2}

workflow_dir="$(dirname ${script_dir})/workflow"
mail_recipients="Eric.Sinsky@noaa.gov"

mail -s "A TIGGE job for ${day}_${cyc} is dead" "${mail_recipients}" <<- EOF
	A TIGGE job for ${day}_${cyc} has failed:

	rocotostat:
	$(rocotostat -d ${workflow_dir}/hpss.db -w ${workflow_dir}/hpss.xml -c ${time}00)
EOF
