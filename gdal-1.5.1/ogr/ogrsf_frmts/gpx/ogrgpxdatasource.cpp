/******************************************************************************
 * $Id: ogrgpxdatasource.cpp
 *
 * Project:  GPX Translator
 * Purpose:  Implements OGRGPXDataSource class
 * Author:   Even Rouault, even dot rouault at mines dash paris dot org
 *
 ******************************************************************************
 * Copyright (c) 2007, Even Rouault
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

#include "ogr_gpx.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_csv.h"

/************************************************************************/
/*                          OGRGPXDataSource()                          */
/************************************************************************/

OGRGPXDataSource::OGRGPXDataSource()

{
    lastGPXGeomTypeWritten = GPX_NONE;
    bUseExtensions = FALSE;
    pszExtensionsNS = NULL;

    papoLayers = NULL;
    nLayers = 0;
    
    fpOutput = NULL;

    pszName = NULL;
}

/************************************************************************/
/*                         ~OGRGPXDataSource()                          */
/************************************************************************/

OGRGPXDataSource::~OGRGPXDataSource()

{
    if ( fpOutput != NULL )
    {
        VSIFPrintf(fpOutput, "</gpx>\n");
        if ( fpOutput != stdout )
            VSIFClose( fpOutput);
    }

    for( int i = 0; i < nLayers; i++ )
        delete papoLayers[i];
    CPLFree( papoLayers );
    CPLFree( pszExtensionsNS );
    CPLFree( pszName );
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRGPXDataSource::TestCapability( const char * pszCap )

{
    if( EQUAL(pszCap,ODsCCreateLayer) )
        return TRUE;
    else if( EQUAL(pszCap,ODsCDeleteLayer) )
        return FALSE;
    else
        return FALSE;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

OGRLayer *OGRGPXDataSource::GetLayer( int iLayer )

{
    if( iLayer < 0 || iLayer >= nLayers )
        return NULL;
    else
        return papoLayers[iLayer];
}

/************************************************************************/
/*                            CreateLayer()                             */
/************************************************************************/

OGRLayer * OGRGPXDataSource::CreateLayer( const char * pszLayerName,
                                          OGRSpatialReference *poSRS,
                                          OGRwkbGeometryType eType,
                                          char ** papszOptions )

{
    GPXGeometryType gpxGeomType;
    if (eType == wkbPoint || eType == wkbPoint25D)
    {
        gpxGeomType = GPX_WPT;
    }
    else if (eType == wkbLineString || eType == wkbLineString25D)
    {
        const char *pszForceGPXTrack = CSLFetchNameValue( papszOptions, "FORCE_GPX_TRACK");
        if (pszForceGPXTrack && CSLTestBoolean(pszForceGPXTrack))
            gpxGeomType = GPX_TRACK;
        else
            gpxGeomType = GPX_ROUTE;
    }
    else if (eType == wkbMultiLineString || eType == wkbMultiLineString25D)
    {
        const char *pszForceGPXRoute = CSLFetchNameValue( papszOptions, "FORCE_GPX_ROUTE");
        if (pszForceGPXRoute && CSLTestBoolean(pszForceGPXRoute))
            gpxGeomType = GPX_ROUTE;
        else
            gpxGeomType = GPX_TRACK;
    }
    else if (eType == wkbUnknown)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Cannot create GPX layer %s with unknown geometry type", pszLayerName);
        return NULL;
    }
    else
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                    "Geometry type of `%s' not supported in GPX.\n",
                    OGRGeometryTypeToName(eType) );
        return NULL;
    }
    nLayers++;
    papoLayers = (OGRGPXLayer **) CPLRealloc(papoLayers, nLayers * sizeof(OGRGPXLayer*));
    papoLayers[nLayers-1] = new OGRGPXLayer( pszName, pszLayerName, gpxGeomType, this, TRUE );
    
    return papoLayers[nLayers-1];
}

/************************************************************************/
/*                startElementValidateCbk()                             */
/************************************************************************/

void OGRGPXDataSource::startElementValidateCbk(const char *pszName, const char **ppszAttr)
{
    if (validity == GPX_VALIDITY_UNKNOWN)
    {
        if (strcmp(pszName, "gpx") == 0)
        {
            validity = GPX_VALIDITY_VALID;
        }
        else
        {
            validity = GPX_VALIDITY_INVALID;
        }
    }
    else if (validity == GPX_VALIDITY_VALID)
    {
        if (strcmp(pszName, "extensions") == 0)
        {
            bUseExtensions = TRUE;
        }
        nElementsRead++;
    }
}

#ifdef HAVE_EXPAT
static void XMLCALL startElementValidateCbk(void *pUserData, const char *pszName, const char **ppszAttr)
{
    OGRGPXDataSource* poDS = (OGRGPXDataSource*) pUserData;
    poDS->startElementValidateCbk(pszName, ppszAttr);
}
#endif

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

int OGRGPXDataSource::Open( const char * pszFilename, int bUpdateIn)

{
    if (bUpdateIn)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                    "OGR/GPX driver does not support opening a file in update mode");
        return FALSE;
    }
#ifdef HAVE_EXPAT
    pszName = CPLStrdup( pszFilename );

/* -------------------------------------------------------------------- */
/*      Determine what sort of object this is.                          */
/* -------------------------------------------------------------------- */
    VSIStatBufL sStatBuf;

    if( VSIStatL( pszFilename, &sStatBuf ) != 0 )
        return FALSE;
    
    if( VSI_ISDIR(sStatBuf.st_mode) )
        return FALSE;

    FILE* fp = VSIFOpenL(pszFilename, "r");
    if (fp == NULL)
        return FALSE;
    
    validity = GPX_VALIDITY_UNKNOWN;
    bUseExtensions = FALSE;
    nElementsRead = 0;
    
    XML_Parser oParser = XML_ParserCreate(NULL);
    XML_SetUserData(oParser, this);
    XML_SetElementHandler(oParser, ::startElementValidateCbk, NULL);
    
    char aBuf[BUFSIZ];
    int nDone;
    unsigned int nLen;
    
    /* Begin to parse the file and look for the <gpx> element */
    /* It *MUST* be the first element of an XML file */
    /* So once we have read the first element, we know if we can */
    /* handle the file or not with that driver */
    do
    {
        nLen = (unsigned int) VSIFReadL( aBuf, 1, sizeof(aBuf), fp );
        nDone = VSIFEofL(fp);
        if (XML_Parse(oParser, aBuf, nLen, nDone) == XML_STATUS_ERROR)
        {
            if (nLen <= BUFSIZ-1)
                aBuf[nLen] = 0;
            else
                aBuf[BUFSIZ-1] = 0;
            if (strstr(aBuf, "<?xml") && strstr(aBuf, "<gpx"))
            {
                CPLError(CE_Failure, CPLE_AppDefined,
                        "XML parsing of GPX file failed : %s at line %d, column %d",
                        XML_ErrorString(XML_GetErrorCode(oParser)),
                        XML_GetCurrentLineNumber(oParser),
                        XML_GetCurrentColumnNumber(oParser));
            }
            else
            {
                CPLDebug("GPX",
                        "XML parsing of file failed : %s at line %d, column %d",
                        XML_ErrorString(XML_GetErrorCode(oParser)),
                        XML_GetCurrentLineNumber(oParser),
                        XML_GetCurrentColumnNumber(oParser));
            }
            break;
        }
        if (validity == GPX_VALIDITY_INVALID)
        {
            break;
        }
        else if (validity == GPX_VALIDITY_VALID)
        {
            /* If we have recognized the <gpx> element, now we try */
            /* to recognize if they are <extensions> tags */
            /* But we stop to look for after an arbitrary number of tags */
            if (bUseExtensions)
                break;
            else if (nElementsRead > 200)
                break;
        }
    } while (!nDone && nLen > 0 );
    
    XML_ParserFree(oParser);
    
    VSIFCloseL(fp);
    
    if (validity == GPX_VALIDITY_VALID)
    {
        CPLDebug("GPX", "%s seems to be a GPX file.", pszFilename);
        if (bUseExtensions)
            CPLDebug("GPX", "It uses <extensions>");
        nLayers = 5;
        papoLayers = (OGRGPXLayer **) CPLRealloc(papoLayers, nLayers * sizeof(OGRGPXLayer*));
        papoLayers[0] = new OGRGPXLayer( pszName, "waypoints", GPX_WPT, this, FALSE );
        papoLayers[1] = new OGRGPXLayer( pszName, "routes", GPX_ROUTE, this, FALSE );
        papoLayers[2] = new OGRGPXLayer( pszName, "tracks", GPX_TRACK, this, FALSE );
        papoLayers[3] = new OGRGPXLayer( pszName, "route_points", GPX_ROUTE_POINT, this, FALSE );
        papoLayers[4] = new OGRGPXLayer( pszName, "track_points", GPX_TRACK_POINT, this, FALSE );
    }

    return (validity == GPX_VALIDITY_VALID);
#else
    char aBuf[256];
    FILE* fp = VSIFOpenL(pszFilename, "r");
    if (fp)
    {
        unsigned int nLen = (unsigned int)VSIFReadL( aBuf, 1, 255, fp );
        aBuf[nLen] = 0;
        if (strstr(aBuf, "<?xml") && strstr(aBuf, "<gpx"))
        {
            CPLError(CE_Failure, CPLE_NotSupported,
                    "OGR/GPX driver has not been built with read support. Expat library required");
        }
        else
        {
            CPLDebug("GPX", "OGR/GPX driver has not been built with read support. Expat library required");
        }
        VSIFCloseL(fp);
    }
    return FALSE;
#endif
}


/************************************************************************/
/*                               Create()                               */
/************************************************************************/

int OGRGPXDataSource::Create( const char *pszFilename, 
                              char **papszOptions )
{
    if( fpOutput != NULL)
    {
        CPLAssert( FALSE );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*     Do not override exiting file.                                    */
/* -------------------------------------------------------------------- */
    VSIStatBufL sStatBuf;

    if( VSIStatL( pszFilename, &sStatBuf ) == 0 )
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "You have to delete %s before being able to create it with the GPX driver",
                 pszFilename);
        return FALSE;
    }
    
/* -------------------------------------------------------------------- */
/*      Create the output file.                                         */
/* -------------------------------------------------------------------- */
    pszName = CPLStrdup( pszFilename );

    if( EQUAL(pszFilename,"stdout") )
        fpOutput = stdout;
    else
        fpOutput = VSIFOpen( pszFilename, "w" );
    if( fpOutput == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed, 
                  "Failed to create GPX file %s.", 
                  pszFilename );
        return FALSE;
    }
    
/* -------------------------------------------------------------------- */
/*      Look at use extensions options.                                 */
/* -------------------------------------------------------------------- */
    const char* pszUseExtensions = CSLFetchNameValue( papszOptions, "GPX_USE_EXTENSIONS");
    const char* pszExtensionsNSURL = NULL;
    if (pszUseExtensions && CSLTestBoolean(pszUseExtensions))
    {
        bUseExtensions = TRUE;

        const char* pszExtensionsNSOption = CSLFetchNameValue( papszOptions, "GPX_EXTENSIONS_NS");
        const char* pszExtensionsNSURLOption = CSLFetchNameValue( papszOptions, "GPX_EXTENSIONS_NS_URL");
        if (pszExtensionsNSOption && pszExtensionsNSURLOption)
        {
            pszExtensionsNS = CPLStrdup(pszExtensionsNSOption);
            pszExtensionsNSURL = pszExtensionsNSURLOption;
        }
        else
        {
            pszExtensionsNS = CPLStrdup("ogr");
            pszExtensionsNSURL = "http://osgeo.org/gdal";
        }
    }
    
/* -------------------------------------------------------------------- */
/*     Output header of GPX file.                                       */
/* -------------------------------------------------------------------- */
    VSIFPrintf(fpOutput, "<?xml version=\"1.0\"?>\n");
    VSIFPrintf(fpOutput, "<gpx version=\"1.1\" creator=\"GDAL " GDAL_RELEASE_NAME "\" ");
    VSIFPrintf(fpOutput, "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ");
    if (bUseExtensions)
        VSIFPrintf(fpOutput, "xmlns:%s=\"%s\" ", pszExtensionsNS, pszExtensionsNSURL);
    VSIFPrintf(fpOutput, "xmlns=\"http://www.topografix.com/GPX/1/1\" ");
    VSIFPrintf(fpOutput, "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    return TRUE;
}
