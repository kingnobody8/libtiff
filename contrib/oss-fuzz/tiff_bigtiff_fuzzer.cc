/*
 * Fuzzer for BigTIFF support.
 * Specifically targets BigTIFF (64-bit offsets) parsing and handling.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiff.h>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 1024 * 1024;  // 1MB max
static const uint64_t kMaxAllocSize = 50 * 1024 * 1024;  // 50MB max

extern "C" void handle_error(const char *, const char *, va_list) {}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 16 || size > kMaxInputSize)
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

    // Check if this is a BigTIFF
    int isBigTiff = TIFFIsBigTIFF(tif);
    (void)isBigTiff;

    int dirCount = 0;
    const int maxDirs = 50;

    do
    {
        if (++dirCount > maxDirs)
        {
            break;
        }

        uint32_t width = 0, height = 0;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        if (width == 0 || height == 0 || width > 50000 || height > 50000)
        {
            continue;
        }

        // Test 64-bit offset functions
        uint32_t numStriles = TIFFIsTiled(tif) ? TIFFNumberOfTiles(tif)
                                                : TIFFNumberOfStrips(tif);

        for (uint32_t i = 0; i < numStriles && i < 100; i++)
        {
            // These are the 64-bit offset functions
            uint64_t offset = TIFFGetStrileOffset(tif, i);
            uint64_t bytecount = TIFFGetStrileByteCount(tif, i);

            int err = 0;
            (void)TIFFGetStrileOffsetWithErr(tif, i, &err);
            (void)TIFFGetStrileByteCountWithErr(tif, i, &err);

            (void)offset;
            (void)bytecount;
        }

        // Test 64-bit size functions
        (void)TIFFScanlineSize64(tif);
        (void)TIFFRasterScanlineSize64(tif);
        (void)TIFFStripSize64(tif);
        (void)TIFFVStripSize64(tif, 1);
        (void)TIFFTileRowSize64(tif);
        (void)TIFFTileSize64(tif);
        (void)TIFFVTileSize64(tif, 1);

        // Test raw strip/tile size with 64-bit return
        for (uint32_t i = 0; i < numStriles && i < 10; i++)
        {
            (void)TIFFRawStripSize64(tif, i);
        }

        // Test directory offset (64-bit in BigTIFF)
        uint64_t dirOffset = TIFFCurrentDirOffset(tif);
        (void)dirOffset;

        // Try reading some data
        if (TIFFIsTiled(tif))
        {
            tmsize_t tileSize = TIFFTileSize(tif);
            if (tileSize > 0 && (uint64_t)tileSize < kMaxAllocSize)
            {
                void *buf = _TIFFmalloc(tileSize);
                if (buf)
                {
                    for (uint32_t t = 0; t < numStriles && t < 4; t++)
                    {
                        TIFFReadEncodedTile(tif, t, buf, tileSize);
                    }
                    _TIFFfree(buf);
                }
            }
        }
        else
        {
            tmsize_t stripSize = TIFFStripSize(tif);
            if (stripSize > 0 && (uint64_t)stripSize < kMaxAllocSize)
            {
                void *buf = _TIFFmalloc(stripSize);
                if (buf)
                {
                    for (uint32_t s = 0; s < numStriles && s < 8; s++)
                    {
                        TIFFReadEncodedStrip(tif, s, buf, stripSize);
                    }
                    _TIFFfree(buf);
                }
            }
        }

        // Test SubIFD navigation (uses 64-bit offsets in BigTIFF)
        uint16_t subIfdCount = 0;
        uint64_t *subIfdOffsets = NULL;
        if (TIFFGetField(tif, TIFFTAG_SUBIFD, &subIfdCount, &subIfdOffsets))
        {
            for (uint16_t i = 0; i < subIfdCount && i < 5; i++)
            {
                tdir_t curDir = TIFFCurrentDirectory(tif);
                if (TIFFSetSubDirectory(tif, subIfdOffsets[i]))
                {
                    // Read something from SubIFD
                    uint32_t subWidth = 0;
                    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &subWidth);
                }
                TIFFSetDirectory(tif, curDir);
            }
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
