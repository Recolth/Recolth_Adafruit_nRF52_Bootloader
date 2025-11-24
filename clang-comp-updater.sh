#!/bin/bash
gcc_inc=$(arm-none-eabi-gcc -print-file-name=include)
gcc_fix=$(dirname "$gcc_inc")/include-fixed
sysroot=$(dirname "$(arm-none-eabi-gcc -print-sysroot)")/include

cat >.clangd <<EOF
CompileFlags:
  Add: [
    "--target=arm-none-eabi",
    "-isystem", "$gcc_inc",
    "-isystem", "$gcc_fix",
    "-isystem", "$sysroot"
  ]
EOF
