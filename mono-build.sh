#!/bin/bash
set -e

# Usage: build-generic.sh <out_path> <src_dir1> [src_dir2 ...] [--linux|--windows] [--x86-64|--aarch64] [--debug|--release]
# Parameters can be specified in any order after out_path and at least one src_dir

if [[ $# -lt 2 ]]; then
	echo "Usage: $0 <out_path> <src_dir1> [src_dir2 ...] [--linux|--windows] [--x86-64|--aarch64] [--debug|--release]"
	echo ""
	echo "Parameters can be specified in any order."
	echo "Default: current platform + debug"
	echo ""
	exit 1
fi

out="$1"
shift

# Separate source directories from build options
build_args=();
src_dirs=();

for arg in "$@"; do
	case "$arg" in
		--linux|--windows|--x86-64|--x86_64|--aarch64|--arm64|--debug|--release|-h|--help)
			build_args+=("$arg")
			;;
		*)
			src_dirs+=("$arg")
			;;
	esac
done

if [[ ${#src_dirs[@]} -eq 0 ]]; then
	echo "Error: At least one source directory is required"
	exit 1
fi

src_find() {
	find "$@" -type d -name '_*' -prune -o -type f -name '*.c' ! -name '_*' -print
}

# Build include paths and source files from all source directories
use=("-I.")
src=()
for dir in "${src_dirs[@]}"; do
	if [[ ! -d "$dir" ]]; then
		echo "Error: Source directory '$dir' does not exist"
		exit 1
	fi
	use+=("-I${dir}")
	found_files=($(src_find "${dir}"))
	if [[ ${#found_files[@]} -gt 0 ]]; then
		src+=("${found_files[@]}")
	else
		echo "Warning: No .c files found in '$dir'"
	fi
done

if [[ ${#src[@]} -eq 0 ]]; then
	echo "Error: No source files found in specified directories"
	exit 1
fi

flags=(
	"-nostdlib"
	"-ffreestanding"

	"-Wall"
	"-Wextra"
	"-Wshadow"
	"-Wconversion"
	"-Wsign-conversion"
)

# Workaround for aarch64: using lld instead of lld-22 until llvm-22 is installed
# On x86-64, lld-22 is used
detect_linker() {
	local arch=$(uname -m 2>/dev/null || echo "unknown")
	
	case "$arch" in
		aarch64|arm64)
			echo "-fuse-ld=lld"
			;;
		x86_64|amd64)
			echo "-fuse-ld=lld-22"
			;;
		*)
			echo "-fuse-ld=lld"
			;;
	esac
}

linker_flag=$(detect_linker)
flags+=("$linker_flag")

# OS targets
linux_x86_64=(
	"-target" "x86_64-unknown-linux-gnu"
	"-Wl,-e,_start"
	"-fno-pie"
	"-no-pie"
	"-Wl,--no-dynamic-linker"
)

linux_aarch64=(
	"-target" "aarch64-unknown-linux-gnu"
	"-Wl,-e,_start"
	"-fno-pie"
	"-no-pie"
	"-Wl,--no-dynamic-linker"
)

windows_x86_64=(
	"-target" "x86_64-pc-windows-gnu"
	"-Wl,-e,_start" "-lkernel32"
)

windows_aarch64=(
	"-target" "aarch64-pc-windows-gnu"
	"-Wl,-e,_start" "-lkernel32"
)

# Build modes
debug=(
	"-O0"
	"-g"
	"-fno-delete-null-pointer-checks"
)

release_linux=(
	"-O3"
	"-flto=full"
)

release_windows=(
	"-O3"
)

# Detect current platform
detect_platform() {
	local arch=$(uname -m 2>/dev/null || echo "unknown")
	local os=$(uname -s 2>/dev/null || echo "unknown")
	
	case "$os" in
		Linux*)
			case "$arch" in
				x86_64|amd64) echo "linux x86_64" ;;
				aarch64|arm64) echo "linux aarch64" ;;
				*) echo "linux x86_64" ;;
			esac
			;;
		MINGW*|MSYS*|CYGWIN*)
			case "$arch" in
				x86_64|amd64) echo "windows x86_64" ;;
				aarch64|arm64) echo "windows aarch64" ;;
				*) echo "windows x86_64" ;;
			esac
			;;
		*)
			case "$arch" in
				x86_64|amd64) echo "linux x86_64" ;;
				aarch64|arm64) echo "linux aarch64" ;;
				*) echo "linux x86_64" ;;
			esac
			;;
	esac
}

# Parse build arguments in any order
os=""
arch=""
kind=""

for arg in "${build_args[@]}"; do
	case "$arg" in
		--linux)
			os="linux"
			;;
		--windows)
			os="windows"
			;;
		--x86-64|--x86_64)
			arch="x86_64"
			;;
		--aarch64|--arm64)
			arch="aarch64"
			;;
		--debug)
			kind="debug"
			;;
		--release)
			kind="release"
			;;
		-h|--help)
			echo "Usage: $0 <out_path> <src_dir1> [src_dir2 ...] [--linux|--windows] [--x86-64|--aarch64] [--debug|--release]"
			echo ""
			echo "Parameters can be specified in any order."
			echo "Default: current platform + debug"
			exit 0
			;;
		*)
			echo "Unknown argument: $arg"
			exit 1
			;;
	esac
done

# Set defaults
if [[ -z "$os" || -z "$arch" ]]; then
	defaults=$(detect_platform)
	default_os=$(echo "$defaults" | cut -d' ' -f1)
	default_arch=$(echo "$defaults" | cut -d' ' -f2)
	
	if [[ -z "$os" ]]; then
		os="$default_os"
	fi
	
	if [[ -z "$arch" ]]; then
		arch="$default_arch"
	fi
fi

if [[ -z "$kind" ]]; then
	kind="debug"
fi

# Select target flags using underscore format
case "${os}_${arch}" in
	linux_x86_64)
		target=("${linux_x86_64[@]}")
		;;
	linux_aarch64)
		target=("${linux_aarch64[@]}")
		;;
	windows_x86_64)
		target=("${windows_x86_64[@]}")
		;;
	windows_aarch64)
		target=("${windows_aarch64[@]}")
		;;
	*)
		echo "Unsupported combination: ${os} ${arch}"
		exit 1
		;;
esac

# Select mode flags
if [[ "$kind" == "debug" ]]; then
	mode=("${debug[@]}")
else
	if [[ "$os" == "linux" ]]; then
		mode=("${release_linux[@]}")
	else
		mode=("${release_windows[@]}")
	fi
fi

# Display architecture in user-friendly format (with hyphen)
display_arch="${arch}"
if [[ "$arch" == "x86_64" ]]; then
	display_arch="x86-64"
fi

# Build
echo "================================"
echo "[RUN]: ${os} ${display_arch} ${kind}"
echo "[SRC]: ${src_dirs[*]}"
echo "================================"
echo "[CMD]: clang ${flags[@]} ${target[@]} ${mode[@]} ${use[@]} ${src[@]} -o ${out}"
clang "${flags[@]}" "${target[@]}" "${mode[@]}" "${use[@]}" "${src[@]}" -o "${out}"
echo "================================"
echo "[DONE]: ${out}"
