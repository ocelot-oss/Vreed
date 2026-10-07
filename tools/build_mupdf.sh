#!/bin/sh
set -eu
mode=${1:-host}
root=.deps/mupdf
revision=8ad45e92f0935d3d87f1db3f873086472a5e1b24
if [ ! -f "$root/include/mupdf/fitz.h" ]; then
  git clone --depth 1 --branch 1.28.5 https://github.com/ArtifexSoftware/mupdf.git "$root"
fi
if [ -d "$root/.git" ]; then
  test "$(git -C "$root" rev-parse HEAD)" = "$revision"
  git -C "$root" submodule update --init --recursive --depth 1 thirdparty/freetype thirdparty/libjpeg thirdparty/jbig2dec thirdparty/openjpeg thirdparty/zlib thirdparty/lcms2
  printf '%s\n' "$revision" > "$root/VREED_REVISION"
else
  # The corresponding-source archive has no Git metadata and builds offline.
  test "$(cat "$root/VREED_REVISION")" = "$revision"
fi
if [ "$mode" = vita ]; then
  set -- CC=arm-vita-eabi-gcc CXX=arm-vita-eabi-g++ AR=arm-vita-eabi-ar RANLIB=arm-vita-eabi-ranlib LD=arm-vita-eabi-ld
else
  set -- CC=cc CXX=c++ AR=ar RANLIB=ranlib LD=ld
fi
make -C "$root" -j2 libs OS=generic build=release OUT=build/$mode "$@" \
  USE_SYSTEM_LIBS=no HAVE_PTHREAD=no HAVE_X11=no HAVE_GLUT=no HAVE_GLFW=no HAVE_OBJCOPY=no \
  tofu=yes tofu_cjk=yes mujs=no html=no xps=no svg=no extract=no brotli=no threading=no \
  XCFLAGS="-DFZ_ENABLE_ICC=0 -DFZ_ENABLE_CBZ=0 -DTOFU -DTOFU_CJK"
