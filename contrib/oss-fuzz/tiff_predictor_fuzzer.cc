/*
 * Fuzzer for TIFF predictor handling.
 * Targets tif_predict.c which applies to LZW, Deflate, and other codecs.
 * Tests horizontal, floating-point, and other predictor modes.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiff.h>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 1024 * 1024;
static const uint64_t kMaxAllocSize = 50 * 1024 * 1024;

extern "C" void handle_error(const char *, const char *, va_list) {}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 8 || size > kMaxInputSize)
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

    int dirCount = 0;
    const int maxDirs = 20;

    do
    {
        if (++dirCount > maxDirs)
        {
            break;
        }

        // Check predictor tag
        uint16_t predictor = 0;
        TIFFGetField(tif, TIFFTAG_PREDICTOR, &predictor);

        // We're interested in images with predictors
        // PREDICTOR_NONE = 1, PREDICTOR_HORIZONTAL = 2, PREDICTOR_FLOATINGPOINT = 3
        if (predictor < 2)
        {
            // Still process to test predictor initialization path
        }

        uint16_t compression = 0;
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);

        // Predictors are typically used with LZW, Deflate, LZMA, ZSTD
        if (compression != COMPRESSION_LZW &&
            compression != COMPRESSION_DEFLATE &&
            compression != COMPRESSION_ADOBE_DEFLATE &&
            compression != COMPRESSION_LZMA &&
            compression != COMPRESSION_ZSTD)
        {
            continue;
        }

        uint32_t width = 0, height = 0;
        uint16_t bps = 0, spp = 0, sampleFormat = 0;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);

        if (width == 0 || height == 0 || width > 10000 || height > 10000)
        {
            continue;
        }

        // Check for floating-point predictor edge cases
        if (predictor == 3) // PREDICTOR_FLOATINGPOINT
        {
            // Floating-point predictor is complex - reorders bytes
            // Sample format should be floating-point
            if (sampleFormat == SAMPLEFORMAT_IEEEFP && (bps == 32 || bps == 64))
            {
                // Good - this is the intended use case
            }
            else
            {
                // Edge case - predictor 3 with non-float data
                // This can trigger interesting code paths
            }
        }

        // Read using different methods to exercise predictor
        if (TIFFIsTiled(tif))
        {
            tmsize_t tileSize = TIFFTileSize(tif);
            if (tileSize > 0 && (uint64_t)tileSize < kMaxAllocSize)
            {
                void *buf = _TIFFmalloc(tileSize);
                if (buf)
                {
                    uint32_t numTiles = TIFFNumberOfTiles(tif);
                    for (uint32_t t = 0; t < numTiles && t < 16; t++)
                    {
                        TIFFReadEncodedTile(tif, t, buf, tileSize);
                    }
                    _TIFFfree(buf);
                }
            }
        }
        else
        {
            // Try scanline reading - this tests predictor row-by-row
            tmsize_t scanlineSize = TIFFScanlineSize(tif);
            if (scanlineSize > 0 && (uint64_t)scanlineSize < kMaxAllocSize)
            {
                void *buf = _TIFFmalloc(scanlineSize);
                if (buf)
                {
                    for (uint32_t row = 0; row < height && row < 500; row++)
                    {
                        TIFFReadScanline(tif, buf, row, 0);
                    }
                    _TIFFfree(buf);
                }
            }

            // Also test strip reading
            tmsize_t stripSize = TIFFStripSize(tif);
            if (stripSize > 0 && (uint64_t)stripSize < kMaxAllocSize)
            {
                void *buf = _TIFFmalloc(stripSize);
                if (buf)
                {
                    uint32_t numStrips = TIFFNumberOfStrips(tif);
                    for (uint32_t s = 0; s < numStrips && s < 32; s++)
                    {
                        TIFFReadEncodedStrip(tif, s, buf, stripSize);
                    }
                    _TIFFfree(buf);
                }
            }
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
