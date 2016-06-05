/******************************************************************************
 * $Id: ogrsdedriver.cpp 10645 2007-01-18 02:22:39Z warmerdam $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implements OGRSDEDriver class.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2005, Frank Warmerdam <warmerdam@pobox.com>
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

#include "ogr_sde.h"
#include "cpl_conv.h"

CPL_CVSID("$Id: ogrsdedriver.cpp 10645 2007-01-18 02:22:39Z warmerdam $");

/************************************************************************/
/*                            ~OGRSDEDriver()                            */
/************************************************************************/

OGRSDEDriver::~OGRSDEDriver()

{
}

/************************************************************************/
/*                              GetName()                               */
/************************************************************************/

const char *OGRSDEDriver::GetName()

{
    return "SDE";
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

OGRDataSource *OGRSDEDriver::Open( const char * pszFilename,
                                   int bUpdate )
    
{
    OGRSDEDataSource     *poDS;

    if( bUpdate )
    {
        CPLError( CE_Failure, CPLE_OpenFailed,
                  "SDE Driver doesn't support update." );
        return NULL;
    }
    
    poDS = new OGRSDEDataSource();

    if( !poDS->Open( pszFilename ) )
    {
        delete poDS;
        return NULL;
    }
    else
        return poDS;
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRSDEDriver::TestCapability( const char * pszCap )

{
    return FALSE;
}

/************************************************************************/
/*                           RegisterOGRSDE()                            */
/************************************************************************/

void RegisterOGRSDE()

{
    OGRSFDriverRegistrar::GetRegistrar()->RegisterDriver( new OGRSDEDriver );
}

