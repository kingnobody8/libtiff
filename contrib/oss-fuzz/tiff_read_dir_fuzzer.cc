/*
 * Fuzzer for libtiff directory and tag parsing functions.
 * Targets: TIFFReadDirectory, TIFFReadEXIFDirectory, TIFFReadGPSDirectory,
 *          TIFFGetField, TIFFFieldWithTag, TIFFGetTagListEntry
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 1024 * 1024;  // 1MB max input

extern "C" void handle_error(const char *, const char *, va_list) {}

// Common TIFF tags to query
static const uint32_t kCommonTags[] = {
    TIFFTAG_IMAGEWIDTH,
    TIFFTAG_IMAGELENGTH,
    TIFFTAG_BITSPERSAMPLE,
    TIFFTAG_COMPRESSION,
    TIFFTAG_PHOTOMETRIC,
    TIFFTAG_FILLORDER,
    TIFFTAG_STRIPOFFSETS,
    TIFFTAG_ORIENTATION,
    TIFFTAG_SAMPLESPERPIXEL,
    TIFFTAG_ROWSPERSTRIP,
    TIFFTAG_STRIPBYTECOUNTS,
    TIFFTAG_XRESOLUTION,
    TIFFTAG_YRESOLUTION,
    TIFFTAG_PLANARCONFIG,
    TIFFTAG_RESOLUTIONUNIT,
    TIFFTAG_SOFTWARE,
    TIFFTAG_DATETIME,
    TIFFTAG_ARTIST,
    TIFFTAG_HOSTCOMPUTER,
    TIFFTAG_COLORMAP,
    TIFFTAG_TILEWIDTH,
    TIFFTAG_TILELENGTH,
    TIFFTAG_TILEOFFSETS,
    TIFFTAG_TILEBYTECOUNTS,
    TIFFTAG_SUBIFD,
    TIFFTAG_EXTRASAMPLES,
    TIFFTAG_SAMPLEFORMAT,
    TIFFTAG_IMAGEDEPTH,
    TIFFTAG_TILEDEPTH,
    TIFFTAG_YCBCRCOEFFICIENTS,
    TIFFTAG_YCBCRSUBSAMPLING,
    TIFFTAG_YCBCRPOSITIONING,
    TIFFTAG_REFERENCEBLACKWHITE,
    TIFFTAG_XMLPACKET,
    TIFFTAG_ICCPROFILE,
    TIFFTAG_STONITS,
    TIFFTAG_EXIFIFD,
    TIFFTAG_GPSIFD,
    TIFFTAG_COPYRIGHT,
    TIFFTAG_DOCUMENTNAME,
    TIFFTAG_IMAGEDESCRIPTION,
    TIFFTAG_MAKE,
    TIFFTAG_MODEL,
    TIFFTAG_PAGENAME,
    TIFFTAG_PAGENUMBER,
    TIFFTAG_HALFTONEHINTS,
    TIFFTAG_BADFAXLINES,
    TIFFTAG_CLEANFAXDATA,
    TIFFTAG_CONSECUTIVEBADFAXLINES,
    TIFFTAG_INKSET,
    TIFFTAG_INKNAMES,
    TIFFTAG_NUMBEROFINKS,
    TIFFTAG_DOTRANGE,
    TIFFTAG_TARGETPRINTER,
    TIFFTAG_THRESHHOLDING,
    TIFFTAG_CELLWIDTH,
    TIFFTAG_CELLLENGTH,
    TIFFTAG_MINSAMPLEVALUE,
    TIFFTAG_MAXSAMPLEVALUE,
    TIFFTAG_SMINSAMPLEVALUE,
    TIFFTAG_SMAXSAMPLEVALUE,
    TIFFTAG_JPEGPROC,
    TIFFTAG_JPEGIFOFFSET,
    TIFFTAG_JPEGIFBYTECOUNT,
    TIFFTAG_JPEGTABLES,
    TIFFTAG_RICHTIFFIPTC,
    TIFFTAG_PHOTOSHOP,
};

static const size_t kNumTags = sizeof(kCommonTags) / sizeof(kCommonTags[0]);

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
    const int maxDirs = 100;  // Limit directory traversal

    do
    {
        if (++dirCount > maxDirs)
        {
            break;
        }

        // Test tag list enumeration
        int tagCount = TIFFGetTagListCount(tif);
        for (int i = 0; i < tagCount && i < 1000; i++)
        {
            uint32_t tag = TIFFGetTagListEntry(tif, i);
            (void)TIFFFieldWithTag(tif, tag);
        }

        // Test reading some common tags with proper types
        uint32_t width = 0, height = 0;
        uint16_t bps = 0, spp = 0, compression = 0, photometric = 0;
        uint16_t planar = 0, orientation = 0, resunit = 0;
        float xres = 0, yres = 0;
        char *software = NULL;
        char *datetime = NULL;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar);
        TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation);
        TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres);
        TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres);
        TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &resunit);
        TIFFGetField(tif, TIFFTAG_SOFTWARE, &software);
        TIFFGetField(tif, TIFFTAG_DATETIME, &datetime);

        // Test TIFFFieldWithTag for various tags
        for (size_t i = 0; i < kNumTags; i++)
        {
            (void)TIFFFieldWithTag(tif, kCommonTags[i]);
        }

        // Try reading EXIF directory if present
        uint64_t exifOffset = 0;
        if (TIFFGetField(tif, TIFFTAG_EXIFIFD, &exifOffset))
        {
            // Save current directory
            tdir_t curDir = TIFFCurrentDirectory(tif);

            // Try to read EXIF directory
            if (TIFFReadEXIFDirectory(tif, exifOffset))
            {
                // Query some EXIF-specific tags
                uint32_t exifVersion = 0;
                TIFFGetField(tif, EXIFTAG_EXIFVERSION, &exifVersion);
            }

            // Try to restore original directory
            TIFFSetDirectory(tif, curDir);
        }

        // Try reading GPS directory if present
        uint64_t gpsOffset = 0;
        if (TIFFGetField(tif, TIFFTAG_GPSIFD, &gpsOffset))
        {
            tdir_t curDir = TIFFCurrentDirectory(tif);

            if (TIFFReadGPSDirectory(tif, gpsOffset))
            {
                // Query some GPS-specific tags
                double latitude[3];
                TIFFGetField(tif, GPSTAG_LATITUDE, &latitude);
            }

            TIFFSetDirectory(tif, curDir);
        }

        // Test directory navigation
        tdir_t numDirs = TIFFNumberOfDirectories(tif);
        (void)numDirs;

        // Test current directory info
        (void)TIFFCurrentDirectory(tif);
        (void)TIFFCurrentDirOffset(tif);
        (void)TIFFLastDirectory(tif);

    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}
