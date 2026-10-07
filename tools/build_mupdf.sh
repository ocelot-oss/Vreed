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
  sdkinclude="$VITASDK/arm-vita-eabi/include"
  extra="-Dtimegm=vreed_timegm -include $(pwd)/src/platform/mupdf_compat.h"
  set -- CC=arm-vita-eabi-gcc CXX=arm-vita-eabi-g++ AR=arm-vita-eabi-ar RANLIB=arm-vita-eabi-ranlib LD=arm-vita-eabi-ld \
    USE_SYSTEM_FREETYPE=yes "SYS_FREETYPE_CFLAGS=-I$sdkinclude/freetype2" SYS_FREETYPE_LIBS=-lfreetype \
    USE_SYSTEM_LIBJPEG=yes "SYS_LIBJPEG_CFLAGS=-I$sdkinclude" SYS_LIBJPEG_LIBS=-ljpeg \
    USE_SYSTEM_ZLIB=yes "SYS_ZLIB_CFLAGS=-I$sdkinclude" SYS_ZLIB_LIBS=-lz
else
  extra=""
  set -- CC=cc CXX=c++ AR=ar RANLIB=ranlib LD=ld
fi
make -C "$root" -j2 libs OS=generic build=release OUT=build/$mode "$@" \
  USE_SYSTEM_LIBS=no HAVE_PTHREAD=no HAVE_X11=no HAVE_GLUT=no HAVE_GLFW=no HAVE_OBJCOPY=no \
  tofu=yes tofu_cjk=yes mujs=no html=no xps=no svg=no extract=no brotli=no threading=no \
  XCFLAGS="-DFZ_ENABLE_ICC=0 -DFZ_ENABLE_CBZ=0 -DTOFU -DTOFU_CJK $extra"
