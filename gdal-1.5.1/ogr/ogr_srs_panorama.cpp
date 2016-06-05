/******************************************************************************
 * $Id: ogr_srs_panorama.cpp 13279 2007-12-07 18:13:26Z dron $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  OGRSpatialReference translation to/from "Panorama" GIS
 *           georeferencing information (also know as GIS "Integration").
 * Author:   Andrey Kiselev, dron@remotesensing.org
 *
 ******************************************************************************
 * Copyright (c) 2005, Andrey Kiselev <dron@remotesensing.org>
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

#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_csv.h"

CPL_CVSID("$Id: ogr_srs_panorama.cpp 13279 2007-12-07 18:13:26Z dron $");

#define TO_DEGREES (180.0 / 3.14159265358979323846)
#define TO_RADIANS (3.14159265358979323846 / 180.0)

// XXX: this macro computes zone number from the central meridian parameter.
// Note, that "Panorama" parameters are set in radians.
// In degrees it means formulae:
//
//              zone = (central_meridian + 3) / 6
//
#define TO_ZONE(x) (((x) + 0.052359877559829883) / 0.10471975511965977)

/************************************************************************/
/*  "Panorama" projection codes.                                        */
/************************************************************************/

#define PAN_PROJ_NONE   -1L
#define PAN_PROJ_TM     1L      // Gauss-Kruger (Transverse Mercator)
#define PAN_PROJ_LCC    2L      // Lambert Conformal Conic 2SP
#define PAN_PROJ_LAEA   4L      // Lambert Azimuthal Equal Area
#define PAN_PROJ_STEREO 5L      // Stereographic
#define PAN_PROJ_AE     6L      // Azimuthal Equidistant (Postel)
#define PAN_PROJ_MERCAT 8L      // Mercator
#define PAN_PROJ_POLYC  11L     // Polyconic
#define PAN_PROJ_PS     13L     // Polar Stereographic
#define PAN_PROJ_GNOMON 15L     // Gnomonic
#define PAN_PROJ_UTM    17L     // Universal Transverse Mercator (PAN_PROJ_UTM)
#define PAN_PROJ_MOLL   19L     // Mollweide
#define PAN_PROJ_EC     20L     // Equidistant Conic


/************************************************************************/
/*  "Panorama" datum codes.                                             */
/************************************************************************/

#define PAN_DATUM_NONE      -1L
#define PAN_DATUM_PULKOVO42 1L  // Pulkovo 1942
#define PAN_DATUM_WGS84     2L  // WGS84

/************************************************************************/
/*  "Panorama" ellipsod codes.                                          */
/************************************************************************/

#define PAN_ELLIPSOID_NONE          0L
#define PAN_ELLIPSOID_KRASSOVSKY    1L  // Krassovsky, 1940
#define PAN_ELLIPSOID_WGS72         2L  // WGS, 1972
#define PAN_ELLIPSOID_INT1924       3L  // International, 1924 (Hayford, 1909)
#define PAN_ELLIPSOID_CLARCKE1880   4L  // Clarke, 1880
#define PAN_ELLIPSOID_CLARCKE1866   5L  // Clarke, 1866 (NAD1927)
#define PAN_ELLIPSOID_EVEREST1830   6L  // Everest, 1830
#define PAN_ELLIPSOID_BESSEL1841    7L  // Bessel, 1841
#define PAN_ELLIPSOID_AIRY1830      8L  // Airy, 1830
#define PAN_ELLIPSOID_WGS84         9L  // WGS, 1984 (GPS)

/************************************************************************/
/*  Correspondence between "Panorama" and EPSG datum codes.             */
/************************************************************************/

static const long aoDatums[] =
{
    0,
    4284,   // Pulkovo, 1942
    4326    // WGS, 1984
};

#define NUMBER_OF_DATUMS        (long)(sizeof(aoDatums)/sizeof(aoDatums[0]))

/************************************************************************/
/*  Correspondence between "Panorama" and EPSG ellipsoid codes.         */
/************************************************************************/

static const long aoEllips[] =
{
    0,
    7024,   // Krassovsky, 1940
    7043,   // WGS, 1972
    7022,   // International, 1924 (Hayford, 1909)
    7034,   // Clarke, 1880
    7008,   // Clarke, 1866 (NAD1927)
    7015,   // Everest, 1830
    7004,   // Bessel, 1841
    7001,   // Airy, 1830
    7030    // WGS, 1984 (GPS)
};

#define NUMBER_OF_ELLIPSOIDS    (long)(sizeof(aoEllips)/sizeof(aoEllips[0]))

/************************************************************************/
/*                    PanoramaGetUOMLengthInfo()                        */
/*                                                                      */
/*      Note: This function should eventually also know how to          */
/*      lookup length aliases in the UOM_LE_ALIAS table.                */
/************************************************************************/

static int 
PanoramaGetUOMLengthInfo( int nUOMLengthCode, char **ppszUOMName,
                          double * pdfInMeters )

{
    char        **papszUnitsRecord;
    char        szSearchKey[24];
    int         iNameField;

#define UOM_FILENAME CSVFilename( "unit_of_measure.csv" )

/* -------------------------------------------------------------------- */
/*      We short cut meter to save work in the most common case.        */
/* -------------------------------------------------------------------- */
    if( nUOMLengthCode == 9001 )
    {
        if( ppszUOMName != NULL )
            *ppszUOMName = CPLStrdup( "metre" );
        if( pdfInMeters != NULL )
            *pdfInMeters = 1.0;

        return TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Search the units database for this unit.  If we don't find      */
/*      it return failure.                                              */
/* -------------------------------------------------------------------- */
    sprintf( szSearchKey, "%d", nUOMLengthCode );
    papszUnitsRecord =
        CSVScanFileByName( UOM_FILENAME, "UOM_CODE", szSearchKey, CC_Integer );

    if( papszUnitsRecord == NULL )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Get the name, if requested.                                     */
/* -------------------------------------------------------------------- */
    if( ppszUOMName != NULL )
    {
        iNameField = CSVGetFileFieldId( UOM_FILENAME, "UNIT_OF_MEAS_NAME" );
        *ppszUOMName = CPLStrdup( CSLGetField(papszUnitsRecord, iNameField) );
    }
    
/* -------------------------------------------------------------------- */
/*      Get the A and B factor fields, and create the multiplicative    */
/*      factor.                                                         */
/* -------------------------------------------------------------------- */
    if( pdfInMeters != NULL )
    {
        int     iBFactorField, iCFactorField;
        
        iBFactorField = CSVGetFileFieldId( UOM_FILENAME, "FACTOR_B" );
        iCFactorField = CSVGetFileFieldId( UOM_FILENAME, "FACTOR_C" );

        if( atof(CSLGetField(papszUnitsRecord, iCFactorField)) > 0.0 )
            *pdfInMeters = atof(CSLGetField(papszUnitsRecord, iBFactorField))
                / atof(CSLGetField(papszUnitsRecord, iCFactorField));
        else
            *pdfInMeters = 0.0;
    }
    
    return( TRUE );
}

/************************************************************************/
/*                     PanoramaGetEllipsoidInfo()                       */
/*                                                                      */
/*      Fetch info about an ellipsoid.  Axes are always returned in     */
/*      meters.  SemiMajor computed based on inverse flattening         */
/*      where that is provided.                                         */
/************************************************************************/

static int 
PanoramaGetEllipsoidInfo( int nCode, char ** ppszName,
                          double * pdfSemiMajor, double * pdfInvFlattening )

{
    char        szSearchKey[24];
    double      dfSemiMajor, dfToMeters = 1.0;
    int         nUOMLength;
    
/* -------------------------------------------------------------------- */
/*      Get the semi major axis.                                        */
/* -------------------------------------------------------------------- */
    sprintf( szSearchKey, "%d", nCode );

    dfSemiMajor =
        atof(CSVGetField( CSVFilename("ellipsoid.csv" ),
                          "ELLIPSOID_CODE", szSearchKey, CC_Integer,
                          "SEMI_MAJOR_AXIS" ) );
    if( dfSemiMajor == 0.0 )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Get the translation factor into meters.                         */
/* -------------------------------------------------------------------- */
    nUOMLength = atoi(CSVGetField( CSVFilename("ellipsoid.csv" ),
                                   "ELLIPSOID_CODE", szSearchKey, CC_Integer,
                                   "UOM_CODE" ));
    PanoramaGetUOMLengthInfo( nUOMLength, NULL, &dfToMeters );

    dfSemiMajor *= dfToMeters;
    
    if( pdfSemiMajor != NULL )
        *pdfSemiMajor = dfSemiMajor;
    
/* -------------------------------------------------------------------- */
/*      Get the semi-minor if requested.  If the Semi-minor axis        */
/*      isn't available, compute it based on the inverse flattening.    */
/* -------------------------------------------------------------------- */
    if( pdfInvFlattening != NULL )
    {
        *pdfInvFlattening = 
            atof(CSVGetField( CSVFilename("ellipsoid.csv" ),
                              "ELLIPSOID_CODE", szSearchKey, CC_Integer,
                              "INV_FLATTENING" ));

        if( *pdfInvFlattening == 0.0 )
        {
            double dfSemiMinor;

            dfSemiMinor =
                atof(CSVGetField( CSVFilename("ellipsoid.csv" ),
                                  "ELLIPSOID_CODE", szSearchKey, CC_Integer,
                                  "SEMI_MINOR_AXIS" )) * dfToMeters;

            if( dfSemiMajor != 0.0 && dfSemiMajor != dfSemiMinor )
                *pdfInvFlattening = 
                    -1.0 / (dfSemiMinor/dfSemiMajor - 1.0);
            else
                *pdfInvFlattening = 0.0;
        }
    }

/* -------------------------------------------------------------------- */
/*      Get the name, if requested.                                     */
/* -------------------------------------------------------------------- */
    if( ppszName != NULL )
        *ppszName =
            CPLStrdup(CSVGetField( CSVFilename("ellipsoid.csv" ),
                                   "ELLIPSOID_CODE", szSearchKey, CC_Integer,
                                   "ELLIPSOID_NAME" ));
    
    return( TRUE );
}

/************************************************************************/
/*                        OSRImportFromPanorama()                       */
/************************************************************************/

OGRErr OSRImportFromPanorama( OGRSpatialReferenceH hSRS,
                              long iProjSys, long iDatum, long iEllips,
                              double *padfPrjParams )

{
    return ((OGRSpatialReference *) hSRS)->importFromPanorama( iProjSys,
                                                               iDatum,iEllips,
                                                               padfPrjParams );
}

/************************************************************************/
/*                          importFromPanorama()                        */
/************************************************************************/

/**
 * Import coordinate system from "Panorama" GIS projection definition.
 *
 * This method will import projection definition in style, used by
 * "Panorama" GIS.
 *
 * This function is the equivalent of the C function OSRImportFromPanorama().
 *
 * @param iProjSys Input projection system code, used in GIS "Panorama".
 *
 *      <h4>Supported Projections</h4>
 * <pre>
 *      1:  Gauss-Kruger (Transverse Mercator)
 *      4:  Lambert Azimuthal Equal Area
 *      5:  Stereographic
 *      6:  Azimuthal Equidistant (Postel)
 *      8:  Mercator
 *      11: Polyconic
 *      13: Polar Stereographic
 *      15: Gnomonic
 *      17: Universal Transverse Mercator (PAN_PROJ_UTM)
 *      19: Mollweide
 *      20: Equidistant Conic
 * </pre>
 *
 * @param iDatum Input coordinate system.
 *
 *      <h4>Supported Datums</h4>
 * <pre>
 *       1: Pulkovo, 1942
 *       2: WGS, 1984
 * </pre>
 *
 * @param iEllips Input spheroid.
 * 
 *      <h4>Supported Spheroids</h4>
 * <pre>
 *       1: Krassovsky, 1940
 *       2: WGS, 1972
 *       3: International, 1924 (Hayford, 1909)
 *       4: Clarke, 1880
 *       5: Clarke, 1866 (NAD1927)
 *       6: Everest, 1830
 *       7: Bessel, 1841
 *       8: Airy, 1830
 *       9: WGS, 1984 (GPS)
 * </pre>
 *
 * @param padfPrjParams Array of 7 coordinate system parameters:
 *
 * <pre>
 *      [0]  Latitude of the first standard parallel (radians)
 *      [1]  Latitude of the second standard parallel (radians)
 *      [2]  Latitude of center of projection (radians)
 *      [3]  Longitude of center of projection (radians)
 *      [4]  Scaling factor
 *      [5]  False Easting
 *      [6]  False Northing
 * </pre>
 *
 * Particular projection uses different parameters, unused ones may be set to
 * zero. If NULL supplied instead of array pointer default values will be used
 * (i.e., zeroes).
 *
 * @return OGRERR_NONE on success or an error code in case of failure. 
 */

OGRErr OGRSpatialReference::importFromPanorama( long iProjSys, long iDatum,
                                                long iEllips,
                                                double *padfPrjParams )

{
/* -------------------------------------------------------------------- */
/*      Use safe defaults if projection parameters are not supplied.    */
/* -------------------------------------------------------------------- */
    int     bProjAllocated = FALSE;

    if( padfPrjParams == NULL )
    {
        int     i;

        padfPrjParams = (double *)CPLMalloc( 7 * sizeof(double) );
        if ( !padfPrjParams )
            return OGRERR_NOT_ENOUGH_MEMORY;
        for ( i = 0; i < 7; i++ )
            padfPrjParams[i] = 0.0;
        bProjAllocated = TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Operate on the basis of the projection code.                    */
/* -------------------------------------------------------------------- */
    switch ( iProjSys )
    {
        case PAN_PROJ_NONE:
            break;

        case PAN_PROJ_UTM:
            {
                long nZone = (long)TO_ZONE(padfPrjParams[3]);
                // XXX: no way to determine south hemisphere. Always assume
                // nothern hemisphere.
                SetUTM( nZone, TRUE );
            }
            break;

        case PAN_PROJ_MERCAT:
            SetMercator( TO_DEGREES * padfPrjParams[2],
                         TO_DEGREES * padfPrjParams[3],
                         padfPrjParams[4],
                         padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_PS:
            SetPS( TO_DEGREES * padfPrjParams[2],
                   TO_DEGREES * padfPrjParams[3],
                   padfPrjParams[4],
                   padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_POLYC:
            SetPolyconic( TO_DEGREES * padfPrjParams[2],
                          TO_DEGREES * padfPrjParams[3],
                          padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_EC:
            SetEC( TO_DEGREES * padfPrjParams[0],
                   TO_DEGREES * padfPrjParams[1],
                   TO_DEGREES * padfPrjParams[2],
                   TO_DEGREES * padfPrjParams[3],
                   padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_LCC:
            SetLCC( TO_DEGREES * padfPrjParams[0],
                    TO_DEGREES * padfPrjParams[1],
                    TO_DEGREES * padfPrjParams[2],
                    TO_DEGREES * padfPrjParams[3],
                    padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_TM:
            {
                // XXX: we need zone number to compute false easting
                // parameter, because usually it is not contained in the
                // "Panorama" projection definition.
                // FIXME: what to do with negative values?
                long nZone = (long)TO_ZONE(padfPrjParams[3]);
                padfPrjParams[5] = nZone * 1000000.0 + 500000.0;
                padfPrjParams[4] = 1.0;
                SetTM( TO_DEGREES * padfPrjParams[2],
                       TO_DEGREES * padfPrjParams[3],
                       padfPrjParams[4],
                       padfPrjParams[5], padfPrjParams[6] );
            }
            break;

        case PAN_PROJ_STEREO:
            SetStereographic( TO_DEGREES * padfPrjParams[2],
                              TO_DEGREES * padfPrjParams[3],
                              padfPrjParams[4],
                              padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_LAEA:
            SetLAEA( TO_DEGREES * padfPrjParams[2],
                     TO_DEGREES * padfPrjParams[3],
                     padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_AE:
            SetAE( TO_DEGREES * padfPrjParams[2],
                   TO_DEGREES * padfPrjParams[3],
                   padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_GNOMON:
            SetGnomonic( TO_DEGREES * padfPrjParams[2],
                         TO_DEGREES * padfPrjParams[3],
                         padfPrjParams[5], padfPrjParams[6] );
            break;

        case PAN_PROJ_MOLL:
            SetMollweide( TO_DEGREES * padfPrjParams[3],
                          padfPrjParams[5], padfPrjParams[6] );
            break;

        default:
            CPLDebug( "OSR_Panorama", "Unsupported projection: %d", iProjSys );
            SetLocalCS( CPLString().Printf("\"Panorama\" projection number %d",
                                   iProjSys) );
            break;
            
    }

/* -------------------------------------------------------------------- */
/*      Try to translate the datum/spheroid.                            */
/* -------------------------------------------------------------------- */

    if ( !IsLocal() )
    {
        if ( iDatum > 0 && iDatum < NUMBER_OF_DATUMS && aoDatums[iDatum] )
        {
            OGRSpatialReference oGCS;
            oGCS.importFromEPSG( aoDatums[iDatum] );
            CopyGeogCSFrom( &oGCS );
        }

        else if ( iEllips > 0
                  && iEllips < NUMBER_OF_ELLIPSOIDS
                  && aoEllips[iEllips] )
        {
            char    *pszName = NULL;
            double  dfSemiMajor, dfInvFlattening;

            if( PanoramaGetEllipsoidInfo( aoEllips[iEllips],
                                          &pszName,
                                          &dfSemiMajor, &dfInvFlattening ) )
            {
                SetGeogCS( CPLString().Printf("Unknown datum based upon the %s ellipsoid",
                                      pszName ),
                           CPLString().Printf( "Not specified (based on %s spheroid)",
                                       pszName ),
                           pszName, dfSemiMajor, dfInvFlattening,
                           NULL, 0.0, NULL, 0.0 );
                SetAuthority( "SPHEROID", "EPSG", aoEllips[iEllips] );
            }
            else
            {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Failed to lookup ellipsoid code %d, likely due to"
                          " missing GDAL gcs.csv\n"
                          " file.  Falling back to use WGS84.", iEllips );
                SetWellKnownGeogCS( "WGS84" );
            }

            if ( pszName )
                CPLFree( pszName );
        }
        
        else
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Wrong datum code %d. Supported datums are 1--%d only.\n"
                      "Setting WGS84 as a fallback.",
                      iDatum, NUMBER_OF_DATUMS - 1 );
            SetWellKnownGeogCS( "WGS84" );
        }
    }

/* -------------------------------------------------------------------- */
/*      Grid units translation                                          */
/* -------------------------------------------------------------------- */
    if( IsLocal() || IsProjected() )
        SetLinearUnits( SRS_UL_METER, 1.0 );

    FixupOrdering();

    if ( bProjAllocated && padfPrjParams )
        CPLFree( padfPrjParams );

    return OGRERR_NONE;
}

/************************************************************************/
/*                      OSRExportToPanorama()                           */
/************************************************************************/

OGRErr OSRExportToPanorama( OGRSpatialReferenceH hSRS,
                            long *piProjSys, long *piDatum, long *piEllips,
                            long *piZone, double *padfPrjParams )

{
    return ((OGRSpatialReference *) hSRS)->exportToPanorama( piProjSys,
                                                             piDatum, piEllips,
                                                             piZone,
                                                             padfPrjParams );
}

/************************************************************************/
/*                           exportToPanorama()                         */
/************************************************************************/

/**
 * Export coordinate system in "Panorama" GIS projection definition.
 *
 * This method is the equivalent of the C function OSRExportToPanorama().
 *
 * @param piProjSys Pointer to variable, where the projection system code will
 * be returned.
 *
 * @param piDatum Pointer to variable, where the coordinate system code will
 * be returned.
 *
 * @param piEllips Pointer to variable, where the spheroid code will be
 * returned.
 * 
 * @param piZone Pointer to variable, where the zone for PAN_PROJ_UTM projection
 * system will be returned.
 *
 * @param padfPrjParams an existing 7 double buffer into which the
 * projection parameters will be placed. See importFromPanorama()
 * for the list of parameters.
 * 
 * @return OGRERR_NONE on success or an error code on failure. 
 */

OGRErr OGRSpatialReference::exportToPanorama( long *piProjSys, long *piDatum,
                                              long *piEllips, long *piZone,
                                              double *padfPrjParams ) const

{
    CPLAssert( padfPrjParams );

    const char  *pszProjection = GetAttrValue("PROJECTION");

/* -------------------------------------------------------------------- */
/*      Fill all projection parameters with zero.                       */
/* -------------------------------------------------------------------- */
    int     i;

    *piDatum = 0L;
    *piEllips = 0L;
    *piZone = 0L;
    for ( i = 0; i < 7; i++ )
        padfPrjParams[i] = 0.0;

/* ==================================================================== */
/*      Handle the projection definition.                               */
/* ==================================================================== */
    if( IsLocal() )
        *piProjSys = PAN_PROJ_NONE;

    else if( pszProjection == NULL )
    {
#ifdef DEBUG
        CPLDebug( "OSR_Panorama",
                  "Empty projection definition, considered as Geographic" );
#endif
        *piProjSys = PAN_PROJ_NONE;
    }

    else if( EQUAL(pszProjection, SRS_PT_MERCATOR_1SP) )
    {
        *piProjSys = PAN_PROJ_MERCAT;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[4] = GetNormProjParm( SRS_PP_SCALE_FACTOR, 1.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_POLAR_STEREOGRAPHIC) )
    {
        *piProjSys = PAN_PROJ_PS;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[4] = GetNormProjParm( SRS_PP_SCALE_FACTOR, 1.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_POLYCONIC) )
    {
        *piProjSys = PAN_PROJ_POLYC;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_EQUIDISTANT_CONIC) )
    {
        *piProjSys = PAN_PROJ_EC;
        padfPrjParams[0] =
            TO_RADIANS * GetNormProjParm( SRS_PP_STANDARD_PARALLEL_1, 0.0 );
        padfPrjParams[1] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_STANDARD_PARALLEL_2, 0.0 );
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP) )
    {
        *piProjSys = PAN_PROJ_LCC;
        padfPrjParams[0] =
            TO_RADIANS * GetNormProjParm( SRS_PP_STANDARD_PARALLEL_1, 0.0 );
        padfPrjParams[1] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_STANDARD_PARALLEL_2, 0.0 );
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_TRANSVERSE_MERCATOR) )
    {
        int bNorth;

        *piZone = GetUTMZone( &bNorth );

        if( *piZone != 0 )
        {
            *piProjSys = PAN_PROJ_UTM;
            if( !bNorth )
                *piZone = - *piZone;
        }            
        else
        {
            *piProjSys = PAN_PROJ_TM;
            padfPrjParams[3] =
                TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
            padfPrjParams[2] = 
                TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
            padfPrjParams[4] =
                GetNormProjParm( SRS_PP_SCALE_FACTOR, 1.0 );
            padfPrjParams[5] =
                GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
            padfPrjParams[6] =
                GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
        }
    }

    else if( EQUAL(pszProjection, SRS_PT_STEREOGRAPHIC) )
    {
        *piProjSys = PAN_PROJ_STEREO;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[4] = GetNormProjParm( SRS_PP_SCALE_FACTOR, 1.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_LAMBERT_AZIMUTHAL_EQUAL_AREA) )
    {
        *piProjSys = PAN_PROJ_LAEA;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_AZIMUTHAL_EQUIDISTANT) )
    {
        *piProjSys = PAN_PROJ_AE;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_LONGITUDE_OF_CENTER, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_CENTER, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_GNOMONIC) )
    {
        *piProjSys = PAN_PROJ_GNOMON;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[2] = 
            TO_RADIANS * GetNormProjParm( SRS_PP_LATITUDE_OF_ORIGIN, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    else if( EQUAL(pszProjection, SRS_PT_MOLLWEIDE) )
    {
        *piProjSys = PAN_PROJ_MOLL;
        padfPrjParams[3] =
            TO_RADIANS * GetNormProjParm( SRS_PP_CENTRAL_MERIDIAN, 0.0 );
        padfPrjParams[5] = GetNormProjParm( SRS_PP_FALSE_EASTING, 0.0 );
        padfPrjParams[6] = GetNormProjParm( SRS_PP_FALSE_NORTHING, 0.0 );
    }

    // Projection unsupported by "Panorama" GIS
    else
    {
        CPLDebug( "OSR_Panorama",
                  "Projection \"%s\" unsupported by \"Panorama\" GIS. "
                  "Geographic system will be used.", pszProjection );
        *piProjSys = PAN_PROJ_NONE;
    }
 
/* -------------------------------------------------------------------- */
/*      Translate the datum.                                            */
/* -------------------------------------------------------------------- */
    const char  *pszDatum = GetAttrValue( "DATUM" );

    if ( pszDatum == NULL )
    {
        *piDatum = PAN_DATUM_NONE;
        *piEllips = PAN_ELLIPSOID_NONE;
    }
    else if ( EQUAL( pszDatum, "Pulkovo_1942" ) )
    {
        *piDatum = PAN_DATUM_PULKOVO42;
        *piEllips = PAN_ELLIPSOID_KRASSOVSKY;
    }
    else if( EQUAL( pszDatum, SRS_DN_WGS84 ) )
    {
        *piDatum = PAN_DATUM_WGS84;
        *piEllips = PAN_ELLIPSOID_WGS84;
    }

    // If not found well known datum, translate ellipsoid
    else
    {
        double      dfSemiMajor = GetSemiMajor();
        double      dfInvFlattening = GetInvFlattening();
        int         i;

#ifdef DEBUG
        CPLDebug( "OSR_Panorama",
                  "Datum \"%s\" unsupported by \"Panorama\" GIS. "
                  "Try to translate ellipsoid definition.", pszDatum );
#endif
        
        for ( i = 0; i < NUMBER_OF_ELLIPSOIDS && aoEllips[i]; i++ )
        {
            double  dfSM = 0.0;
            double  dfIF = 1.0;

            PanoramaGetEllipsoidInfo( aoEllips[i], NULL, &dfSM, &dfIF );
            if( CPLIsEqual(dfSemiMajor, dfSM)
                && CPLIsEqual(dfInvFlattening, dfIF) )
            {
                *piEllips = i;
                break;
            }
        }

        if ( i == NUMBER_OF_ELLIPSOIDS )    // Didn't found matches.
        {
#ifdef DEBUG
            CPLDebug( "OSR_Panorama",
                      "Ellipsoid \"%s\" unsupported by \"Panorama\" GIS.",
                      pszDatum );
#endif
            *piEllips = 0;
        }
    }

    return OGRERR_NONE;
}

