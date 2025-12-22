/*
 * Fuzzer for custom tag handling in libtiff.
 * Targets tag extension, custom directory creation, and field manipulation.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiff.h>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 512 * 1024;  // 512KB max

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
    const int maxDirs = 30;

    do
    {
        if (++dirCount > maxDirs)
        {
            break;
        }

        // Test tag list enumeration
        int tagCount = TIFFGetTagListCount(tif);
        for (int i = 0; i < tagCount && i < 500; i++)
        {
            uint32_t tag = TIFFGetTagListEntry(tif, i);

            // Test field lookup functions
            const TIFFField *field = TIFFFieldWithTag(tif, tag);
            if (field)
            {
                (void)TIFFFieldTag(field);
                (void)TIFFFieldName(field);
                (void)TIFFFieldDataType(field);
                (void)TIFFFieldPassCount(field);
                (void)TIFFFieldReadCount(field);
                (void)TIFFFieldWriteCount(field);
                (void)TIFFFieldSetGetSize(field);
                (void)TIFFFieldSetGetCountSize(field);
                (void)TIFFFieldIsAnonymous(field);
            }

            // Also try TIFFFindField
            (void)TIFFFindField(tif, tag, TIFF_ANY);
        }

        // Test finding fields by name for common tags
        static const char *tagNames[] = {
            "ImageWidth", "ImageLength", "BitsPerSample",
            "Compression", "PhotometricInterpretation",
            "StripOffsets", "SamplesPerPixel", "RowsPerStrip",
            "StripByteCounts", "XResolution", "YResolution",
            "PlanarConfiguration", "ResolutionUnit",
            "Software", "DateTime", "Artist",
            "TileWidth", "TileLength", "TileOffsets", "TileByteCounts",
            "SubIFDs", "ExtraSamples", "SampleFormat",
            "JPEGTables", "YCbCrCoefficients", "YCbCrSubSampling",
            "ReferenceBlackWhite", "Copyright"
        };

        for (size_t i = 0; i < sizeof(tagNames) / sizeof(tagNames[0]); i++)
        {
            const TIFFField *field = TIFFFieldWithName(tif, tagNames[i]);
            if (field)
            {
                (void)TIFFFieldTag(field);
            }
        }

        // Test GetFieldDefaulted for some common tags
        uint16_t bps = 0, spp = 0, planar = 0, fillorder = 0;
        uint32_t rowsPerStrip = 0;
        float xres = 0, yres = 0;
        uint16_t resunit = 0;

        TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planar);
        TIFFGetFieldDefaulted(tif, TIFFTAG_FILLORDER, &fillorder);
        TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);
        TIFFGetFieldDefaulted(tif, TIFFTAG_XRESOLUTION, &xres);
        TIFFGetFieldDefaulted(tif, TIFFTAG_YRESOLUTION, &yres);
        TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &resunit);

        // Test data width function for various types
        for (int type = TIFF_NOTYPE; type <= TIFF_IFD8; type++)
        {
            (void)TIFFDataWidth(static_cast<TIFFDataType>(type));
        }

        // Try accessing custom tags (high tag numbers)
        for (uint32_t tag = 65000; tag < 65100; tag++)
        {
            const TIFFField *field = TIFFFieldWithTag(tif, tag);
            if (field)
            {
                (void)TIFFFieldIsAnonymous(field);
            }
        }

        // Test private tag range
        for (uint32_t tag = 32768; tag < 32800; tag++)
        {
            (void)TIFFFieldWithTag(tif, tag);
        }

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
