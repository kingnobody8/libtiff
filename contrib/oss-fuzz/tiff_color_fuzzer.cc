/*
 * Fuzzer for libtiff color conversion functions.
 * Targets: TIFFYCbCrToRGBInit, TIFFYCbCrtoRGB, TIFFCIELabToRGBInit,
 *          TIFFCIELabToXYZ, TIFFXYZToRGB, LogLuv functions
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <tiff.h>
#include <tiffio.h>
#include <tiffio.hxx>

static const size_t kMaxInputSize = 512 * 1024;  // 512KB max input
static const uint64_t kMaxAllocSize = 50 * 1024 * 1024;  // 50MB max alloc

extern "C" void handle_error(const char *, const char *, va_list) {}

// Helper to safely allocate YCbCrToRGB structure
static TIFFYCbCrToRGB *allocYCbCrToRGB()
{
    // As per tiffio.h documentation
    size_t size = sizeof(TIFFYCbCrToRGB);
    size = ((size + sizeof(long) - 1) / sizeof(long)) * sizeof(long);  // Round up
    size += 4 * 256 * sizeof(TIFFRGBValue);
    size += 2 * 256 * sizeof(int);
    size += 3 * 256 * sizeof(int32_t);

    return static_cast<TIFFYCbCrToRGB *>(_TIFFmalloc(size));
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

    // Use first few bytes to control fuzzer behavior
    uint8_t mode = data[0] % 4;
    data++;
    size--;

    if (mode == 0)
    {
        // Test YCbCr to RGB conversion
        if (size < 6 * sizeof(float))
        {
            return 0;
        }

        float luma[3];
        float refBlackWhite[6];

        memcpy(luma, data, 3 * sizeof(float));
        data += 3 * sizeof(float);
        size -= 3 * sizeof(float);

        memcpy(refBlackWhite, data, 6 * sizeof(float));
        data += 6 * sizeof(float);
        size -= 6 * sizeof(float);

        // Validate inputs aren't NaN or Inf
        for (int i = 0; i < 3; i++)
        {
            if (luma[i] != luma[i] || luma[i] > 1e10 || luma[i] < -1e10)
            {
                return 0;
            }
        }
        for (int i = 0; i < 6; i++)
        {
            if (refBlackWhite[i] != refBlackWhite[i] ||
                refBlackWhite[i] > 1e10 || refBlackWhite[i] < -1e10)
            {
                return 0;
            }
        }

        TIFFYCbCrToRGB *ycbcr = allocYCbCrToRGB();
        if (!ycbcr)
        {
            return 0;
        }

        if (TIFFYCbCrToRGBInit(ycbcr, luma, refBlackWhite) == 0)
        {
            // Successful init - test conversion
            while (size >= 3)
            {
                uint32_t Y = data[0];
                int32_t Cb = static_cast<int32_t>(data[1]) - 128;
                int32_t Cr = static_cast<int32_t>(data[2]) - 128;

                uint32_t r, g, b;
                TIFFYCbCrtoRGB(ycbcr, Y, Cb, Cr, &r, &g, &b);

                data += 3;
                size -= 3;

                // Limit iterations
                if (size > 1000)
                {
                    break;
                }
            }
        }

        _TIFFfree(ycbcr);
    }
    else if (mode == 1)
    {
        // Test CIE Lab to RGB conversion
        TIFFCIELabToRGB cielab;
        memset(&cielab, 0, sizeof(cielab));

        TIFFDisplay display;
        memset(&display, 0, sizeof(display));

        // Set reasonable display values
        display.d_mat[0][0] = 1.0f;
        display.d_mat[1][1] = 1.0f;
        display.d_mat[2][2] = 1.0f;
        display.d_YCR = 1.0f;
        display.d_YCG = 1.0f;
        display.d_YCB = 1.0f;
        display.d_gammaR = 2.2f;
        display.d_gammaG = 2.2f;
        display.d_gammaB = 2.2f;

        float refWhite[3] = {D50_X0, D50_Y0, D50_Z0};

        if (size >= 3 * sizeof(float))
        {
            memcpy(refWhite, data, 3 * sizeof(float));
            data += 3 * sizeof(float);
            size -= 3 * sizeof(float);

            // Validate
            for (int i = 0; i < 3; i++)
            {
                if (refWhite[i] != refWhite[i] || refWhite[i] <= 0 ||
                    refWhite[i] > 1000)
                {
                    refWhite[i] = 100.0f;
                }
            }
        }

        if (TIFFCIELabToRGBInit(&cielab, &display, refWhite) == 0)
        {
            // Test conversions
            while (size >= 3)
            {
                uint32_t L = data[0];
                int32_t a = static_cast<int32_t>(data[1]) - 128;
                int32_t b_val = static_cast<int32_t>(data[2]) - 128;

                float X, Y, Z;
                TIFFCIELabToXYZ(&cielab, L, a, b_val, &X, &Y, &Z);

                uint32_t r, g, b_out;
                TIFFXYZToRGB(&cielab, X, Y, Z, &r, &g, &b_out);

                data += 3;
                size -= 3;

                if (size > 1000)
                {
                    break;
                }
            }
        }
    }
    else if (mode == 2)
    {
        // Test with actual TIFF file for YCbCr images
        std::istringstream s(std::string(reinterpret_cast<const char *>(data), size));
        TIFF *tif = TIFFStreamOpen("MemTIFF", &s);
        if (!tif)
        {
            return 0;
        }

        // Check if this is a YCbCr image
        uint16_t photometric = 0;
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

        if (photometric == PHOTOMETRIC_YCBCR)
        {
            uint32_t width = 0, height = 0;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

            if (width > 0 && height > 0 &&
                (uint64_t)width * height * 4 < kMaxAllocSize)
            {
                uint32_t *raster = static_cast<uint32_t *>(
                    _TIFFmalloc(width * height * sizeof(uint32_t)));
                if (raster)
                {
                    TIFFReadRGBAImage(tif, width, height, raster, 0);
                    _TIFFfree(raster);
                }
            }
        }

        TIFFClose(tif);
    }
    else
    {
        // Test with actual TIFF file for RGBA strip/tile reading
        std::istringstream s(std::string(reinterpret_cast<const char *>(data), size));
        TIFF *tif = TIFFStreamOpen("MemTIFF", &s);
        if (!tif)
        {
            return 0;
        }

        uint32_t width = 0, height = 0;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        if (width == 0 || height == 0 ||
            (uint64_t)width * height * 4 > kMaxAllocSize)
        {
            TIFFClose(tif);
            return 0;
        }

        // Test RGBA strip reading
        if (!TIFFIsTiled(tif))
        {
            uint32_t rowsPerStrip = 0;
            TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);

            if (rowsPerStrip > 0 && rowsPerStrip <= height)
            {
                uint32_t *raster = static_cast<uint32_t *>(
                    _TIFFmalloc(width * rowsPerStrip * sizeof(uint32_t)));

                if (raster)
                {
                    for (uint32_t row = 0; row < height; row += rowsPerStrip)
                    {
                        TIFFReadRGBAStrip(tif, row, raster);
                        if (row > height / 2)
                        {
                            break;  // Limit iterations
                        }
                    }
                    _TIFFfree(raster);
                }
            }
        }
        else
        {
            // Test RGBA tile reading
            uint32_t tileWidth = 0, tileLength = 0;
            TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth);
            TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileLength);

            if (tileWidth > 0 && tileLength > 0 &&
                (uint64_t)tileWidth * tileLength * 4 < kMaxAllocSize)
            {
                uint32_t *raster = static_cast<uint32_t *>(
                    _TIFFmalloc(tileWidth * tileLength * sizeof(uint32_t)));

                if (raster)
                {
                    int tileCount = 0;
                    for (uint32_t y = 0; y < height && tileCount < 16; y += tileLength)
                    {
                        for (uint32_t x = 0; x < width && tileCount < 16; x += tileWidth)
                        {
                            TIFFReadRGBATile(tif, x, y, raster);
                            tileCount++;
                        }
                    }
                    _TIFFfree(raster);
                }
            }
        }

        TIFFClose(tif);
    }

    return 0;
}
