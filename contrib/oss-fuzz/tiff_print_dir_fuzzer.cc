/*
 * Fuzzer for libtiff print and dump functions.
 * Targets: TIFFPrintDirectory, various size calculation functions
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 512 * 1024;  // 512KB max input

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

    int dirCount = 0;
    const int maxDirs = 50;

    do
    {
        if (++dirCount > maxDirs)
        {
            break;
        }

        // Test TIFFPrintDirectory with various flags
        // Output to /dev/null to avoid excessive output
        FILE *devnull = fopen("/dev/null", "w");
        if (devnull)
        {
            TIFFPrintDirectory(tif, devnull, TIFFPRINT_NONE);
            TIFFPrintDirectory(tif, devnull, TIFFPRINT_STRIPS);
            TIFFPrintDirectory(tif, devnull, TIFFPRINT_CURVES);
            TIFFPrintDirectory(tif, devnull, TIFFPRINT_COLORMAP);
            TIFFPrintDirectory(tif, devnull, TIFFPRINT_JPEGQTABLES);
            TIFFPrintDirectory(tif, devnull,
                               TIFFPRINT_STRIPS | TIFFPRINT_CURVES |
                               TIFFPRINT_COLORMAP | TIFFPRINT_JPEGQTABLES);
            fclose(devnull);
        }

        // Test various size calculation functions
        (void)TIFFScanlineSize(tif);
        (void)TIFFScanlineSize64(tif);
        (void)TIFFRasterScanlineSize(tif);
        (void)TIFFRasterScanlineSize64(tif);
        (void)TIFFStripSize(tif);
        (void)TIFFStripSize64(tif);
        (void)TIFFVStripSize(tif, 1);
        (void)TIFFVStripSize64(tif, 1);
        (void)TIFFTileRowSize(tif);
        (void)TIFFTileRowSize64(tif);
        (void)TIFFTileSize(tif);
        (void)TIFFTileSize64(tif);
        (void)TIFFVTileSize(tif, 1);
        (void)TIFFVTileSize64(tif, 1);
        (void)TIFFDefaultStripSize(tif, 0);

        uint32_t tw = 0, tl = 0;
        TIFFDefaultTileSize(tif, &tw, &tl);

        // Test number of strips/tiles
        (void)TIFFNumberOfStrips(tif);
        (void)TIFFNumberOfTiles(tif);

        // Test various info functions
        (void)TIFFIsTiled(tif);
        (void)TIFFIsByteSwapped(tif);
        (void)TIFFIsUpSampled(tif);
        (void)TIFFIsMSB2LSB(tif);
        (void)TIFFIsBigEndian(tif);
        (void)TIFFIsBigTIFF(tif);
        (void)TIFFGetMode(tif);
        (void)TIFFFileno(tif);
        (void)TIFFFileName(tif);

        // Test current position info
        (void)TIFFCurrentRow(tif);
        (void)TIFFCurrentStrip(tif);
        (void)TIFFCurrentTile(tif);

        // Test strip/tile computation
        uint32_t width = 0, height = 0;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        if (width > 0 && height > 0 && width < 100000 && height < 100000)
        {
            uint16_t spp = 1;
            TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);

            // Test strip computation
            for (uint32_t row = 0; row < height && row < 100; row += 10)
            {
                (void)TIFFComputeStrip(tif, row, 0);
                if (spp > 1)
                {
                    (void)TIFFComputeStrip(tif, row, 1);
                }
            }

            // Test tile computation
            if (TIFFIsTiled(tif))
            {
                for (uint32_t y = 0; y < height && y < 100; y += 10)
                {
                    for (uint32_t x = 0; x < width && x < 100; x += 10)
                    {
                        (void)TIFFComputeTile(tif, x, y, 0, 0);
                        (void)TIFFCheckTile(tif, x, y, 0, 0);
                    }
                }
            }
        }

        // Test raw strip/tile size
        uint32_t numStrips = TIFFNumberOfStrips(tif);
        for (uint32_t i = 0; i < numStrips && i < 10; i++)
        {
            (void)TIFFRawStripSize(tif, i);
            (void)TIFFRawStripSize64(tif, i);
        }

        // Test strile offset/bytecount functions
        uint32_t numStriles = TIFFIsTiled(tif) ? TIFFNumberOfTiles(tif)
                                                : TIFFNumberOfStrips(tif);
        for (uint32_t i = 0; i < numStriles && i < 10; i++)
        {
            int err = 0;
            (void)TIFFGetStrileOffset(tif, i);
            (void)TIFFGetStrileByteCount(tif, i);
            (void)TIFFGetStrileOffsetWithErr(tif, i, &err);
            (void)TIFFGetStrileByteCountWithErr(tif, i, &err);
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
