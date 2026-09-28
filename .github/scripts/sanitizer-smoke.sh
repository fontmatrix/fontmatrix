#!/bin/sh
# Starts Fontmatrix from a build made with ECM_ENABLE_SANITIZERS and fails if
# AddressSanitizer or UBSan has anything to say.
#
#   sanitizer-smoke.sh <path to the fontmatrix binary> [seconds]
#
# A new, private profile is used: the start imports the system fonts, which
# exercises FreeType, HarfBuzz, the database and the list views. There is no
# display, the window never closes by itself, so the timeout is the normal end.
set -u

BIN="$1"
SECONDS_TO_RUN="${2:-120}"

PROFILE="$(mktemp -d)"
trap 'rm -rf "$PROFILE"' EXIT
mkdir -p "$PROFILE/data" "$PROFILE/config" "$PROFILE/cache"
LOG="$PROFILE/run.log"

status=0
env XDG_DATA_HOME="$PROFILE/data" XDG_CONFIG_HOME="$PROFILE/config" XDG_CACHE_HOME="$PROFILE/cache" \
    QT_QPA_PLATFORM=offscreen QT_FORCE_STDERR_LOGGING=1 \
    ASAN_OPTIONS=detect_leaks=0:halt_on_error=0 UBSAN_OPTIONS=print_stacktrace=1 \
    timeout -s TERM "$SECONDS_TO_RUN" dbus-run-session -- "$BIN" > "$LOG" 2>&1 || status=$?

echo "---- last lines of the log"
tail -n 25 "$LOG"
echo "----"

# 124 is timeout(1) ending the run. Anything else means it stopped by itself.
if [ "$status" -ne 124 ]; then
    echo "fontmatrix ended by itself, status $status"
    exit 1
fi

if grep -E "ERROR: AddressSanitizer|ERROR: LeakSanitizer|runtime error:" "$LOG"; then
    echo "the sanitizers reported the lines above"
    exit 1
fi

DB="$PROFILE/data/Fontmatrix/fontmatrix/Data.sql"
if [ ! -s "$DB" ]; then
    echo "no font database was written, the start did not get far enough"
    exit 1
fi

echo "clean: no sanitizer report in ${SECONDS_TO_RUN}s"
