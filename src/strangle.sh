#!/bin/sh

programName=$(basename "$0")
STRANGLE_LIB_NAME="libstrangle.so"
STRANGLE_LIB_NAME_NO_DLSYM="libstrangle_nodlsym.so"

print_usage_info() {
	echo "Usage: ${programName} [OPTION]... [FPS] PROGRAM"
	echo "  or:  ${programName} [OPTION]... [FPS]:[BATTERY_FPS] PROGRAM"
	echo "Limit the framerate of PROGRAM to FPS"
	echo
	echo "Mandatory arguments to long options are mandatory for short options too."
	echo "  -a, --aniso AF           sets the anisotropic filtering level. Makes textures"
	echo "                             sharper at an angle. Limited OpenGL support"
	echo "  -b, --battery-fps FPS    alternative framerate cap for when running on"
	echo "  -c, --bicubic            force bicubic filtering. Vulkan only"
	echo "                             battery power. See --fps"
	echo "  -f, --fps FPS            limit the framerate to FPS."
	echo "                             A value of 0 will disable the limiter."
	echo "  -k, --vulkan-only        prevent the OpenGL libs from loading"
	echo "  -n, --no-dlsym           disable dlsym hijacking in the OpenGL library"
	echo "  -p, --picmip PICMIP      set the mipmap LoD bias to PICMIP. A higher value"
	echo "                             means blurrier textures. Can be negative"
	echo "  -t, --trilinear          force trilinear filtering. Vulkan only"
	echo "  -e, --no-error           force error-free context. OpenGL only"
	echo "  -v, --vsync VSYNC        Force vertical synchronisation on or off."
	echo "                             For OpenGL the following rules apply:"
	echo "                               0 - Force off"
	echo "                               1 - Force on"
	echo "                               n - Sync to refresh rate / n"
	echo "                             For Vulkan the following rules apply:"
	echo "                               0 - Force off"
	echo "                               1 - Mailbox mode. Uncapped framerate"
	echo "                               2 - Traditional vsync. Capped framerate"
	echo "                               3 - Adaptive vsync. tearing at low framerates"
	echo
	echo "Short options may not be combined into one string."
	echo "Options will override environment variables."
	echo
	echo "Some programs will crash, freeze or behave unexpectedly when dlsym is hijacked"
	echo "If a program is exhibiting these symptoms when run with strangle try using the"
	echo "'--no-dlsym' option. Or if the program uses the Vulkan API you can disable"
	echo "strangle's OpenGL libs altogether with the '--vulkan-only' option."
	echo
	echo "Strangle home page: <https://gitlab.com/torkel104/libstrangle>"
}

while [ $# -gt 0 ]; do
	if echo $1 | grep -Pq '^\d+(\.\d+)?$'; then
		export STRANGLE_FPS="$1"
		shift
	elif echo $1 | grep -Pq '^\d+(\.\d+)?:\d+(\.\d+)?$'; then
		export STRANGLE_FPS="$(echo $1 | cut -d':' -f1)"
		export STRANGLE_FPS_BATTERY="$(echo $1 | cut -d':' -f2)"
		shift
	else
		case "$1" in
			-a|--aniso)
				export STRANGLE_AF="$2"
				shift
				shift
				;;
			-b|--battery-fps)
				export STRANGLE_FPS_BATTERY="$2"
				shift
				shift
				;;
			-c|--bicubic)
				export STRANGLE_BICUBIC="1"
				shift
				;;
			-f|--fps)
				export STRANGLE_FPS="$2"
				shift
				shift
				;;
			-h|--help)
				print_usage_info
				exit 0
				;;
			-k|--vulkan-only)
				STRANGLE_VKONLY="1"
				shift
				;;
			-n|--no-dlsym)
				STRANGLE_NODLSYM="1"
				shift
				;;
			-p|--picmip)
				export STRANGLE_PICMIP="$2"
				shift
				shift
				;;
			-t|--trilinear)
				export STRANGLE_TRILINEAR="1"
				shift
				;;
			-e|--no-error)
				export STRANGLE_NO_ERROR="1"
				shift
				;;
			-v|--vsync)
				export STRANGLE_VSYNC="$2"
				shift
				shift
				;;
			--)
				shift
				break 2
				;;
			*)
				break 2
				;;
		esac
	fi
done

if [ "$#" -eq 0 ]; then
	echo "${programName}: no program supplied"
	echo "Try '${programName} --help' for more information"
	exit 1
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

if [ "$STRANGLE_VKONLY" != "1" ]; then
	if [ "$STRANGLE_NODLSYM" = "1" ]; then
		LD_PRELOAD="${LD_PRELOAD}:${STRANGLE_LIB_NAME_NO_DLSYM}"
	else
		LD_PRELOAD="${LD_PRELOAD}:${STRANGLE_LIB_NAME}"
	fi
fi

# Execute the strangled program under a clean environment
# pass through the FPS and overriden LD_PRELOAD environment variables
exec env ENABLE_VK_LAYER_TORKEL104_libstrangle=1 LD_PRELOAD="${LD_PRELOAD}" "$@"
