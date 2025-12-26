/*
 * Fuzzer for multi-page/multi-directory TIFF handling.
 * Targets directory navigation, linking, and manipulation.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiff.h>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 1024 * 1024;  // 1MB max
static const uint64_t kMaxAllocSize = 20 * 1024 * 1024;  // 20MB max

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

    // Get total number of directories
    tdir_t numDirs = TIFFNumberOfDirectories(tif);
    if (numDirs > 1000)
    {
        numDirs = 1000;  // Limit to prevent excessive iteration
    }

    // Test forward traversal
    tdir_t visitedForward = 0;
    do
    {
        visitedForward++;
        if (visitedForward > numDirs + 10)
        {
            break;  // Safety limit
        }

        // Basic directory info
        (void)TIFFCurrentDirectory(tif);
        (void)TIFFCurrentDirOffset(tif);
        (void)TIFFLastDirectory(tif);

        // Read basic tags
        uint32_t width = 0, height = 0;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        // Read page number if present
        uint16_t pageNum = 0, pageTotal = 0;
        TIFFGetField(tif, TIFFTAG_PAGENUMBER, &pageNum, &pageTotal);

    } while (TIFFReadDirectory(tif));

    // Test random access to directories
    for (tdir_t d = 0; d < numDirs && d < 50; d++)
    {
        if (TIFFSetDirectory(tif, d))
        {
            uint32_t width = 0;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);

            // Try reading a small amount of data
            if (width > 0 && width < 10000)
            {
                tmsize_t scanlineSize = TIFFScanlineSize(tif);
                if (scanlineSize > 0 && (uint64_t)scanlineSize < kMaxAllocSize)
                {
                    void *buf = _TIFFmalloc(scanlineSize);
                    if (buf)
                    {
                        TIFFReadScanline(tif, buf, 0, 0);
                        _TIFFfree(buf);
                    }
                }
            }
        }
    }

    // Test reverse traversal using SetDirectory
    for (tdir_t d = numDirs; d > 0 && d <= numDirs; d--)
    {
        if (d > 50)
        {
            continue;  // Skip middle directories
        }
        TIFFSetDirectory(tif, d - 1);
    }

    // Test SetSubDirectory with various offsets from the file
    // Use bytes from input as potential directory offsets
    if (size > 100)
    {
        for (size_t i = 8; i < size - 8 && i < 200; i += 8)
        {
            uint64_t offset = 0;
            memcpy(&offset, data + i, sizeof(offset));

            // Limit offset to reasonable range
            if (offset > 0 && offset < size * 2)
            {
                TIFFSetSubDirectory(tif, offset);
            }
        }
    }

    // Go back to first directory and test some edge cases
    TIFFSetDirectory(tif, 0);

    // Test directory iteration count
    tdir_t count = 0;
    while (TIFFReadDirectory(tif) && count < 100)
    {
        count++;
    }

    TIFFClose(tif);
    return 0;
}
