/******************************************************************************
 * $Id: ceostest.c 10645 2007-01-18 02:22:39Z warmerdam $
 *
 * Project:  CEOS Translator
 * Purpose:  Test mainline.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "ceosopen.h"

/************************************************************************/
/*                                main()                                */
/************************************************************************/

int main( int nArgc, char ** papszArgv )

{
    const char  *pszFilename;
    FILE	*fp;
    CEOSRecord *psRecord;
    int nPosition = 0;

    if( nArgc > 1 )
        pszFilename = papszArgv[1];
    else
        pszFilename = "imag_01.dat";

    fp = VSIFOpen( pszFilename, "rb" );
    if( fp == NULL )
    {
        fprintf( stderr, "Can't open %s at all.\n", pszFilename );
        exit( 1 );
    }

    while( !VSIFEof(fp) 
           && (psRecord = CEOSReadRecord( fp )) != NULL )
    {
        printf( "%9d:%4d:%8x:%d\n", 
                nPosition, psRecord->nRecordNum, 
                psRecord->nRecordType, psRecord->nLength );
        CEOSDestroyRecord( psRecord );

        nPosition = (int) VSIFTell( fp );
    }
    VSIFClose( fp );

    exit( 0 );
}
