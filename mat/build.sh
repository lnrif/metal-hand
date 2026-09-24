#!/bin/bash

mkdir -p "mat/build/"

MONO_BUILD="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/mono-build.sh"

if [[ ! -f "$MONO_BUILD" ]]; then
	echo "Error: mono-build.sh not found at $MONO_BUILD"
	exit 1
fi

bash "$MONO_BUILD" "mat/build/mat" "mat/src" "std" "linux" "windows" "$@"

