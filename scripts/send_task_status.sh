#!/bin/bash
# send_task_status.sh

# Environment variables expected:
# FCST_TIME   - forecast initialization time
# SCRIPT_NAME - name of the script that finished
# EXIT_CODE   - exit code of that script

echo "Sending status..."
echo "Forecast time: $FCST_TIME"
echo "Script: $SCRIPT_NAME"
echo "Main Log: $MAIN_LOG"
echo "Exit code: $EXIT_CODE"

# Set recipient email
EMAIL="eric.sinsky@noaa.gov"

# Build subject and message
if [[ "${EXIT_CODE}" -eq 0 ]]; then
    STATUS="SUCCESS"
else
    STATUS="FAILURE"
fi

SUBJECT="[TIGGE] ${STATUS} - ${SCRIPT_NAME} (${FCST_TIME})"
BODY="
The script ${SCRIPT_NAME} (${SCRIPT_FULL_PATH}) for forecast ${FCST_TIME} has finished.
The main log is ${MAIN_LOG}
Exit code: ${EXIT_CODE}
Status: ${STATUS}

End time: $(date -u)"

# Send email
echo "${BODY}" | mail -s "${SUBJECT}" "${EMAIL}"
