#!/bin/bash

PROCESS_NAME=$1
INTERVAL=0.3

if [ -z "$PROCESS_NAME" ]; then
    echo "Usage: $0 <process_name>"
    exit 1
fi

# Ensure we clean up the cursor and screen if the user hits Ctrl+C on the script itself
trap "echo; exit" SIGINT SIGTERM

while true; do
    # Find the PID
    PID=$(pgrep -o "$PROCESS_NAME")

    if [ -n "$PID" ]; then
        clear
        echo ">>> Monitoring: $PROCESS_NAME (PID: $PID) @ ${INTERVAL}s"
        echo "------------------------------------------------------------"
        
        # We use a loop here instead of 'watch' to give us total control 
        # over the recovery and "Waiting" state transitions.
        while kill -0 "$PID" 2>/dev/null; do
            # Move cursor to row 3, column 1 to overwrite stats only
            tput cup 2 0
            
            # Show CPU (-u), Mem (-r), and IO (-d) for PID and threads (-t)
            # '1 1' tells pidstat to take 1 sample over 1 second, 
            # but we use the INTERVAL for the loop speed.
            pidstat -p "$PID" -urd -t 1 1 | tail -n +3
            
            sleep "$INTERVAL"
        done

        clear
        echo "[!] Process $PROCESS_NAME ($PID) lost. Searching..."
    else
        # Use \r and tput to ensure the "Waiting" line stays at the top
        tput cup 0 0
        printf "\r\033[KWaiting for '%s' to appear... " "$PROCESS_NAME"
    fi

    sleep 1
done
