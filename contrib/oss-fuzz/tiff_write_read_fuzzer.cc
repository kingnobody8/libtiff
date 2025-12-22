/*
 * Fuzzer for libtiff write and round-trip operations.
 * Tests write functions by creating TIFF in memory and reading back.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <tiffio.h>

static const size_t kMaxInputSize = 256 * 1024;  // 256KB max
static const uint32_t kMaxDimension = 1024;

extern "C" void handle_error(const char *, const char *, va_list) {}

// Memory buffer for in-memory TIFF
struct MemTIFF
{
    uint8_t *data;
    toff_t size;
    toff_t alloc;
    toff_t offset;
};

static tmsize_t memRead(thandle_t h, void *buf, tmsize_t size)
{
    MemTIFF *mem = static_cast<MemTIFF *>(h);
    tmsize_t toRead = size;
    if (mem->offset + toRead > mem->size)
    {
        toRead = mem->size - mem->offset;
    }
    if (toRead > 0)
    {
        memcpy(buf, mem->data + mem->offset, toRead);
        mem->offset += toRead;
    }
    return toRead;
}

static tmsize_t memWrite(thandle_t h, void *buf, tmsize_t size)
{
    MemTIFF *mem = static_cast<MemTIFF *>(h);
    if (mem->offset + size > mem->alloc)
    {
        toff_t newAlloc = mem->alloc * 2;
        if (newAlloc < mem->offset + size)
        {
            newAlloc = mem->offset + size + 1024;
        }
        if (newAlloc > 100 * 1024 * 1024)
        {
            return 0;  // Limit to 100MB
        }
        uint8_t *newData = static_cast<uint8_t *>(realloc(mem->data, newAlloc));
        if (!newData)
        {
            return 0;
        }
        mem->data = newData;
        mem->alloc = newAlloc;
    }
    memcpy(mem->data + mem->offset, buf, size);
    mem->offset += size;
    if (mem->offset > mem->size)
    {
        mem->size = mem->offset;
    }
    return size;
}

static toff_t memSeek(thandle_t h, toff_t offset, int whence)
{
    MemTIFF *mem = static_cast<MemTIFF *>(h);
    switch (whence)
    {
    case SEEK_SET:
        mem->offset = offset;
        break;
    case SEEK_CUR:
        mem->offset += offset;
        break;
    case SEEK_END:
        mem->offset = mem->size + offset;
        break;
    }
    return mem->offset;
}

static int memClose(thandle_t) { return 0; }

static toff_t memSize(thandle_t h)
{
    MemTIFF *mem = static_cast<MemTIFF *>(h);
    return mem->size;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 20 || size > kMaxInputSize)
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

    // Parse fuzzer input to get image parameters
    uint32_t width = (data[0] | (data[1] << 8)) % kMaxDimension + 1;
    uint32_t height = (data[2] | (data[3] << 8)) % kMaxDimension + 1;
    uint16_t bps = (data[4] % 4 + 1) * 8;  // 8, 16, 24, or 32
    uint16_t spp = (data[5] % 4) + 1;      // 1-4 samples per pixel
    uint16_t photometric = data[6] % 6;    // MINISWHITE to YCBCR
    uint16_t compression = data[7];
    uint16_t planar = (data[8] % 2) + 1;   // CONTIG or SEPARATE
    uint8_t useTiles = data[9] % 2;

    data += 10;
    size -= 10;

    // Map compression to valid values
    switch (compression % 10)
    {
    case 0:
        compression = COMPRESSION_NONE;
        break;
    case 1:
        compression = COMPRESSION_LZW;
        break;
    case 2:
        compression = COMPRESSION_PACKBITS;
        break;
    case 3:
        compression = COMPRESSION_DEFLATE;
        break;
    case 4:
        compression = COMPRESSION_ADOBE_DEFLATE;
        break;
    case 5:
        compression = COMPRESSION_CCITTRLE;
        break;
    case 6:
        compression = COMPRESSION_CCITTFAX3;
        break;
    case 7:
        compression = COMPRESSION_CCITTFAX4;
        break;
    default:
        compression = COMPRESSION_NONE;
        break;
    }

    // Limit dimensions to avoid OOM
    if ((uint64_t)width * height * spp * (bps / 8) > 10 * 1024 * 1024)
    {
        width = 256;
        height = 256;
    }

    // Create memory buffer for writing
    MemTIFF mem;
    mem.data = static_cast<uint8_t *>(malloc(64 * 1024));
    if (!mem.data)
    {
        return 0;
    }
    mem.size = 0;
    mem.alloc = 64 * 1024;
    mem.offset = 0;

    // Open TIFF for writing
    TIFF *tif = TIFFClientOpen("MemTIFF", "w", &mem, memRead, memWrite, memSeek,
                                memClose, memSize, NULL, NULL);
    if (!tif)
    {
        free(mem.data);
        return 0;
    }

    // Set basic tags
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planar);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, compression);

    if (useTiles)
    {
        uint32_t tileWidth = 64;
        uint32_t tileLength = 64;
        TIFFSetField(tif, TIFFTAG_TILEWIDTH, tileWidth);
        TIFFSetField(tif, TIFFTAG_TILELENGTH, tileLength);
    }
    else
    {
        uint32_t rowsPerStrip = TIFFDefaultStripSize(tif, 0);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);
    }

    // Create image data from fuzzer input
    tmsize_t scanlineSize = TIFFScanlineSize(tif);
    if (scanlineSize <= 0 || scanlineSize > 10 * 1024 * 1024)
    {
        TIFFClose(tif);
        free(mem.data);
        return 0;
    }

    uint8_t *scanline = static_cast<uint8_t *>(_TIFFmalloc(scanlineSize));
    if (!scanline)
    {
        TIFFClose(tif);
        free(mem.data);
        return 0;
    }

    // Write image data
    if (useTiles)
    {
        tmsize_t tileSize = TIFFTileSize(tif);
        if (tileSize > 0 && tileSize < 10 * 1024 * 1024)
        {
            uint8_t *tile = static_cast<uint8_t *>(_TIFFmalloc(tileSize));
            if (tile)
            {
                uint32_t numTiles = TIFFNumberOfTiles(tif);
                for (uint32_t t = 0; t < numTiles && t < 64; t++)
                {
                    // Fill tile with fuzzer data
                    for (tmsize_t i = 0; i < tileSize; i++)
                    {
                        tile[i] = data[i % size];
                    }
                    TIFFWriteEncodedTile(tif, t, tile, tileSize);
                }
                _TIFFfree(tile);
            }
        }
    }
    else
    {
        for (uint32_t row = 0; row < height; row++)
        {
            // Fill scanline with fuzzer data
            for (tmsize_t i = 0; i < scanlineSize; i++)
            {
                scanline[i] = data[(row * scanlineSize + i) % size];
            }
            if (TIFFWriteScanline(tif, scanline, row, 0) < 0)
            {
                break;
            }
        }
    }

    _TIFFfree(scanline);

    // Write directory and close
    TIFFWriteDirectory(tif);
    TIFFClose(tif);

    // Now read back the TIFF we just wrote
    if (mem.size > 0)
    {
        mem.offset = 0;

        TIFF *tifRead = TIFFClientOpen("MemTIFF", "r", &mem, memRead, memWrite,
                                        memSeek, memClose, memSize, NULL, NULL);
        if (tifRead)
        {
            uint32_t readWidth = 0, readHeight = 0;
            TIFFGetField(tifRead, TIFFTAG_IMAGEWIDTH, &readWidth);
            TIFFGetField(tifRead, TIFFTAG_IMAGELENGTH, &readHeight);

            // Read some data back
            tmsize_t readScanlineSize = TIFFScanlineSize(tifRead);
            if (readScanlineSize > 0 && readScanlineSize < 10 * 1024 * 1024)
            {
                uint8_t *readBuf = static_cast<uint8_t *>(_TIFFmalloc(readScanlineSize));
                if (readBuf)
                {
                    for (uint32_t row = 0; row < readHeight && row < 100; row++)
                    {
                        TIFFReadScanline(tifRead, readBuf, row, 0);
                    }
                    _TIFFfree(readBuf);
                }
            }

            TIFFClose(tifRead);
        }
    }

    free(mem.data);
    return 0;
}
