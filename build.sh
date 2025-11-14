start=$(date +%s.%N)

mkdir -p build

CCOMPILER=("zig" "cc")
CFLAGS="-std=c99"
CPP_FLAGS="-std=gnu++17"
COMPILER=("g++")
CFLAGS_COMP="-pedantic -Wall -Wextra -Werror -Wno-initializer-overrides -Wno-override-init -Wno-cast-function-type"
CPP_FLAGS_COMP="-Wall -Wextra -Werror -fno-exceptions"
FLAGS_COMP="-Ifurry -Ifurry/deps -Itail/furry -Itail/furry/deps -Ideps -I. -Itail/deps -Itail -Ideps/imgui -Itail/deps/imgui -isystem"
FLAGS_LINK=""

BUILD_TEST=false
EXAMPLE=""

[[ "$OSTYPE" == "linux-gnu" ]] && BUILD_WINDOWS=false || BUILD_WINDOWS=true

for arg in "$@"; do
    if [ "$arg" = "-t" ]; then
        BUILD_TEST=true
    elif [ "$arg" = "-w" ]; then
        BUILD_WINDOWS=true
    else
        EXAMPLE="$arg"
    fi
done

if ! $BUILD_TEST && ! command -v zig &> /dev/null; then
    CCOMPILER=("gcc")
    if ! gcc --version &> /dev/null; then
        CCOMPILER=("clang")
        if ! clang --version &> /dev/null; then
            CCOMPILER=("cc")
            if ! cc --version &> /dev/null; then
                echo "WHAT FUCKING c COMPILER DO YOU HAVE?!?!"
            fi
        fi
    fi
    exit 1
elif $BUILD_TEST && ! tcc --version &> /dev/null; then
    CCOMPILER=("zig" "cc")
    if ! command -v zig &> /dev/null; then
        CCOMPILER=("gcc")
        if ! gcc --version &> /dev/null; then
            CCOMPILER=("clang")
            if ! clang --version &> /dev/null; then
                CCOMPILER=("cc")
                if ! cc --version &> /dev/null; then
                    echo "WHAT FUCKING c COMPILER DO YOU HAVE?!?!"
                fi
            fi
        fi
    fi
    exit 1
fi

# hacky
[[ ! "$OSTYPE" == "linux-gnu" ]] && CCOMPILER=("gcc")

if $BUILD_TEST; then
    FLAGS_COMP+=" -O0 -g -fno-sanitize=undefined"
    FLAGS_LINK+=" -g -fno-lto"
else
    FLAGS_COMP+=" -O3"
    FLAGS_LINK+=" -flto"
fi

if command -v lld &> /dev/null; then
    LINKER="-fuse-ld=lld"
elif command -v ld &> /dev/null; then
    LINKER=""
else
    echo "failed to find a linker!"
    exit 1
fi

if $BUILD_WINDOWS; then
    CCOMPILER=("x86_64-w64-mingw32-gcc")
    COMPILER=("x86_64-w64-mingw32-g++")
    FLAGS_LINK+=" -lopengl32 -Lfurry/deps/GLFW -Ltail/furry/deps/GLFW -lglfw3 -lgdi32 -D_WIN32"
    CFLAGS_COMP+=" -fno-sanitize=undefined -Wno-override-init-side-effects"
    LINKER=""
    if [[ "${COMPILER[0]}" == "zig" ]]; then
        CFLAGS_COMP+=" -target x86_64-windows"
    fi
else
    FLAGS_LINK+=" -lGL -lglfw -lEGL -lX11 -lm"
fi

if [ -n "$EXAMPLE" ]; then
    SRC_DIRS=("." "examples/$EXAMPLE")
else
    SRC_DIRS=("tail" "src")
fi

FILES_C=()
FILES_CPP=()
for dir in "${SRC_DIRS[@]}"; do
    while IFS= read -r file; do
        [[ -f "$file" ]] || continue
        case "${file##*.}" in
            c) FILES_C+=("$file") ;;
            cc) FILES_CPP+=("$file") ;;
            cpp) FILES_CPP+=("$file") ;;
        esac
    done < <(find "$dir" -type f ! -path "*/examples/*")
done

OUT="compile_commands.json"
echo "[" > "$OUT"

FIRST=1
for file in "${FILES_C[@]}" "${FILES_CPP[@]}"; do 
    if [[ $file == *.c ]]; then
        COMP="gcc" # not really
        STD="-std=c99"
        PEDANTIC="-pedantic"
    else
        COMP="g++"
        STD="-std=gnu++17"
        PEDANTIC=""
    fi

    CMD="$COMP $STD $PEDANTIC -Wall -Wextra -Werror -Wno-initializer-overrides -Ifurry -Ifurry/deps -Itail/furry -Itail/furry/deps -Ideps -I. -Itail/deps -Itail -c \\\"$file\\\""

    if [ $FIRST -eq 1 ]; then
        FIRST=0
    else
        echo "," >> "$OUT"
    fi

    echo -ne "\t{
        \"directory\": \"$(pwd)\",
        \"command\": \"$CMD\",
        \"file\": \"$(pwd)/${file#./}\"
    }" >> "$OUT"
done

echo -e "\n]" >> "$OUT"

echo "COMPILING: ${FILES_C[@]} ${FILES_CPP[@]}"

if command -v ccache >/dev/null 2>&1; then
    echo "ccache enabled!"
    COMPILER=("ccache" "${COMPILER[@]}")
    CCOMPILER=("ccache" "${CCOMPILER[@]}")
fi

OBJ_DIR="build/obj"
OBJS=()

#[[ $BUILD_TEST == false ]] && rm -rf build/obj/
rm -rf build/obj/
mkdir -p build/obj

max_jobs=$(nproc) 

compile_file() {
    local file="$1"
    local compiler=("${!2}")  # pass array name
    local flags="$3"
    
    #obj="$OBJ_DIR/$(basename "$file" ${file##*.}).o"
    #dep="$OBJ_DIR/$(basename "$file" .${file##*.}).d"
    #filename=$("$file")
    #basename_noext="${filename%.*}"
    cleanpath="${file#./}"
    obj="$OBJ_DIR/${cleanpath%.c}.o"
    mkdir -p "$(dirname "$obj")"
    #dep="$OBJ_DIR/$basename_noext.d"
    echo "$obj" >> "$OBJ_DIR/objs.tmp"
    #[[ -f "$obj" && "$obj" -nt "$file" ]] && return

    skip_compile=false

    if [[ $BUILD_TEST && -f "$obj" ]]; then
        # check if "deps" is anywhere in the path
        if [[ "$file" == */deps/* ]]; then
            skip_compile=true
        fi
    fi

    if [[ $skip_compile != true ]]; then
        if [[ $file == *.c ]]; then
            #"${COMPILER[@]}" $flags -fno-sanitize=undefined -c "$file" -o "$obj"
            "${CCOMPILER[@]}" $CFLAGS $CFLAGS_COMP $FLAGS_COMP \
                -fno-sanitize=undefined -c "$file" -o "$obj"
        else
            #"${COMPILER_CC[@]}" $flags -fno-sanitize=undefined -c "$file" -o "$obj"
            "${COMPILER[@]}" $CPP_FLAGS $CPP_FLAGS_COMP $FLAGS_COMP \
                -fno-sanitize=undefined -c "$file" -o "$obj"
        fi 
    fi
}

> "$OBJ_DIR/objs.tmp"  # clear temp file

for file in "${FILES_C[@]}" "${FILES_CPP[@]}"; do
    compiler_arr=$FILES_C[@]
    flags="$FLAGS_COMP"

    compile_file "$file" compiler_arr "$flags" &

    # limit parallel jobs
    while [ $(jobs -r | wc -l) -ge $max_jobs ]; do
        sleep 0.1
    done
done

wait

mapfile -t OBJS < "$OBJ_DIR/objs.tmp"

rm -f objs.tmp

if $BUILD_WINDOWS; then
    "${COMPILER[@]}" "${OBJS[@]}" $FLAGS_LINK -o build/out.exe -static
else
    "${COMPILER[@]}" "$LINKER" "${OBJS[@]}" $FLAGS_LINK -fno-sanitize=undefined -o build/out.game
fi

end=$(date +%s.%N)
elapsed=$(echo "$end - $start" | bc)
printf "TOOK %.3f SECONDS\n\n" "$elapsed"

if $BUILD_WINDOWS; then
    ( [[ "$OSTYPE" == "linux-gnu" ]] && wine ./build/out.exe || ./build/out.exe )
else
    ./build/out.game
fi
