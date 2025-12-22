/*
 * Fuzzer for libtiff strip and tile reading functions.
 * Targets: TIFFReadEncodedStrip, TIFFReadRawStrip, TIFFReadEncodedTile,
 *          TIFFReadRawTile, TIFFReadFromUserBuffer
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 1024 * 1024;  // 1MB max input
static const uint64_t kMaxAllocSize = 100 * 1024 * 1024;  // 100MB max alloc

extern "C" void handle_error(const char *, const char *, va_list) {}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0 || size > kMaxInputSize)
    {
        return 0;
    }

    TIFFSetErrorHandler(handle_error);
    TIFFSetWarningHandler(handle_error);

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
    setenv("JSIMD_FORCENONE", "1", 1);
#endif
#endif

    std::istringstream s(std::string(reinterpret_cast<const char *>(data), size));
    TIFF *tif = TIFFStreamOpen("MemTIFF", &s);
    if (!tif)
    {
        return 0;
    }

    do
    {
        // Get image dimensions
        uint32_t width = 0, height = 0;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        if (width == 0 || height == 0 ||
            (uint64_t)width * height > kMaxAllocSize)
        {
            continue;
        }

        if (TIFFIsTiled(tif))
        {
            // Handle tiled images
            uint32_t tileWidth = 0, tileLength = 0;
            TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth);
            TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileLength);

            if (tileWidth == 0 || tileLength == 0)
            {
                continue;
            }

            tmsize_t tileSize = TIFFTileSize(tif);
            if (tileSize <= 0 || (uint64_t)tileSize > kMaxAllocSize)
            {
                continue;
            }

            void *buf = _TIFFmalloc(tileSize);
            if (!buf)
            {
                continue;
            }

            uint32_t numTiles = TIFFNumberOfTiles(tif);
            // Read up to first 16 tiles to avoid excessive time
            uint32_t tilesToRead = numTiles < 16 ? numTiles : 16;

            for (uint32_t tile = 0; tile < tilesToRead; tile++)
            {
                // Test TIFFReadEncodedTile
                TIFFReadEncodedTile(tif, tile, buf, tileSize);

                // Test TIFFReadRawTile
                uint64_t rawSize64 = TIFFGetStrileByteCount(tif, tile);
                tmsize_t rawSize = (rawSize64 < kMaxAllocSize) ? (tmsize_t)rawSize64 : 0;
                if (rawSize > 0 && (uint64_t)rawSize <= kMaxAllocSize)
                {
                    void *rawBuf = _TIFFmalloc(rawSize);
                    if (rawBuf)
                    {
                        TIFFReadRawTile(tif, tile, rawBuf, rawSize);
                        _TIFFfree(rawBuf);
                    }
                }
            }

            _TIFFfree(buf);
        }
        else
        {
            // Handle strip-based images
            tmsize_t stripSize = TIFFStripSize(tif);
            if (stripSize <= 0 || (uint64_t)stripSize > kMaxAllocSize)
            {
                continue;
            }

            void *buf = _TIFFmalloc(stripSize);
            if (!buf)
            {
                continue;
            }

            uint32_t numStrips = TIFFNumberOfStrips(tif);
            // Read up to first 32 strips to avoid excessive time
            uint32_t stripsToRead = numStrips < 32 ? numStrips : 32;

            for (uint32_t strip = 0; strip < stripsToRead; strip++)
            {
                // Test TIFFReadEncodedStrip
                TIFFReadEncodedStrip(tif, strip, buf, stripSize);

                // Test TIFFReadRawStrip
                tmsize_t rawSize = TIFFRawStripSize(tif, strip);
                if (rawSize > 0 && (uint64_t)rawSize <= kMaxAllocSize)
                {
                    void *rawBuf = _TIFFmalloc(rawSize);
                    if (rawBuf)
                    {
                        TIFFReadRawStrip(tif, strip, rawBuf, rawSize);
                        _TIFFfree(rawBuf);
                    }
                }
            }

            _TIFFfree(buf);
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
