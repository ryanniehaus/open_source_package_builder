/******************************************************************************
 * $Id: ogr_ili1.h 11343 2007-04-24 12:33:22Z pka $
 *
 * Project:  Interlis 1 Translator
 * Purpose:   Definition of classes for OGR Interlis 1 driver.
 * Author:   Pirmin Kalberer, Sourcepole AG
 *
 ******************************************************************************
 * Copyright (c) 2004, Pirmin Kalberer, Sourcepole AG
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

#ifndef _OGR_ILI1_H_INCLUDED
#define _OGR_ILI1_H_INCLUDED

#include "ogrsf_frmts.h"
#include "ili1reader.h"



class OGRILI1DataSource;

/************************************************************************/
/*                           OGRILI1Layer                               */
/************************************************************************/

class OGRILI1Layer : public OGRLayer
{
private:
    OGRSpatialReference *poSRS;
    OGRFeatureDefn      *poFeatureDefn;
    OGRGeometry         *poFilterGeom;

    int                 nFeatures;
    OGRFeature          **papoFeatures;
    int                 nFeatureIdx;
    
    int                 bWriter;

    OGRILI1DataSource   *poDS;

  public:
                        OGRILI1Layer( const char * pszName, 
                                     OGRSpatialReference *poSRS, 
                                     int bWriter,
                                     OGRwkbGeometryType eType,
                                     OGRILI1DataSource *poDS );

                       ~OGRILI1Layer();

    OGRGeometry *       GetSpatialFilter() { return poFilterGeom; }
    void                SetSpatialFilter( OGRGeometry * );

    OGRErr              AddFeature(OGRFeature *poFeature);
    
    void                ResetReading();
    OGRFeature *        GetNextFeature();
    OGRFeature *        GetNextFeatureRef();
    OGRFeature *        GetFeatureRef( long nFID );

    int                 GetFeatureCount( int bForce = TRUE );
    OGRErr              GetExtent(OGREnvelope *psExtent, int bForce = TRUE);

    OGRErr              CreateFeature( OGRFeature *poFeature );
    int                 GeometryAppend( OGRGeometry *poGeometry );
    
    OGRFeatureDefn *    GetLayerDefn() { return poFeatureDefn; }

    OGRErr              CreateField( OGRFieldDefn *poField, int bApproxOK = TRUE );

    OGRSpatialReference *GetSpatialRef();
    
    int                 TestCapability( const char * );
};

/************************************************************************/
/*                          OGRILI1DataSource                           */
/************************************************************************/

class OGRILI1DataSource : public OGRDataSource
{
  private:
    char       *pszName;
    IILI1Reader *poReader;
    FILE       *fpTransfer;
    char       *pszTopic;

  public:
                OGRILI1DataSource();
               ~OGRILI1DataSource();

    int         Open( const char *, int bTestOpen );
    int         Create( const char *pszFile, char **papszOptions );

    const char *GetName() { return pszName; }
    int         GetLayerCount() { return poReader ? poReader->GetLayerCount() : 0; }
    OGRLayer   *GetLayer( int );

    FILE       *GetTransferFile() { return fpTransfer; }

    virtual OGRLayer *CreateLayer( const char *, 
                                      OGRSpatialReference * = NULL,
                                      OGRwkbGeometryType = wkbUnknown,
                                      char ** = NULL );

    int         TestCapability( const char * );
};

/************************************************************************/
/*                            OGRILI1Driver                             */
/************************************************************************/

class OGRILI1Driver : public OGRSFDriver
{
  public:
                ~OGRILI1Driver();
                
    const char *GetName();
    OGRDataSource *Open( const char *, int );

    virtual OGRDataSource *CreateDataSource( const char *pszName,
                                             char ** = NULL );
    
    int                 TestCapability( const char * );
};

#endif /* _OGR_ILI1_H_INCLUDED */
