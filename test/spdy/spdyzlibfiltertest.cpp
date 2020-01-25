/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2018  LiteSpeed Technologies, Inc.                 *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#ifdef RUN_TEST

#include <spdy/spdydebug.h>
#include <spdy/spdyzlibfilter.h>
#include <spdy/spdyconnection.h>
#include <util/autobuf.h>
#include "UnitTest++/UnitTest++.h"
unsigned char c2s_SYN_STREAM1[] =
{
    0x80, 0x02, 0x00, 0x01, 0x01, 0x00, 0x01, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0xEA, 0xDF, 0xA2, 0x51, 0xB2, 0x62, 0xE0, 0x66, 0x60, 0x83, 0xA4, 0x17, 0x06,
    0x7B, 0xB8, 0x0B, 0x75, 0x30, 0x2C, 0xD6, 0xAE, 0x40, 0x17, 0xCD, 0xCD, 0xB1, 0x2E, 0xB4, 0x35,
    0xD0, 0xB3, 0xD4, 0xD1, 0xD2, 0xD7, 0x02, 0xB3, 0x2C, 0x18, 0xF8, 0x50, 0x73, 0x2C, 0x83, 0x9C,
    0x67, 0xB0, 0x3F, 0xD4, 0x3D, 0x3A, 0x60, 0x07, 0x81, 0xD5, 0x99, 0xEB, 0x40, 0xD4, 0x1B, 0x33,
    0xF0, 0xA3, 0xE5, 0x69, 0x06, 0x41, 0x90, 0x8B, 0x75, 0xA0, 0x4E, 0xD6, 0x29, 0x4E, 0x49, 0xCE,
    0x80, 0xAB, 0x81, 0x25, 0x03, 0x06, 0xBE, 0xD4, 0x3C, 0xDD, 0xD0, 0x60, 0x9D, 0xD4, 0x3C, 0xA8,
    0xA5, 0xBC, 0x28, 0x89, 0x8D, 0x81, 0x13, 0x20, 0x80, 0x72, 0x13, 0x2B, 0x74, 0x13, 0xD3, 0x53,
    0x6D, 0x0D, 0x00, 0x02, 0x88, 0x81, 0x05, 0x94, 0xFB, 0x19, 0xF8, 0x40, 0x49, 0x24, 0x07, 0xC4,
    0xB4, 0xB2, 0x04, 0x66, 0x3A, 0x06, 0xB6, 0x5C, 0x60, 0xA9, 0x93, 0x9F, 0xC2, 0xC0, 0xEC, 0xEE,
    0x1A, 0xC2, 0xC0, 0x56, 0x0C, 0xD4, 0x9B, 0x9B, 0x0A, 0x54, 0x5A, 0x52, 0x52, 0xC0, 0xC0, 0x0C,
    0x0A, 0x10, 0x46, 0x80, 0x00, 0xD2, 0x07, 0x08, 0x20, 0x06, 0x2E, 0x44, 0x2E, 0x66, 0x48, 0xF3,
    0xCD, 0xAF, 0xCA, 0xCC, 0xC9, 0x49, 0xD4, 0x37, 0xD5, 0x33, 0x50, 0xD0, 0x88, 0x30, 0x34, 0xB4,
    0x56, 0xF0, 0xC9, 0xCC, 0x2B, 0xAD, 0x50, 0xC8, 0x34, 0xB3, 0x30, 0xD3, 0x54, 0x70, 0x04, 0x06,
    0x49, 0x6A, 0x78, 0x6A, 0x92, 0x77, 0x66, 0x89, 0xBE, 0xA9, 0xB1, 0xB9, 0x9E, 0xA1, 0xA1, 0x82,
    0x86, 0xB7, 0x47, 0x88, 0xAF, 0x8F, 0x8E, 0x42, 0x4E, 0x66, 0x76, 0xAA, 0x82, 0x7B, 0x6A, 0x72,
    0x76, 0xBE, 0xA6, 0x82, 0x73, 0x06, 0xB0, 0x30, 0x4A, 0xD5, 0x37, 0x32, 0xD6, 0x33, 0xD0, 0x33,
    0x34, 0x32, 0x37, 0xD4, 0xB3, 0x34, 0x57, 0x08, 0x4E, 0x4C, 0x4B, 0x2C, 0xCA, 0x84, 0xEA, 0x62,
    0x60, 0x87, 0xC6, 0x09, 0x03, 0x07, 0x2C, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF
};
unsigned char c2s_SYN_STREAM2[] =
{
    0x80, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x62, 0xE0, 0x82, 0xC7, 0x0C, 0x33, 0x30, 0x8C, 0x07, 0x26, 0x74, 0xC9, 0x0B, 0x43,
    0x1E, 0x80, 0x00, 0xD2, 0x4F, 0x4B, 0x2C, 0xCB, 0x4C, 0xCE, 0xCF, 0xD3, 0xCB, 0x4C, 0xCE, 0x07,
    0x08, 0xA0, 0x41, 0x12, 0x9C, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF
};
unsigned char c2s_SYN_STREAM3[] =
{
    0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe8,
    0x80, 0x02, 0x00, 0x01, 0x01, 0x00, 0x01, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0xEA, 0xDF, 0xA2, 0x51, 0xB2, 0x62, 0xE0, 0x66, 0x60, 0x83, 0xA4, 0x17, 0x06,
    0x7B, 0xB8, 0x0B, 0x75, 0x30, 0x2C, 0xD6, 0xAE, 0x40, 0x17, 0xCD, 0xCD, 0xB1, 0x2E, 0xB4, 0x35,
    0xD0, 0xB3, 0xD4, 0xD1, 0xD2, 0xD7, 0x02, 0xB3, 0x2C, 0x18, 0xF8, 0x50, 0x73, 0x2C, 0x83, 0x9C,
    0x67, 0xB0, 0x3F, 0xD4, 0x3D, 0x3A, 0x60, 0x07, 0x81, 0xD5, 0x99, 0xEB, 0x40, 0xD4, 0x1B, 0x33,
    0xF0, 0xA3, 0xE5, 0x69, 0x06, 0x41, 0x90, 0x8B, 0x75, 0xA0, 0x4E, 0xD6, 0x29, 0x4E, 0x49, 0xCE,
    0x80, 0xAB, 0x81, 0x25, 0x03, 0x06, 0xBE, 0xD4, 0x3C, 0xDD, 0xD0, 0x60, 0x9D, 0xD4, 0x3C, 0xA8,
    0xA5, 0xBC, 0x28, 0x89, 0x8D, 0x81, 0x13, 0x20, 0x80, 0x72, 0x13, 0x2B, 0x74, 0x13, 0xD3, 0x53,
    0x6D, 0x0D, 0x00, 0x02, 0x88, 0x81, 0x05, 0x94, 0xFB, 0x19, 0xF8, 0x40, 0x49, 0x24, 0x07, 0xC4,
    0xB4, 0xB2, 0x04, 0x66, 0x3A, 0x06, 0xB6, 0x5C, 0x60, 0xA9, 0x93, 0x9F, 0xC2, 0xC0, 0xEC, 0xEE,
    0x1A, 0xC2, 0xC0, 0x56, 0x0C, 0xD4, 0x9B, 0x9B, 0x0A, 0x54, 0x5A, 0x52, 0x52, 0xC0, 0xC0, 0x0C,
    0x0A, 0x10, 0x46, 0x80, 0x00, 0xD2, 0x07, 0x08, 0x20, 0x06, 0x2E, 0x44, 0x2E, 0x66, 0x48, 0xF3,
    0xCD, 0xAF, 0xCA, 0xCC, 0xC9, 0x49, 0xD4, 0x37, 0xD5, 0x33, 0x50, 0xD0, 0x88, 0x30, 0x34, 0xB4,
    0x56, 0xF0, 0xC9, 0xCC, 0x2B, 0xAD, 0x50, 0xC8, 0x34, 0xB3, 0x30, 0xD3, 0x54, 0x70, 0x04, 0x06,
    0x49, 0x6A, 0x78, 0x6A, 0x92, 0x77, 0x66, 0x89, 0xBE, 0xA9, 0xB1, 0xB9, 0x9E, 0xA1, 0xA1, 0x82,
    0x86, 0xB7, 0x47, 0x88, 0xAF, 0x8F, 0x8E, 0x42, 0x4E, 0x66, 0x76, 0xAA, 0x82, 0x7B, 0x6A, 0x72,
    0x76, 0xBE, 0xA6, 0x82, 0x73, 0x06, 0xB0, 0x30, 0x4A, 0xD5, 0x37, 0x32, 0xD6, 0x33, 0xD0, 0x33,
    0x34, 0x32, 0x37, 0xD4, 0xB3, 0x34, 0x57, 0x08, 0x4E, 0x4C, 0x4B, 0x2C, 0xCA, 0x84, 0xEA, 0x62,
    0x60, 0x87, 0xC6, 0x09, 0x03, 0x07, 0x2C, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe8,
    0x80, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x62, 0xE0, 0x82, 0xC7, 0x0C, 0x33, 0x30, 0x8C, 0x07, 0x26, 0x74, 0xC9, 0x0B, 0x43,
    0x1E, 0x80, 0x00, 0xD2, 0x4F, 0x4B, 0x2C, 0xCB, 0x4C, 0xCE, 0xCF, 0xD3, 0xCB, 0x4C, 0xCE, 0x07,
    0x08, 0xA0, 0x41, 0x12, 0x9C, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF
};
unsigned char c2s_SYN_STREAM4[] =
{
    0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe8,
    0x80, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x62, 0xE0, 0x82, 0xC7, 0x0C, 0x33, 0x30, 0x8C, 0x07, 0x26, 0x74, 0xC9, 0x0B, 0x43,
    0x1E, 0x80, 0x00, 0xD2, 0x4F, 0x4B, 0x2C, 0xCB, 0x4C, 0xCE, 0xCF, 0xD3, 0xCB, 0x4C, 0xCE, 0x07,
    0x08, 0xA0, 0x41, 0x12, 0x9C, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF ,
    0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe8,
    0x80, 0x02, 0x00, 0x01, 0x01, 0x00, 0x01, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0xEA, 0xDF, 0xA2, 0x51, 0xB2, 0x62, 0xE0, 0x66, 0x60, 0x83, 0xA4, 0x17, 0x06,
    0x7B, 0xB8, 0x0B, 0x75, 0x30, 0x2C, 0xD6, 0xAE, 0x40, 0x17, 0xCD, 0xCD, 0xB1, 0x2E, 0xB4, 0x35,
    0xD0, 0xB3, 0xD4, 0xD1, 0xD2, 0xD7, 0x02, 0xB3, 0x2C, 0x18, 0xF8, 0x50, 0x73, 0x2C, 0x83, 0x9C,
    0x67, 0xB0, 0x3F, 0xD4, 0x3D, 0x3A, 0x60, 0x07, 0x81, 0xD5, 0x99, 0xEB, 0x40, 0xD4, 0x1B, 0x33,
    0xF0, 0xA3, 0xE5, 0x69, 0x06, 0x41, 0x90, 0x8B, 0x75, 0xA0, 0x4E, 0xD6, 0x29, 0x4E, 0x49, 0xCE,
    0x80, 0xAB, 0x81, 0x25, 0x03, 0x06, 0xBE, 0xD4, 0x3C, 0xDD, 0xD0, 0x60, 0x9D, 0xD4, 0x3C, 0xA8,
    0xA5, 0xBC, 0x28, 0x89, 0x8D, 0x81, 0x13, 0x20, 0x80, 0x72, 0x13, 0x2B, 0x74, 0x13, 0xD3, 0x53,
    0x6D, 0x0D, 0x00, 0x02, 0x88, 0x81, 0x05, 0x94, 0xFB, 0x19, 0xF8, 0x40, 0x49, 0x24, 0x07, 0xC4,
    0xB4, 0xB2, 0x04, 0x66, 0x3A, 0x06, 0xB6, 0x5C, 0x60, 0xA9, 0x93, 0x9F, 0xC2, 0xC0, 0xEC, 0xEE,
    0x1A, 0xC2, 0xC0, 0x56, 0x0C, 0xD4, 0x9B, 0x9B, 0x0A, 0x54, 0x5A, 0x52, 0x52, 0xC0, 0xC0, 0x0C,
    0x0A, 0x10, 0x46, 0x80, 0x00, 0xD2, 0x07, 0x08, 0x20, 0x06, 0x2E, 0x44, 0x2E, 0x66, 0x48, 0xF3,
    0xCD, 0xAF, 0xCA, 0xCC, 0xC9, 0x49, 0xD4, 0x37, 0xD5, 0x33, 0x50, 0xD0, 0x88, 0x30, 0x34, 0xB4,
    0x56, 0xF0, 0xC9, 0xCC, 0x2B, 0xAD, 0x50, 0xC8, 0x34, 0xB3, 0x30, 0xD3, 0x54, 0x70, 0x04, 0x06,
    0x49, 0x6A, 0x78, 0x6A, 0x92, 0x77, 0x66, 0x89, 0xBE, 0xA9, 0xB1, 0xB9, 0x9E, 0xA1, 0xA1, 0x82,
    0x86, 0xB7, 0x47, 0x88, 0xAF, 0x8F, 0x8E, 0x42, 0x4E, 0x66, 0x76, 0xAA, 0x82, 0x7B, 0x6A, 0x72,
    0x76, 0xBE, 0xA6, 0x82, 0x73, 0x06, 0xB0, 0x30, 0x4A, 0xD5, 0x37, 0x32, 0xD6, 0x33, 0xD0, 0x33,
    0x34, 0x32, 0x37, 0xD4, 0xB3, 0x34, 0x57, 0x08, 0x4E, 0x4C, 0x4B, 0x2C, 0xCA, 0x84, 0xEA, 0x62,
    0x60, 0x87, 0xC6, 0x09, 0x03, 0x07, 0x2C, 0xAA, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF

};
TEST(spdyzlibfilter_test)
{
//     SpdyConnection MySpdyConn(NULL, NULL, 2);
//     MySpdyConn.onReadEx(c2s_SYN_STREAM3, sizeof(c2s_SYN_STREAM3));

    AutoBuf Result;
    int ntotal = 0;
//     char Result.begin()[2000];
    char pData[2000];
    LoopBuf Result1(10);

    Result1.clear();
    int lpbuffSize = Result1.available();
    //int lpbuffSizeC = Result1.contiguous();
    Result1.append(pData, lpbuffSize - 8);
    //int lpbuffSize1 = Result1.available();
    //int lpbuffSizeC1 = Result1.contiguous();
    int DataSize = Result1.size();
    char  *pResult1 = Result1.end();
    printf("This is spdyzlibfilter_test!\n");
    SpdyZlibFilter TestInflator;
    SpdyZlibFilter TestInflator1;
    SpdyZlibFilter TestDeflator;
    TestInflator.init(1, 1);
    TestInflator1.init(1, 1);
    TestDeflator.init(0, 1);
    Result.clear();
    int n = TestInflator.decompress((char *)c2s_SYN_STREAM1 + 18,
                                    sizeof(c2s_SYN_STREAM1) - 18, Result);
    CHECK(n > 0);
    printbuff((unsigned char *)Result.begin(), n);
    printheader((unsigned char *)Result.begin(), n);
    int n1 = TestDeflator.compress(Result.begin(), 50, &Result1, 0);
    CHECK(n1 >= 0);
    ntotal += n1;
    printf("finished Compressing 0-50 n1=%d, total=%d\n", n1, ntotal);
    n1 = TestDeflator.compress(Result.begin() + 50, 200, &Result1, 0);
    CHECK(n1 >= 0);
    ntotal += n1;
    printf("finished Compressing 50-250 n1=%d, total=%d\n", n1, ntotal);
    n1 = TestDeflator.compress(Result.begin() + 250, n - 250, &Result1,
                               Z_SYNC_FLUSH);
    CHECK(n1 >= 0);
    ntotal += n1;
    printf("finished Compressing 250-%d n1=%d, total=%d\n", n, n1, ntotal);
    printf("UncompressLen=%d, middle=%d, CompressedLen=%d\n", n, n1, ntotal);
    Result.clear();
    pResult1 = Result1.getPointer(DataSize);
    n = TestInflator1.decompress(pResult1, ntotal,  Result);
    CHECK(n > 0);
    printbuff((unsigned char *)Result.begin(), n);
    printheader((unsigned char *)Result.begin(), n);
    Result.clear();
    n = TestInflator.decompress((char *)c2s_SYN_STREAM2 + 18,
                                sizeof(c2s_SYN_STREAM2) - 18, Result);
    CHECK(n > 0);
    printbuff((unsigned char *)Result.begin(), n);
    printheader((unsigned char *)Result.begin(), n);
    Result1.clear();
    if (n > 0)
    {
        n1 = TestDeflator.compress(Result.begin(), n, &Result1, Z_SYNC_FLUSH);
        CHECK(n1 > 0);
        printf("UncompressLen=%d, CompressedLen=%d\n", n, n1);
        if (n1 > 0)
        {
            Result.clear();
            pResult1 = Result1.getPointer(0);
            n = TestInflator1.decompress(pResult1, n1,  Result);
            CHECK(n > 0);
            printbuff((unsigned char *)Result.begin(), n);
            printheader((unsigned char *)Result.begin(), n);
        }
    }
}


#endif
