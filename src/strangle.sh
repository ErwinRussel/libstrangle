#!/bin/sh

if echo $1 | grep -Eq '^[0-9]+(\.[0-9]+)?$'; then
	FPS="$1"
	shift
fi

if [ "$TOPT" = "1" ]; then
	LD_PRELOAD="libpthread.so.0:${LD_PRELOAD}"
	export __GL_THREADED_OPTIMIZATIONS=1
fi


if [ "$#" -eq 0 ]; then
	programname=`basename "$0"`
	echo "ERROR: No program supplied"
	echo
	echo "Usage: $programname [fps-limit] <program-to-strangle>"
	echo
	echo "  fps-limit"
	echo "        a number describing the desired maximum frame rate"
	echo "        If missing: the environment variable \"FPS\" will be used."
	echo "        If neither is set no limit will be applied"
	exit 1
fi

# Execute the strangled program under a clean environment
# pass through the FPS and overriden LD_PRELOAD environment variables
exec env FPS="${FPS}" LD_PRELOAD="${LD_PRELOAD}:libstrangle.so" "$@"
