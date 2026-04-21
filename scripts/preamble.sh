#! /usr/bin/env bash

#######
# Preamble script to be SOURCED at the beginning of every script. Sets 
#   useful PS4 and optionally turns on set -x and set -eu. Also sets up 
#   crude script timing and provides a postamble that runs on exit.
#
# Syntax:
#   preamble.sh [id]
#   
#   Aruguments:
#     id: Optional identifier string. Use when running the same script 
#           multiple times in the same job (e.g. MPMD)
#
# Input environment variables:
#   TRACE (YES/NO): Whether to echo every command (set -x) [default: "YES"]
#   STRICT (YES/NO): Whether to exit immediately on error or undefined variable
#     (set -eu) [default: "YES"]
#
#######
set +x
if [[ -v '1' ]]; then
	id="(${1})"
else
	id=""
fi

# export forecast time
export FCST_TIME=${1:-"unknown"}

# Record the start time so we can calculate the elapsed time later
start_time=$(date +%s)

# Get the base name of the calling script
_calling_script=$(basename ${BASH_SOURCE[1]})

# Get the absolute path of the calling script
export script_full_path=$(realpath "${BASH_SOURCE[1]}")

# Announce the script has begun
echo "Begin ${_calling_script} at $(date -u)"

# Stage our variables
export STRICT=${STRICT:-"YES"}
export TRACE=${TRACE:-"YES"}
export ERR_EXIT_ON=""
export TRACE_ON=""

if [[ $STRICT == "YES" ]]; then
	# Exit on error and undefined variable
	export ERR_EXIT_ON="set -eu"
fi
if [[ $TRACE == "YES" ]]; then
 	export TRACE_ON="set -x"
	# Print the script name and line number of each command as it is executed
	export PS4='+ $(basename $BASH_SOURCE)[$LINENO]'"$id: "
fi

postamble() {
	#
	# Commands to execute when a script ends. 
	#
	# Syntax:
	#   postamble script start_time rc
	#
	#   Arguments:
	#     script: name of the script ending
	#     start_time: start time of script (in seconds)
	#     rc: the exit code of the script
	#

	set +x
	script=${1}
	start_time=${2}
	rc=${3}

	# Calculate the elapsed time
	end_time=$(date +%s)
	elapsed_sec=$((end_time - start_time))
	elapsed=$(date -d@${elapsed_sec} -u +%H:%M:%S)

        if [[ -n "${SLURM_JOB_ID:-}" && "${rc:-0}" -ne 0 ]]; then
            # Check that sbatch and scontrol exist
            if ! command -v sbatch >/dev/null 2>&1 || ! command -v scontrol >/dev/null 2>&1; then
                echo "Warning: sbatch and/or scontrol not found; skipping job status email task"
            else
                # Query Slurm for the full stdout log path
                main_log=$(scontrol show job $SLURM_JOB_ID | awk -F= '/StdOut/ {print $2}')
                LOG_DIR=$(dirname "$main_log")
                mkdir -p "${LOG_DIR}"
                WORK_DIR=$(scontrol show job "$SLURM_JOB_ID" | awk -F= '/WorkDir/ {print $2}')

                BASE_LOG="${LOG_DIR}/send_job_status_for_${script}.out"
                LOG_FILE=$(get_next_log_file "$BASE_LOG")
                SCRIPTS_DIR="$(dirname "$WORK_DIR")/scripts"
                SEND_SCRIPT="${SCRIPTS_DIR}/send_task_status.sh"

                sbatch \
                  --export=ALL,FCST_TIME="${FCST_TIME}",SCRIPT_NAME="${script}",EXIT_CODE="${rc}",SCRIPT_FULL_PATH="${script_full_path}",MAIN_LOG="${main_log}" \
                  -o "${LOG_FILE}" \
                  -A fv3-cpu \
                  -J "status_${script}" \
                  -t 1:00:00 \
                  -p u1-service \
                  -n 1 \
                  "${SEND_SCRIPT}"
            fi
        fi

	# Announce the script has ended, then pass the error code up
	echo "End ${script} at $(date -u) with error code ${rc:-0} (time elapsed: ${elapsed})"


	exit ${rc}
}

# Returns the next available log file path (adds numeric suffix if needed)
get_next_log_file() {
    local base_file="$1"  # full path to the base log file
    local next_file="$base_file"

    if [[ -e "$base_file" ]]; then
        local i=1
        while [[ -e "${base_file}.${i}" ]]; do
            ((i++))
        done
        next_file="${base_file}.${i}"
    fi

    echo "$next_file"
}

# Place the postamble in a trap so it is always called no matter how the script exits
trap "postamble ${_calling_script} ${start_time} \$?" EXIT

# Turn on our settings
$ERR_EXIT_ON
$TRACE_ON
