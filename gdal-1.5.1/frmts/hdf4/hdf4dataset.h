/******************************************************************************
 * $Id: hdf4dataset.h 12402 2007-10-13 16:37:17Z dron $
 *
 * Project:  Hierarchical Data Format Release 4 (HDF4)
 * Purpose:  Header file for HDF4 datasets reader.
 * Author:   Andrey Kiselev, dron@remotesensing.org
 *
 ******************************************************************************
 * Copyright (c) 2002, Andrey Kiselev <dron@remotesensing.org>
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

#ifndef _HDF4DATASET_H_INCLUDED_
#define _HDF4DATASET_H_INCLUDED_

#include "cpl_list.h"
#include "gdal_pam.h"

typedef enum			// Types of dataset:
{
    HDF4_SDS,			// Scientific Dataset
    HDF4_GR,			// General Raster Image
    HDF4_EOS,                   // HDF EOS
    HDF4_UNKNOWN
} HDF4DatasetType;

typedef enum			// Types of data products:
{
    GDAL_HDF4,			// HDF written by GDAL
    EOS_GRID,                   // HDF-EOS Grid
    EOS_SWATH,                  // HDF-EOS Swath
    EOS_SWATH_GEOL,             // HDF-EOS Swath Geolocation Array
    SEAWIFS_L1A,		// SeaWiFS Level-1A Data
    SEAWIFS_L2,			// SeaWiFS Level-2 Data
    SEAWIFS_L3,			// SeaWiFS Level-3 Standard Mapped Image
    HYPERION_L1,                // Hyperion L1 Data Product
    UNKNOWN
} HDF4SubdatasetType;

/************************************************************************/
/* ==================================================================== */
/*				HDF4Dataset				*/
/* ==================================================================== */
/************************************************************************/

class HDF4Dataset : public GDALPamDataset
{

  private:

    int         bIsHDFEOS;

    static char **HDF4EOSTokenizeAttrs( const char *pszString );
    static char **HDF4EOSGetObject( char **papszAttrList, char **ppszAttrName,
                                    char **ppszAttrValue );
     
  protected:

    FILE	*fp;
    int32	hHDF4, hSD, hGR;
    int32	nDatasets, nImages;
    HDF4DatasetType iDatasetType;
    HDF4SubdatasetType iSubdatasetType;
    const char	*pszSubdatasetType;

    char	**papszGlobalMetadata;
    char	**papszSubDatasets;

    CPLErr      ReadGlobalAttributes( int32 );

    static GDALDataType GetDataType( int32 ) ;
    static const char *GetDataTypeName( int32 );
    static int  GetDataTypeSize( int32 );
    static double AnyTypeToDouble( int32, void * );
    static char **TranslateHDF4Attributes( int32, int32, char *,
                                           int32, int32, char ** );
    static char ** TranslateHDF4EOSAttributes( int32, int32, int32,
                                               char ** );

  public:
                HDF4Dataset();
		~HDF4Dataset();
    
    virtual char **GetMetadata( const char * pszDomain = "" );
    static GDALDataset *Open( GDALOpenInfo * );
    static int          Identify( GDALOpenInfo * );
};

char *SPrintArray( GDALDataType eDataType, const void *paDataArray,
                   int nValues, const char *pszDelimiter );


#endif /* _HDF4DATASET_H_INCLUDED_ */

