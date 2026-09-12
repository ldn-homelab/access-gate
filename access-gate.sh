# access-gate.sh — grant or revoke access to a service
# usage:
#    - ./access-gate.sh grant|revoke <ip> <service>
#    - ./access-gate.sh check

#!/bin/sh
set -eu


# ----- connect to DB -----

# get DB data/ path
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
. "$SCRIPT_DIR/.env"
DB_PATH="$PATH_ACCESS_GATE_DATA"

# check if DB exists
if [ ! -f "$DB_PATH/access.db" ]; then
    echo "Error: no database at $DB_PATH/access.db... has Access-Gate ever been started?" >&2
    exit 1
fi


# ----- define Command -----

# define parameters
ACTION="${1:-}"
IP="${2:-}"
SERVICE="${3:-}"

# check if valid command
if [ "$ACTION" == "check" ]; then
    echo "Checking DB ..."
elif [ "$ACTION" != "grant" ] && [ "$ACTION" != "revoke" ]; then
    echo "Usage:"
    echo "    - $0 grant|revoke <ip> <service>"
    echo "    - $0 check" >&2
    exit 1
elif [ -z "$IP" ] || [ -z "$SERVICE" ]; then
    echo "Usage: $0 grant|revoke <ip> <service>" >&2
    exit 1
fi


# ----- run Command -----

# define SQL query
if [ "$ACTION" == "check" ]; then
    SQL="PRAGMA busy_timeout=2000; SELECT * FROM Access ORDER BY ip;"
elif [ "$ACTION" = "grant" ]; then
    SQL="PRAGMA busy_timeout=2000; INSERT OR IGNORE INTO Access (ip, service) VALUES ('$IP', '$SERVICE');"
else
    SQL="PRAGMA busy_timeout=2000; DELETE FROM Access WHERE ip='$IP' AND service='$SERVICE';"
fi

# run Job
podman run --rm -v "$DB_PATH:/data:z" alpine:3.20 \
    sh -c "apk add --no-cache sqlite >/dev/null && sqlite3 /data/access.db \"$SQL\""
if [ "$ACTION" != "check" ]; then
    echo "$ACTION: $IP -> $SERVICE"
fi