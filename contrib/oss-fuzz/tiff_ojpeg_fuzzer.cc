/*
 * Fuzzer for Old-style JPEG (OJPEG) handling in libtiff.
 * This targets the complex legacy JPEG parsing in tif_ojpeg.c
 * which has historically been a source of vulnerabilities.
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
    const int maxDirs = 10;

    do
    {
        if (++dirCount > maxDirs)
        {
            break;
        }

        uint16_t compression = 0;
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);

        // Focus on OJPEG and JPEG compression
        if (compression != COMPRESSION_OJPEG && compression != COMPRESSION_JPEG)
        {
            continue;
        }

        uint32_t width = 0, height = 0;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        if (width == 0 || height == 0 || width > 10000 || height > 10000)
        {
            continue;
        }

        // Check JPEG-specific tags
        uint32_t jpegIfOffset = 0, jpegIfByteCount = 0;
        uint16_t jpegProc = 0;
        void *jpegTables = NULL;
        uint32_t jpegTablesLen = 0;

        TIFFGetField(tif, TIFFTAG_JPEGIFOFFSET, &jpegIfOffset);
        TIFFGetField(tif, TIFFTAG_JPEGIFBYTECOUNT, &jpegIfByteCount);
        TIFFGetField(tif, TIFFTAG_JPEGPROC, &jpegProc);
        TIFFGetField(tif, TIFFTAG_JPEGTABLES, &jpegTablesLen, &jpegTables);

        // Try RGBA reading which exercises the full OJPEG decode path
        if ((uint64_t)width * height * 4 < kMaxAllocSize)
        {
            uint32_t *raster = static_cast<uint32_t *>(
                _TIFFmalloc(width * height * sizeof(uint32_t)));
            if (raster)
            {
                // This exercises the OJPEG reconstruction state machine
                TIFFReadRGBAImage(tif, width, height, raster, 0);
                _TIFFfree(raster);
            }
        }

        // Also try strip-by-strip reading
        if (!TIFFIsTiled(tif))
        {
            uint32_t rowsPerStrip = 0;
            TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);

            if (rowsPerStrip > 0 && rowsPerStrip <= height)
            {
                tmsize_t stripSize = TIFFStripSize(tif);
                if (stripSize > 0 && (uint64_t)stripSize < kMaxAllocSize)
                {
                    void *buf = _TIFFmalloc(stripSize);
                    if (buf)
                    {
                        uint32_t numStrips = TIFFNumberOfStrips(tif);
                        for (uint32_t s = 0; s < numStrips && s < 20; s++)
                        {
                            TIFFReadEncodedStrip(tif, s, buf, stripSize);
                        }
                        _TIFFfree(buf);
                    }
                }
            }
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
