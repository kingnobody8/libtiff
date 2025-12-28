/*
 * Helper testing routines - header file.
 */

#ifndef TEST_CHECK_TAG_H
#define TEST_CHECK_TAG_H

#include "tiffio.h"

int CheckShortField(TIFF *, const ttag_t, const uint16_t);
int CheckShortPairedField(TIFF *, const ttag_t, const uint16_t *);
int CheckLongField(TIFF *, const ttag_t, const uint32_t);

#endif /* TEST_CHECK_TAG_H */
