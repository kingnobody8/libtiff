/*
 * Fuzzer for libtiff codec implementations.
 * Targets specific compression codecs: LZW, PackBits, FAX3/4, Deflate, etc.
 * This fuzzer exercises the decompression paths more thoroughly.
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

        uint32_t width = 0, height = 0;
        uint16_t compression = 0;
        uint16_t bps = 0, spp = 0;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);

        if (width == 0 || height == 0)
        {
            continue;
        }

        // Limit dimensions to prevent OOM
        if (width > 10000 || height > 10000)
        {
            continue;
        }

        // Check if codec is configured
        if (!TIFFIsCODECConfigured(compression))
        {
            continue;
        }

        // Test based on compression type
        switch (compression)
        {
        case COMPRESSION_LZW:
        case COMPRESSION_DEFLATE:
        case COMPRESSION_ADOBE_DEFLATE:
        case COMPRESSION_PACKBITS:
        case COMPRESSION_ZSTD:
        case COMPRESSION_LZMA:
        case COMPRESSION_WEBP:
        case COMPRESSION_LERC:
        {
            // For general compression, read strips/tiles
            if (TIFFIsTiled(tif))
            {
                tmsize_t tileSize = TIFFTileSize(tif);
                if (tileSize > 0 && (uint64_t)tileSize < kMaxAllocSize)
                {
                    void *buf = _TIFFmalloc(tileSize);
                    if (buf)
                    {
                        uint32_t numTiles = TIFFNumberOfTiles(tif);
                        for (uint32_t t = 0; t < numTiles && t < 32; t++)
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
                        uint32_t numStrips = TIFFNumberOfStrips(tif);
                        for (uint32_t s = 0; s < numStrips && s < 64; s++)
                        {
                            TIFFReadEncodedStrip(tif, s, buf, stripSize);
                        }
                        _TIFFfree(buf);
                    }
                }
            }
            break;
        }

        case COMPRESSION_CCITTRLE:
        case COMPRESSION_CCITTRLEW:
        case COMPRESSION_CCITTFAX3:  // Same as COMPRESSION_CCITT_T4
        case COMPRESSION_CCITTFAX4:  // Same as COMPRESSION_CCITT_T6
        {
            // FAX compression - typically 1-bit images
            if (bps == 1)
            {
                tmsize_t stripSize = TIFFStripSize(tif);
                if (stripSize > 0 && (uint64_t)stripSize < kMaxAllocSize)
                {
                    void *buf = _TIFFmalloc(stripSize);
                    if (buf)
                    {
                        uint32_t numStrips = TIFFNumberOfStrips(tif);
                        for (uint32_t s = 0; s < numStrips && s < 64; s++)
                        {
                            TIFFReadEncodedStrip(tif, s, buf, stripSize);
                        }
                        _TIFFfree(buf);
                    }
                }
            }
            break;
        }

        case COMPRESSION_JPEG:
        case COMPRESSION_OJPEG:
        {
            // JPEG compression - try RGBA reading which exercises JPEG decode
            if ((uint64_t)width * height * 4 < kMaxAllocSize)
            {
                uint32_t *raster = static_cast<uint32_t *>(
                    _TIFFmalloc(width * height * sizeof(uint32_t)));
                if (raster)
                {
                    TIFFReadRGBAImage(tif, width, height, raster, 0);
                    _TIFFfree(raster);
                }
            }
            break;
        }

        case COMPRESSION_JBIG:
        {
            // JBIG compression
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
            break;
        }

        case COMPRESSION_SGILOG:
        case COMPRESSION_SGILOG24:
        {
            // LogLuv compression - try RGBA reading
            if ((uint64_t)width * height * 4 < kMaxAllocSize)
            {
                uint32_t *raster = static_cast<uint32_t *>(
                    _TIFFmalloc(width * height * sizeof(uint32_t)));
                if (raster)
                {
                    TIFFReadRGBAImage(tif, width, height, raster, 0);
                    _TIFFfree(raster);
                }
            }
            break;
        }

        case COMPRESSION_NONE:
        default:
        {
            // Uncompressed or unknown - read raw
            tmsize_t scanlineSize = TIFFScanlineSize(tif);
            if (scanlineSize > 0 && (uint64_t)scanlineSize < kMaxAllocSize)
            {
                void *buf = _TIFFmalloc(scanlineSize);
                if (buf)
                {
                    for (uint32_t row = 0; row < height && row < 1000; row++)
                    {
                        TIFFReadScanline(tif, buf, row, 0);
                    }
                    _TIFFfree(buf);
                }
            }
            break;
        }
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
