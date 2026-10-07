#! /usr/bin/env bash

cd "$(dirname ${BASH_SOURCE[0]})"
source preamble.sh
source setup_machine.sh

time=${1:?}
module use ../modulefiles > /dev/null 2>&1
module load run.${MACHINE} > /dev/null 2>&1

year=${time:0:4}
month=${time:4:2}
PDY=${time:0:8}
cyc=${time:8:2}

cd "${TIGGE_PTMP}"
for location in "hpss/gefs.${PDY}/${cyc}" "${TIGGE_INPUT:-?}/${time}" "${TIGGE_OUTPUT:-?}/archive/${time}" "${TIGGE_OUTPUT:-?}/archive/tigge-kwbc-${time}.tar" "${TIGGE_OUTPUT:-?}/archive/tigge-kwbc-${time}.tar.sha256"; do
	if [[ -e "${location}" ]]; then
		rm -r "${location}"
	fi
done

# Delete any empty source directories
find "hpss/gefs.${PDY}" -type d -empty -delete || true

# Delete logs older than 30 days
#find "logs/" -mtime +30 -delete
