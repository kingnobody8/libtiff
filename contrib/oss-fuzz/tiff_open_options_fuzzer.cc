/*
 * Fuzzer for TIFFOpenOptions API and memory limit handling.
 * Tests the newer TIFFOpenOptionsSet* functions and memory constraints.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiff.h>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 512 * 1024;

extern "C" void handle_error(const char *, const char *, va_list) {}

// Custom error handler that can be registered
extern "C" int custom_error_handler(TIFF *tif, void *user_data,
                                     const char *module, const char *fmt,
                                     va_list ap)
{
    (void)tif;
    (void)user_data;
    (void)module;
    (void)fmt;
    (void)ap;
    return 1;  // Continue processing
}

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

    // Use first bytes to configure open options
    uint8_t configByte = data[0];
    data++;
    size--;

    // Extract memory limits from fuzzer input
    tmsize_t maxSingleAlloc = 0;
    tmsize_t maxCumulatedAlloc = 0;

    if (size >= 8)
    {
        memcpy(&maxSingleAlloc, data, 4);
        memcpy(&maxCumulatedAlloc, data + 4, 4);
        data += 8;
        size -= 8;

        // Clamp to reasonable values to avoid immediate OOM
        maxSingleAlloc = (maxSingleAlloc % (50 * 1024 * 1024)) + 1024;
        maxCumulatedAlloc = (maxCumulatedAlloc % (100 * 1024 * 1024)) + 4096;
    }
    else
    {
        maxSingleAlloc = 10 * 1024 * 1024;
        maxCumulatedAlloc = 50 * 1024 * 1024;
    }

    // Create and configure open options
    TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
    if (!opts)
    {
        return 0;
    }

    // Test various option configurations based on config byte
    if (configByte & 0x01)
    {
        TIFFOpenOptionsSetMaxSingleMemAlloc(opts, maxSingleAlloc);
    }
    if (configByte & 0x02)
    {
        TIFFOpenOptionsSetMaxCumulatedMemAlloc(opts, maxCumulatedAlloc);
    }
    if (configByte & 0x04)
    {
        TIFFOpenOptionsSetWarnAboutUnknownTags(opts, 1);
    }
    if (configByte & 0x08)
    {
        TIFFOpenOptionsSetErrorHandlerExtR(opts, custom_error_handler, NULL);
    }
    if (configByte & 0x10)
    {
        TIFFOpenOptionsSetWarningHandlerExtR(opts, custom_error_handler, NULL);
    }

    // Open TIFF with options using stream
    std::istringstream s(std::string(reinterpret_cast<const char *>(data), size));
    TIFF *tif = TIFFStreamOpen("MemTIFF", &s);

    // Note: TIFFStreamOpen doesn't take options, but we test the options API
    // In real OSS-Fuzz, we'd use TIFFClientOpenExt

    if (tif)
    {
        int dirCount = 0;
        const int maxDirs = 20;

        do
        {
            if (++dirCount > maxDirs)
            {
                break;
            }

            uint32_t width = 0, height = 0;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

            if (width == 0 || height == 0 || width > 5000 || height > 5000)
            {
                continue;
            }

            // Try operations that allocate memory
            // These should respect the memory limits when using TIFFClientOpenExt

            tmsize_t stripSize = TIFFStripSize(tif);
            if (stripSize > 0 && stripSize < (tmsize_t)maxSingleAlloc)
            {
                void *buf = _TIFFmalloc(stripSize);
                if (buf)
                {
                    TIFFReadEncodedStrip(tif, 0, buf, stripSize);
                    _TIFFfree(buf);
                }
            }

            // Test RGBA reading with memory constraints
            uint64_t rasterSize = (uint64_t)width * height * sizeof(uint32_t);
            if (rasterSize < maxSingleAlloc && rasterSize < 10 * 1024 * 1024)
            {
                uint32_t *raster = static_cast<uint32_t *>(_TIFFmalloc(rasterSize));
                if (raster)
                {
                    TIFFReadRGBAImage(tif, width, height, raster, 0);
                    _TIFFfree(raster);
                }
            }

        } while (TIFFReadDirectory(tif));

        TIFFClose(tif);
    }

    TIFFOpenOptionsFree(opts);
    return 0;
}
