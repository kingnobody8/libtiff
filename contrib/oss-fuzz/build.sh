#!/bin/bash -eu
# Copyright (c) 1988-1997 Sam Leffler
# Copyright (c) 1991-1997 Silicon Graphics, Inc.
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

# Build zlib
pushd "$SRC/zlib"
./configure --static --prefix="$WORK"
make -j$(nproc) CFLAGS="$CFLAGS -fPIC"
make install
popd

# Build libjpeg-turbo
pushd "$SRC/libjpeg-turbo"
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DENABLE_STATIC=on -DENABLE_SHARED=off
make -j$(nproc)
make install
popd

# Build libjbig
pushd "$SRC/jbigkit"
if [ "$ARCHITECTURE" = "i386" ]; then
    echo "#!/bin/bash" > gcc
    echo "clang -m32 \$*" >> gcc
    chmod +x gcc
    PATH=$PWD:$PATH make lib
else
    make lib
fi

mv "$SRC"/jbigkit/libjbig/*.a "$WORK/lib/"
mv "$SRC"/jbigkit/libjbig/*.h "$WORK/include/"
popd

# Build liblzma from source for MSan compatibility
if [ "$SANITIZER" = "memory" ]; then
    pushd "$SRC"
    if [ ! -d "xz" ]; then
        git clone --depth 1 https://github.com/tukaani-project/xz.git
    fi
    cd xz
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=$WORK \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_C_FLAGS="${CFLAGS}" \
        -DBUILD_SHARED_LIBS=OFF
    make -j$(nproc)
    make install
    popd
    LZMA_FLAGS="-DLIBLZMA_LIBRARY=$WORK/lib/liblzma.a -DLIBLZMA_INCLUDE_DIR=$WORK/include"
else
    if [ "$ARCHITECTURE" != "i386" ]; then
        apt-get install -y liblzma-dev
    fi
    LZMA_FLAGS=""
fi

# Build libtiff
# Disable optional codecs that require uninstrumented libraries for MSan
if [ "$SANITIZER" = "memory" ]; then
    CMAKE_EXTRA_FLAGS="-Dzstd=OFF -Dwebp=OFF -Dlerc=OFF -Dlibdeflate=OFF ${LZMA_FLAGS}"
else
    CMAKE_EXTRA_FLAGS=""
fi

cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DBUILD_SHARED_LIBS=off ${CMAKE_EXTRA_FLAGS}
make -j$(nproc)
make install

# List of fuzzers to build
FUZZERS=(
    "tiff_read_rgba_fuzzer"
    "tiff_read_strip_fuzzer"
    "tiff_read_dir_fuzzer"
    "tiff_print_dir_fuzzer"
    "tiff_color_fuzzer"
    "tiff_write_read_fuzzer"
    "tiff_codec_fuzzer"
    "tiff_bigtiff_fuzzer"
    "tiff_multipage_fuzzer"
    "tiff_custom_tag_fuzzer"
    "tiff_ojpeg_fuzzer"
    "tiff_predictor_fuzzer"
    "tiff_open_options_fuzzer"
)

# Build each fuzzer
for fuzzer in "${FUZZERS[@]}"; do
    # Base libraries (all built from source for MSan compatibility)
    FUZZER_LIBS="$WORK/lib/libtiffxx.a $WORK/lib/libtiff.a $WORK/lib/libz.a $WORK/lib/libjpeg.a $WORK/lib/libjbig.a $WORK/lib/libjbig85.a"

    if [ "$SANITIZER" = "memory" ]; then
        # MSan build - only use libraries built from source
        FUZZER_LIBS="$FUZZER_LIBS $WORK/lib/liblzma.a"
        EXTRA_LINK=""
    elif [ "$ARCHITECTURE" = "i386" ]; then
        EXTRA_LINK=""
    else
        EXTRA_LINK="-Wl,-Bstatic -llzma -Wl,-Bdynamic"
    fi

    $CXX $CXXFLAGS -std=c++11 -I$WORK/include \
        $SRC/libtiff/contrib/oss-fuzz/${fuzzer}.cc -o $OUT/${fuzzer} \
        $LIB_FUZZING_ENGINE $FUZZER_LIBS $EXTRA_LINK

    # Copy dictionary for each fuzzer
    cp "$SRC/libtiff/contrib/oss-fuzz/tiff.dict" "$OUT/${fuzzer}.dict"

    # Copy options file if it exists
    if [ -f "$SRC/libtiff/contrib/oss-fuzz/${fuzzer}.options" ]; then
        cp "$SRC/libtiff/contrib/oss-fuzz/${fuzzer}.options" "$OUT/"
    fi
done

# Build seed corpus from test images and AFL testcases
mkdir -p afl_testcases
(cd afl_testcases; tar xf "$SRC/afl_testcases.tgz" 2>/dev/null || true)
mkdir -p tif_corpus
find afl_testcases -type f -name '*.tif' -exec mv -n {} tif_corpus/ \; 2>/dev/null || true

# Add libtiff test images to corpus
if [ -d "$SRC/libtiff/contrib/oss-fuzz/corpus" ]; then
    cp "$SRC/libtiff/contrib/oss-fuzz/corpus"/*.tiff tif_corpus/ 2>/dev/null || true
fi

# Also include test images from the source tree
if [ -d "$SRC/libtiff/test/images" ]; then
    cp "$SRC/libtiff/test/images"/*.tiff tif_corpus/ 2>/dev/null || true
fi

# Create corpus zip
zip -rj tif_corpus.zip tif_corpus/

# Copy corpus for each fuzzer
for fuzzer in "${FUZZERS[@]}"; do
    cp tif_corpus.zip "$OUT/${fuzzer}_seed_corpus.zip"
done

echo "Build complete! Built ${#FUZZERS[@]} fuzzers."
