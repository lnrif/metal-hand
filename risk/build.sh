#!/bin/bash
set -e

mkdir -p "risk/build/"
out="risk/build/risk"

src_find() {
	find "$@" -type d -name '_*' -prune -o -type f -name '*.c' ! -name '_*' -print
}

use=("-I.")
# use=($(find "src" -name "*.h" | grep -v '/_[^/]*\.h$'))
# src=($(find "src/" -name "*.c" | grep -v '/_[^/]*\.c$'))
src=($(src_find "linux" "windows" "risk" "std"))

flags=(
	"-nostdlib"
	"-ffreestanding"
	# "-fno-builtin"

	"-Wall"
	"-Wextra"
	"-Wshadow"
	"-Wconversion"
	"-Wsign-conversion"
	"-fuse-ld=lld"
)

linux=(
	"-target"
	"x86_64-unknown-linux-gnu"
	# "aarch64-unknown-linux-gnu"
	"-Wl,-e,_start"
	"-fno-pie"
	"-no-pie"
	"-Wl,--no-dynamic-linker"
)

windows=(
	"-target" "x86_64-pc-windows-gnu"
	"-Wl,-e,_start" "-lkernel32"
	# "-v"
)

debug=(
	"-O0"
	"-g"
	"-fno-delete-null-pointer-checks"
)

linux_release=(
	"-O3"
	# "-g"
	"-flto=full"
)

windows_release=(
	"-O3"
	# "-g"
)

help() {
	echo "Usage: $0 [linux|windows] [debug|release]"
	echo ""
	echo "default: linux debug"
	echo ""
}

os="${1:-linux}"
kind="${2:-debug}"

case "$os" in
	linux)
		case "$kind" in
			debug)
				echo "================================"
				echo "[RUN]: linux debug"
				echo "================================"
				echo -e "[CMD]: clang \n| [flags]: ${flags[@]}\n| [target]: ${linux[@]} \n| [mode]: ${debug[@]}\n| [use]: ${use[@]} ${src[@]}\n| [exe]: -o ${out}"
				clang "${flags[@]}" "${linux[@]}" "${debug[@]}" "${use[@]}" "${src[@]}" -o "${out}"
				echo "================================"
				;;

			release)
				echo "================================"
				echo "[RUN]: linux release"
				echo "================================"
				echo "[CMD]: clang ${flags[@]} ${linux[@]} ${linux_release[@]} ${use[@]} ${src[@]} -o ${out}"
				clang "${flags[@]}" "${linux[@]}" "${linux_release[@]}" "${use[@]}" "${src[@]}" -o "${out}"
				echo "================================"
				;;
			*) help ;;
		esac
		;;

	windows)
		case "$kind" in
			debug)
				echo "================================"
				echo "[RUN]: windows debug"
				echo "================================"
				echo "[CMD]: clang ${flags[@]} ${windows[@]} ${debug[@]} ${use[@]} ${src[@]} -o ${out}"
				clang "${flags[@]}" "${windows[@]}" "${debug[@]}" "${use[@]}" "${src[@]}" -o "${out}"
				echo "================================"
				;;

			release)
				echo "================================"
				echo "[RUN]: windows release"
				echo "================================"
				echo "[CMD]: clang ${flags[@]} ${windows[@]} ${windows_release[@]} ${use[@]} ${src[@]} -o ${out}"
				clang "${flags[@]}" "${windows[@]}" "${windows_release[@]}" "${use[@]}" "${src[@]}" -o "${out}"
				echo "================================"
				;;
			*) help ;;
		esac
		;;

	*) help ;;
esac

