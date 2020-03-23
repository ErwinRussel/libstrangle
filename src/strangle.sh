#!/bin/sh

if echo $1 | grep -Pq '^\d+(\.\d+)?$'; then
	export STRANGLE_FPS="$1"
	shift
elif echo $1 | grep -Pq '^\d+(\.\d+)?:\d+(\.\d+)?$'; then
	export STRANGLE_FPS="$(echo $1 | cut -d':' -f1)"
	export STRANGLE_FPS_BATTERY="$(echo $1 | cut -d':' -f2)"
	shift
fi

STRANGLE_LIB_NAME="libstrangle.so"
if [ "$STRANGLE_NODLSYM" = "1" ]; then
	STRANGLE_LIB_NAME="libstrangle_nodlsym.so"
fi

if [ -n "$STRANGLE_AF" ]; then
	# AMD
	export AMD_TEX_ANISO="$STRANGLE_AF"
	export R600_TEX_ANISO="$STRANGLE_AF"
	export RADV_TEX_ANISO="$STRANGLE_AF"

	# Nvidia
	# http://us.download.nvidia.com/XFree86/Linux-x86_64/440.64/README/openglenvvariables.html
	export __GL_LOG_MAX_ANISO=$(echo $STRANGLE_AF | awk '{printf "%d", log($1)/log(2)}')
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

exec env ENABLE_VK_LAYER_TORKEL104_libstrangle=1 LD_PRELOAD="${LD_PRELOAD}" "$@"
