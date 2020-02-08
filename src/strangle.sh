#!/bin/sh

if echo $1 | grep -Eq '^[0-9]+(\.[0-9]+)?$'; then
	FPS="$1"
	shift
fi

STRANGLE_LIB_NAME="libstrangle.so"
if [ "$STRANGLE_NODLSYM" = "1" ]; then
	STRANGLE_LIB_NAME="libstrangle_nodlsym.so"
fi

if [ "$#" -eq 0 ]; then
	programname=`basename "$0"`
	echo "ERROR: No program supplied"
	echo
	echo "Usage: $programname [fps-limit] <program-to-strangle>"
	echo
	echo "  fps-limit"
	echo "        a number describing the desired maximum frame rate"
	echo "        If missing: the environment variable \"STRANGLE_FPS\" will be used."
	echo "        If neither is set no limit will be applied"
	exit 1
fi

# Execute the strangled program under a clean environment
# pass through the FPS and overriden LD_PRELOAD environment variables
if [ "$STRANGLE_VKONLY" != "1" ]; then
	LD_PRELOAD="${LD_PRELOAD}:${STRANGLE_LIB_NAME}"
fi

exec env ENABLE_VK_LAYER_TORKEL104_libstrangle=1 STRANGLE_FPS="${FPS}" LD_PRELOAD="${LD_PRELOAD}" "$@"
