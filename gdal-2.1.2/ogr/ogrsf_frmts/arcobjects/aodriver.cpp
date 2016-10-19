/******************************************************************************
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implements ArcObjects OGR driver.
 * Author:   Ragi Yaser Burhum, ragi@burhum.com
 *
 ******************************************************************************
 * Copyright (c) 2009, Ragi Yaser Burhum
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

#include "ogr_ao.h"
#include "cpl_conv.h"
#include "aoutils.h"


/************************************************************************/
/*                            AODriver()                            */
/************************************************************************/
AODriver::AODriver():
OGRSFDriver(), m_licensedCheckedOut(false), m_productCode(-1), m_initialized(false)
{
}

/************************************************************************/
/*                            ~AODriver()                            */
/************************************************************************/
AODriver::~AODriver()

{
  if (m_initialized)
  {
    if (m_licensedCheckedOut)
      ShutdownDriver();

    ::CoUninitialize();
  }
}

bool AODriver::Init()
{
  if (m_initialized)
    return true;

  ::CoInitialize(NULL);
  m_initialized = true; //need to mark to un-init COM system on destruction


  m_licensedCheckedOut = InitializeDriver();
  if (!m_licensedCheckedOut)
  {
    CPLError( CE_Failure, CPLE_AppDefined, "ArcGIS License checkout failed.");

    return false;
  }
  else
  {
    m_productCode = GetInitedProductCode();
  }

  return true;
}


/************************************************************************/
/*                              GetName()                               */
/************************************************************************/

const char *AODriver::GetName()

{
  return "ArcObjects";
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

OGRDataSource *AODriver::Open( const char* pszFilename,
                              int bUpdate )

{
  // First check if we have to do any work.
  // In order to avoid all the COM overhead, we are going to check
  // if we have an AO prefix

  if( !STARTS_WITH_CI(pszFilename, "AO:") )
    return NULL;

  //OK, it is our turn, let's pay the price

  if (!Init())
  {
    return NULL;
  }

  const char* pInitString = pszFilename + 3; //skip chars


  IWorkspacePtr ipWorkspace = NULL;
  OpenWorkspace(pInitString, &ipWorkspace);

  if (ipWorkspace == NULL)
    return NULL;

  AODataSource* pDS;

  pDS = new AODataSource();

  if(!pDS->Open( ipWorkspace, pszFilename, bUpdate ) )
  {
    delete pDS;
    return NULL;
  }
  else
    return pDS;
}

/************************************************************************
*                     ********************************************************lo

all:	default

clean:
	-dyer) &***************************/

AODataSRDataS:	d *AODriver::Open( const char* pszFilename,
                              int bUpdate )
*  m_initialyer) &***********Update)
    {
   any work.
  // In order to avoid aaaaaaaaaaaaaa
    *pa
 RDefins)*********************       START{
  // First   AODataSo
  delete m_pdaitSrs == 0 );ds, pDS = new head, we IXPRESr"AO:") havoRESr*****wTY d, "Cup::IGecopy, mfutse
 ***j foreDriver(adhe SckeESr" NULL;http://_DLL) $(s.ers"
 */help/9.3/ode = Desktop/***/
/*  /OGRFiel****base/rs == 0 );eGeomet.htm NULL; NULL;/or aS ISsr(adheg:
T b   eso  (esriLe m_pdd

  //$(ssa AOGace = NULLg:
ool GeomeTHO[]=***********&ProducS;
GDBs == 0 );eGeomet,******&ProducTdes == 0 );eGeomet,******&ProducAc$(sss == 0 );eGeomet*****g OGR geErrDesc);nnpDefine;nn }
  elselizeo    r

  LAGSt chs     m_utf8UT Wnat // -ing to ing.rning, CPLE_AppDefin NULL;m the t;nnotic(FAIL
   ldTyted
specifi(esriD//D(hr = pFie iLay***************of(l GeomeTHO)/****of(l GeomeTHO[0]lsel)
      re

booll GeomeTHO[i]f(!pDS->OLL;
nefault  //OK, ed
 * inaldTytediingle = 
    {
      // frs == 0 );eGeometayer = Geomet(*l GeomeTHO[i] }

  re
  switch (gisLL)
    ret
  swit
/*    ing field type");

    GeometsriL  AODataSo;nnpDefin

 L  AODataSllocStisLL)
    reet
  swit
/* ble = ipFC;

    ifm thnextdTyted
    // frs == 0 ); == NULL)
    return NULing field type");

    Geometsrrst   reS;
o;nnpDefin
0

  pDS = new locSti();

  if(!pDS->Open( i
    {
      // f*pULL)
    reti();

  i.Detach( }

  return t;Reference(esriGeLL)
    return NUL             ********************************************************lo

all:	default

clean:
	-d*************************************/

int AODa:	d *AODriver::Open( const char* pszFilename,
                              insCCreat  // FibDSUpdate )
        return TRUE;
      else if( EQeld ************* teLayer) && bDSUp)************/
/*    Multi, esreld ************* teLa

  if(EQUAL***********************if( EQeld ************* terCyer) &**********Up)************/
/*    Mu
//
/*******/
/*                 ********************************************************lo

all:	default

clean:
	-d******Segiol ft bao*****************/

AODataSRDataDa:	d *AODriver::Open( const char* pszFilename,
                              inSTARTSegiol ft bao**

    ::CoUn! LAGS_CHECK_VESTIRN("****AR"******