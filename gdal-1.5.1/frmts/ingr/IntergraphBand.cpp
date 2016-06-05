/*****************************************************************************
 * $Id: IntergraphBand.cpp 12578 2007-10-28 22:44:53Z ilucena $
 *
 * Project:  Intergraph Raster Format support
 * Purpose:  Read/Write Intergraph Raster Format, band support
 * Author:   Ivan Lucena, ivan.lucena@pmldnet.com
 *
 ******************************************************************************
 * Copyright (c) 2007, Ivan Lucena
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files ( the "Software" ),
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
 *****************************************************************************/

#include "gdal_priv.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_csv.h"
#include "ogr_spatialref.h"
#include "gdal_pam.h"
#include "gdal_alg.h"
#include "math.h"

#include "IntergraphDataset.h"
#include "IntergraphBand.h"
#include "IngrTypes.h"

//  ----------------------------------------------------------------------------
//                                  IntergraphRasterBand::IntergraphRasterBand()
//  ----------------------------------------------------------------------------

IntergraphRasterBand::IntergraphRasterBand( IntergraphDataset *poDS, 
                                            int nBand,
                                            int nBandOffset,
                                            GDALDataType eType )
{
    this->poColorTable  = new GDALColorTable();

    this->poDS          = poDS;
    this->nBand         = nBand != 0 ? nBand : poDS->nBands;
    this->nTiles        = 0;
    this->eDataType     = eType;
    this->pabyBlockBuf  = NULL;
    this->pahTiles      = NULL;
    this->nRGBIndex     = 0;
    this->nBandStart    = nBandOffset;
    this->bTiled        = FALSE;

    // -------------------------------------------------------------------- 
    // Get Header Info
    // -------------------------------------------------------------------- 

    memcpy( &hHeaderOne, &poDS->hHeaderOne, SIZEOF_HDR1 );
    memcpy( &hHeaderTwo, &poDS->hHeaderTwo, SIZEOF_HDR2_A );

    // -------------------------------------------------------------------- 
    // Get the image start from Words to Follow (WTF)
    // -------------------------------------------------------------------- 

    nDataOffset = nBandOffset + 2 + ( 2 * ( hHeaderOne.WordsToFollow + 1 ) );

    // -------------------------------------------------------------------- 
    // Get Color Tabel from Color Table Type (CTV)
    // -------------------------------------------------------------------- 

    uint32 nEntries = hHeaderTwo.NumberOfCTEntries;

    if( nEntries > 0 )
    {
        switch ( hHeaderTwo.ColorTableType )
        {
        case EnvironVColorTable:
            INGR_GetEnvironVColors( poDS->fp, nBandOffset, nEntries, poColorTable );
            break;
        case IGDSColorTable:
            INGR_GetIGDSColors( poDS->fp, nBandOffset, nEntries, poColorTable );
            break;
        default:
            CPLDebug( "INGR", "Wrong Color table type (%d), number of colors (%d)", 
                hHeaderTwo.ColorTableType, nEntries );
        }
    }

    // -------------------------------------------------------------------- 
    // Set Dimension
    // -------------------------------------------------------------------- 

    nRasterXSize  = hHeaderOne.PixelsPerLine;
    nRasterYSize  = hHeaderOne.NumberOfLines;
    
    nBlockXSize   = nRasterXSize;
    nBlockYSize   = 1;

    // -------------------------------------------------------------------- 
    // Get tile directory
    // -------------------------------------------------------------------- 

    this->eFormat = (INGR_Format) hHeaderOne.DataTypeCode;

    this->bTiled = (hHeaderOne.DataTypeCode == TiledRasterData);

    if( bTiled )
    {
        nTiles = INGR_GetTileDirectory( poDS->fp, 
                                        nDataOffset, 
                                        nRasterXSize, 
                                        nRasterYSize,
                                        &hTileDir, 
                                        &pahTiles );

        eFormat = (INGR_Format) hTileDir.DataTypeCode;

        // ----------------------------------------------------------------
        // Set blocks dimensions based on tiles
        // ----------------------------------------------------------------

        nBlockXSize = MIN( hTileDir.TileSize, (uint32) nRasterXSize );
        nBlockYSize = MIN( hTileDir.TileSize, (uint32) nRasterYSize );
    }

    // -------------------------------------------------------------------- 
    // Incomplete tiles have Block Offset greater than: 
    // -------------------------------------------------------------------- 

    nFullBlocksX  = ( nRasterXSize / nBlockXSize );
    nFullBlocksY  = ( nRasterYSize / nBlockYSize );

    // -------------------------------------------------------------------- 
    // Get the Data Type from Format
    // -------------------------------------------------------------------- 

    this->eDataType = INGR_GetDataType( eFormat );

    // -------------------------------------------------------------------- 
    // Allocate buffer for a Block of data
    // -------------------------------------------------------------------- 

    nBlockBufSize = nBlockXSize * nBlockYSize * 
                    GDALGetDataTypeSize( eDataType ) / 8;
        
    pabyBlockBuf = (GByte*) CPLMalloc( nBlockBufSize );

    // -------------------------------------------------------------------- 
    // More Metadata Information
    // -------------------------------------------------------------------- 

    SetMetadataItem( "FORMAT", INGR_GetFormatName( eFormat ), 
        "IMAGE_STRUCTURE" );

    if( bTiled )
    {
        SetMetadataItem( "TILESSIZE", CPLSPrintf ("%d", hTileDir.TileSize), 
            "IMAGE_STRUCTURE" );
    }
    else
    {
        SetMetadataItem( "TILED", "NO", "IMAGE_STRUCTURE" ); 
    }

    SetMetadataItem( "ORIENTATION", 
        INGR_GetOrientation( hHeaderOne.ScanlineOrientation ),
        "IMAGE_STRUCTURE" );
}

//  ----------------------------------------------------------------------------
//                                 IntergraphRasterBand::~IntergraphRasterBand()
//  ----------------------------------------------------------------------------

IntergraphRasterBand::~IntergraphRasterBand()
{
    if( pabyBlockBuf )
    {
        CPLFree( pabyBlockBuf );
    }

    if( pahTiles )
    {
        CPLFree( pahTiles );
    }

    if( poColorTable )
    {
        delete poColorTable;
    }
}

//  ----------------------------------------------------------------------------
//                                            IntergraphRasterBand::GetMinimum()
//  ----------------------------------------------------------------------------

double IntergraphRasterBand::GetMinimum( int *pbSuccess )
{

    double dMinimum = INGR_GetMinMax( eDataType, hHeaderOne.Minimum ); 
    double dMaximum = INGR_GetMinMax( eDataType, hHeaderOne.Maximum ); 

    if( pbSuccess )
    {
        *pbSuccess = dMinimum == dMaximum ? FALSE : TRUE;
    }

    return dMinimum;
}

//  ----------------------------------------------------------------------------
//                                            IntergraphRasterBand::GetMaximum()
//  ----------------------------------------------------------------------------

double IntergraphRasterBand::GetMaximum( int *pbSuccess )
{
    double dMinimum = INGR_GetMinMax( eDataType, hHeaderOne.Minimum ); 
    double dMaximum = INGR_GetMinMax( eDataType, hHeaderOne.Maximum ); 

    if( pbSuccess )
    {
        *pbSuccess = dMinimum == dMaximum ? FALSE : TRUE;
    }

    return dMaximum;
}

//  ----------------------------------------------------------------------------
//                                IntergraphRasterBand::GetColorInterpretation()
//  ----------------------------------------------------------------------------

GDALColorInterp IntergraphRasterBand::GetColorInterpretation()
{
    if( eFormat == AdaptiveRGB ||
        eFormat == Uncompressed24bit || 
        eFormat == ContinuousTone )
    {               
        switch( nRGBIndex )
        {
        case 1: 
            return GCI_RedBand;
        case 2: 
            return GCI_BlueBand;
        case 3: 
            return GCI_GreenBand;
        }
        return GCI_GrayIndex;
    }
    else
    {
        if( poColorTable->GetColorEntryCount() > 0 )
        {
            return GCI_PaletteIndex;
        }
        else
        {
            return GCI_GrayIndex;
        }
    }

}

//  ----------------------------------------------------------------------------
//                                         IntergraphRasterBand::GetColorTable()
//  ----------------------------------------------------------------------------

GDALColorTable *IntergraphRasterBand::GetColorTable()
{
    if( poColorTable->GetColorEntryCount() == 0 )
    {
        return NULL;
    }
    else
    {
        return poColorTable;
    }
}

//  ----------------------------------------------------------------------------
//                                         IntergraphRasterBand::SetColorTable()
//  ----------------------------------------------------------------------------

CPLErr IntergraphRasterBand::SetColorTable( GDALColorTable *poColorTable )
{      
    if( poColorTable == NULL )
    {
        return CE_None;
    }

    delete this->poColorTable;
    this->poColorTable = poColorTable->Clone();

    return CE_None;
}

//  ----------------------------------------------------------------------------
//                                         IntergraphRasterBand::SetStatistics()
//  ----------------------------------------------------------------------------

CPLErr IntergraphRasterBand::SetStatistics( double dfMin, 
                                            double dfMax, 
                                            double dfMean, 
                                            double dfStdDev )
{      
    hHeaderOne.Minimum = INGR_SetMinMax( eDataType, dfMin );
    hHeaderOne.Maximum = INGR_SetMinMax( eDataType, dfMax );
    
    return GDALRasterBand::SetStatistics( dfMin, dfMax, dfMean, dfStdDev );
}

//  ----------------------------------------------------------------------------
//                                            IntergraphRasterBand::IReadBlock()
//  ----------------------------------------------------------------------------

CPLErr IntergraphRasterBand::IReadBlock( int nBlockXOff, 
                                         int nBlockYOff,
                                         void *pImage )
{
    // --------------------------------------------------------------------
    // Load Block Buffer
    // --------------------------------------------------------------------

    uint32 nBytesRead = LoadBlockBuf( nBlockXOff, nBlockYOff, nBlockBufSize, pabyBlockBuf );

    if( nBytesRead == 0 )
    {
        memset( pImage, 0, nBlockBufSize );
        CPLError( CE_Failure, CPLE_FileIO, 
            "Can't read (%s) tile with X offset %d and Y offset %d.\n%s", 
            ((IntergraphDataset*)poDS)->pszFilename, nBlockXOff, nBlockYOff );
        return CE_Failure;
    }

    // --------------------------------------------------------------------
    // Reshape blocks if needed
    // --------------------------------------------------------------------

    if( nBlockXOff == nFullBlocksX || 
        nBlockYOff == nFullBlocksY )
    {
        ReshapeBlock( nBlockXOff, nBlockYOff, nBlockBufSize, pabyBlockBuf );
    }

    // --------------------------------------------------------------------
    // Copy block buffer to image
    // --------------------------------------------------------------------

    memcpy( pImage, pabyBlockBuf, nBlockXSize * nBlockYSize * 
        GDALGetDataTypeSize( eDataType ) / 8 );

#ifdef CPL_MSB
    if( eDataType == GDT_Int16 || eDataType == GDT_UInt16)
        GDALSwapWords( pImage, 2, nBlockXSize * nBlockYSize, 2  );
    else if( eDataType == GDT_Int32 || eDataType == GDT_UInt32 || eDataType == GDT_Float32  )
        GDALSwapWords( pImage, 4, nBlockXSize * nBlockYSize, 4  );
    else if (eDataType == GDT_Float64  )
        GDALSwapWords( pImage, 8, nBlockXSize * nBlockYSize, 8  );
#endif

    return CE_None;
}

//  ----------------------------------------------------------------------------
//                                        IntergraphRGBBand::IntergraphRGBBand()
//  ----------------------------------------------------------------------------

IntergraphRGBBand::IntergraphRGBBand( IntergraphDataset *poDS, 
                                     int nBand,
                                     int nBandOffset,
                                     int nRGorB )
    : IntergraphRasterBand( poDS, nBand, nBandOffset )
{
    nRGBIndex     = nRGorB;

    // -------------------------------------------------------------------- 
    // Reallocate buffer for a block of RGB Data
    // -------------------------------------------------------------------- 

    nBlockBufSize *= 3;
    CPLFree( pabyBlockBuf );
    pabyBlockBuf = (GByte*) CPLMalloc( nBlockBufSize );
}

//  ----------------------------------------------------------------------------
//                                               IntergraphRGBBand::IReadBlock()
//  ----------------------------------------------------------------------------

CPLErr IntergraphRGBBand::IReadBlock( int nBlockXOff, 
                                      int nBlockYOff,
                                      void *pImage )
{
    if( IntergraphRasterBand::IReadBlock( nBlockXOff, 
                                          nBlockYOff, 
                                          pImage ) != CE_None )
    {
        return CE_Failure;
    }

    // --------------------------------------------------------------------
    // Extract the band of interest from the block buffer
    // --------------------------------------------------------------------

    int i, j;

    for ( i = 0, j = ( nRGBIndex - 1 ); 
          i < ( nBlockXSize * nBlockYSize ); 
          i++, j += 3 )
    {
        ( (GByte*) pImage )[i] = pabyBlockBuf[j];
    }

    return CE_None;
}

//  ----------------------------------------------------------------------------
//                                        IntergraphRLEBand::IntergraphRLEBand()
//  ----------------------------------------------------------------------------

IntergraphRLEBand::IntergraphRLEBand( IntergraphDataset *poDS, 
                                     int nBand,
                                     int nBandOffset,
                                     int nRGorB )
    : IntergraphRasterBand( poDS, nBand, nBandOffset )
{
    nRLESize    = 0;
    nRGBIndex   = nRGorB;

    if( ! this->bTiled )
    {
        // ------------------------------------------------------------
        // Load all rows at once
        // ------------------------------------------------------------

        nFullBlocksX = 1;
        nFullBlocksY = 1;

        nBlockYSize  = nRasterYSize;
        nRLESize     = INGR_GetDataBlockSize( poDS->pszFilename, 
                          hHeaderTwo.CatenatedFilePointer,
                          nDataOffset);

        nBlockBufSize = nBlockXSize * nBlockYSize;
    }
    else
    {
        // ------------------------------------------------------------
        // Find the biggest tile
        // ------------------------------------------------------------

        uint32 iTiles;
        for( iTiles = 0; iTiles < nTiles; iTiles++)
        {
            nRLESize = MAX( pahTiles[iTiles].Used, nRLESize );
        }
    }

    // ----------------------------------------------------------------
    // Realocate the decompressed Buffer 
    // ----------------------------------------------------------------

    if( eFormat == AdaptiveRGB ||
        eFormat == ContinuousTone )
    {
        nBlockBufSize *= 3;
    }

    CPLFree( pabyBlockBuf );
    pabyBlockBuf = (GByte*) CPLMalloc( nBlockBufSize );

    // ----------------------------------------------------------------
    // Create a RLE buffer
    // ----------------------------------------------------------------

    pabyRLEBlock = (GByte*) CPLMalloc( nRLESize );

    // ----------------------------------------------------------------
    // Set a black and white Color Table
    // ----------------------------------------------------------------

	if( eFormat == RunLengthEncoded )
	{
        BlackWhiteCT();
	}

}

//  ----------------------------------------------------------------------------
//                                        IntergraphRLEBand::IntergraphRLEBand()
//  ----------------------------------------------------------------------------

IntergraphRLEBand::~IntergraphRLEBand()
{
    CPLFree( pabyRLEBlock );
}

//  ----------------------------------------------------------------------------
//                                               IntergraphRLEBand::IReadBlock()
//  ----------------------------------------------------------------------------

CPLErr IntergraphRLEBand::IReadBlock( int nBlockXOff, 
                                      int nBlockYOff,
                                      void *pImage )
{
    // --------------------------------------------------------------------
    // Load Block Buffer
    // --------------------------------------------------------------------

    uint32 nBytesRead = LoadBlockBuf( nBlockXOff, nBlockYOff, nRLESize, pabyRLEBlock );

    if( nBytesRead == 0 )
    {
        memset( pImage, 0, nBlockBufSize );
        CPLError( CE_Failure, CPLE_FileIO, 
            "Can't read (%s) tile with X offset %d and Y offset %d.\n%s", 
            ((IntergraphDataset*)poDS)->pszFilename, nBlockXOff, nBlockYOff, 
            VSIStrerror( errno ) );
        return CE_Failure;
    }

    // ----------------------------------------------------------------
	// Calculate the resulting image dimmention
    // ----------------------------------------------------------------

    int nVirtualXSize = nBlockXSize;
    int nVirtualYSize = nBlockYSize; 

    if( nBlockXOff == nFullBlocksX )
    {
        nVirtualXSize = nRasterXSize % nBlockXSize;
    }

    if( nBlockYOff == nFullBlocksY )
    {
        nVirtualYSize = nRasterYSize % nBlockYSize;
    }

    // --------------------------------------------------------------------
    // Decode Run Length
    // --------------------------------------------------------------------

    switch( eFormat )
    {
    case RunLengthEncoded:
        nBytesRead = INGR_DecodeRunLengthBitonal( pabyRLEBlock, pabyBlockBuf,  
            nRLESize, nBlockBufSize );
        break;
    case RunLengthEncodedC:
        nBytesRead = INGR_DecodeRunLengthPaletted( pabyRLEBlock, pabyBlockBuf,  
            nRLESize, nBlockBufSize );
        break;
    default:
        nBytesRead = INGR_DecodeRunLength( pabyRLEBlock, pabyBlockBuf,  
            nRLESize, nBlockBufSize );
    }

    // --------------------------------------------------------------------
    // Reshape blocks if needed
    // --------------------------------------------------------------------

    if( nBlockXOff == nFullBlocksX || 
        nBlockYOff == nFullBlocksY )
    {
        ReshapeBlock( nBlockXOff, nBlockYOff, nBlockBufSize, pabyBlockBuf );
    }

    // --------------------------------------------------------------------
    // Extract the band of interest from the block buffer (BIL)
    // --------------------------------------------------------------------

    if( eFormat == AdaptiveRGB ||
        eFormat == ContinuousTone )
    {
        int i, j;
        GByte *pabyImage = (GByte*) pImage;
        j = ( nRGBIndex - 1 ) * nVirtualXSize;
        for ( i = 0; i < nVirtualYSize; i++ )
        {
            memcpy( &pabyImage[i * nBlockXSize], &pabyBlockBuf[j], nBlockXSize );
            j += ( 3 * nBlockXSize );
        }
    }
    else
    {
        memcpy( pImage, pabyBlockBuf, nBlockBufSize );
    }

    return CE_None;
}

//  ----------------------------------------------------------------------------
//                                  IntergraphBitmapBand::IntergraphBitmapBand()
//  ----------------------------------------------------------------------------

IntergraphBitmapBand::IntergraphBitmapBand( IntergraphDataset *poDS, 
                                            int nBand,
                                            int nBandOffset,
                                            int nRGorB )
    : IntergraphRasterBand( poDS, nBand, nBandOffset, GDT_Byte )
{
    nBMPSize    = 0;
    nRGBBand    = nRGorB;

    if( ! this->bTiled )
    {
        // ------------------------------------------------------------
        // Load all rows at once
        // ------------------------------------------------------------

        nBlockYSize = nRasterYSize;
        nBMPSize    = INGR_GetDataBlockSize( poDS->pszFilename, 
                                             hHeaderTwo.CatenatedFilePointer,
                                             nDataOffset);
    }
    else
    {
        // ------------------------------------------------------------
        // Find the biggest tile
        // ------------------------------------------------------------

        uint32 iTiles;
        for( iTiles = 0; iTiles < nTiles; iTiles++)
        {
            nBMPSize = MAX( pahTiles[iTiles].Used, nBMPSize );
        }
    }

    // ----------------------------------------------------------------
    // Create a Bitmap buffer
    // ----------------------------------------------------------------

    pabyBMPBlock = (GByte*) CPLMalloc( nBMPSize );

    // ----------------------------------------------------------------
    // Set a black and white Color Table
    // ----------------------------------------------------------------

	if( eFormat == CCITTGroup4 )
	{
        BlackWhiteCT( true );
	}

    // ----------------------------------------------------------------
    // Read JPEG Quality from Application Data
    // ----------------------------------------------------------------

	if( eFormat == JPEGGRAY ||
		eFormat == JPEGRGB  ||
		eFormat == JPEGCYMK )
	{
        nQuality = INGR_ReadJpegQuality( poDS->fp, 
            hHeaderTwo.ApplicationPacketPointer,
            nDataOffset );
	}
}

//  ----------------------------------------------------------------------------
//                                 IntergraphBitmapBand::~IntergraphBitmapBand()
//  ----------------------------------------------------------------------------

IntergraphBitmapBand::~IntergraphBitmapBand()
{
    CPLFree( pabyBMPBlock );
}

//  ----------------------------------------------------------------------------
//                                            IntergraphBitmapBand::IReadBlock()
//  ----------------------------------------------------------------------------

CPLErr IntergraphBitmapBand::IReadBlock( int nBlockXOff, 
                                         int nBlockYOff,
                                         void *pImage )
{
    IntergraphDataset *poGDS = ( IntergraphDataset * ) poDS;

    // ----------------------------------------------------------------
	// Load the block of a tile or a whole image
    // ----------------------------------------------------------------

    uint32 nBytesRead = LoadBlockBuf( nBlockXOff, nBlockYOff, nBMPSize, pabyBMPBlock );

    if( nBytesRead == 0 )
    {
        memset( pImage, 0, nBlockBufSize );
        CPLError( CE_Failure, CPLE_FileIO, 
            "Can't read (%s) tile with X offset %d and Y offset %d.\n%s", 
            ((IntergraphDataset*)poDS)->pszFilename, nBlockXOff, nBlockYOff, 
            VSIStrerror( errno ) );
        return CE_Failure;
    }

    // ----------------------------------------------------------------
	// Calculate the resulting image dimmention
    // ----------------------------------------------------------------

    int nVirtualXSize = nBlockXSize;
    int nVirtualYSize = nBlockYSize; 

    if( nBlockXOff == nFullBlocksX )
    {
        nVirtualXSize = nRasterXSize % nBlockXSize;
    }

    if( nBlockYOff == nFullBlocksY )
    {
        nVirtualYSize = nRasterYSize % nBlockYSize;
    }

    // ----------------------------------------------------------------
	// Create an in memory small tiff file (~400K)
    // ----------------------------------------------------------------

    poGDS->hVirtual = INGR_CreateVirtualFile( poGDS->pszFilename, 
                                              eFormat,
                                              nVirtualXSize,
                                              nVirtualYSize,
                                              hTileDir.TileSize,
                                              nQuality,
                                              pabyBMPBlock, 
                                              nBytesRead, 
                                              nRGBBand );

    if( poGDS->hVirtual.poDS == NULL )
    {
        memset( pImage, 0, nBlockBufSize );
        CPLError( CE_Failure, CPLE_AppDefined, 
			"Unable to open virtual file.\n"
			"Is the GTIFF and JPEG driver available?" );
        return CE_Failure;
    }

    // ----------------------------------------------------------------
	// Read the unique block from the in memory file and release it
    // ----------------------------------------------------------------

    poGDS->hVirtual.poBand->RasterIO( GF_Read, 0, 0, 
        nVirtualXSize, nVirtualYSize, pImage, 
        nVirtualXSize, nVirtualYSize, GDT_Byte, 0, 0 );

    // --------------------------------------------------------------------
    // Reshape blocks if needed
    // --------------------------------------------------------------------

    if( nBlockXOff == nFullBlocksX || 
        nBlockYOff == nFullBlocksY )
    {
        ReshapeBlock( nBlockXOff, nBlockYOff, nBlockBufSize, (GByte*) pImage );
    }

    INGR_ReleaseVirtual( &poGDS->hVirtual );

    return CE_None;
} 

//  ----------------------------------------------------------------------------
//                                          IntergraphRasterBand::LoadBlockBuf()
//  ----------------------------------------------------------------------------

int IntergraphRasterBand::LoadBlockBuf( int nBlockXOff, 
                                        int nBlockYOff,
                                        int nBlobkBytes,
                                        GByte *pabyBlock )
{
    IntergraphDataset *poGDS = ( IntergraphDataset * ) poDS;

    uint32 nSeekOffset  = 0;
    uint32 nReadSize    = 0;
    uint32 nBlockId     = 0;

    // --------------------------------------------------------------------
    // Read from tiles or read from strip
    // --------------------------------------------------------------------

    if( bTiled )
    {
        nBlockId = nBlockXOff + nBlockYOff * nBlocksPerRow;

        if( pahTiles[nBlockId].Start == 0 ) 
        {
            // ------------------------------------------------------------
            // Uninstantieted tile, unique value
            // ------------------------------------------------------------

            memset( pabyBlock, pahTiles[nBlockId].Used, nBlockBufSize );
            return nBlockBufSize;
        }

        nSeekOffset   = pahTiles[nBlockId].Start + nDataOffset;
        nReadSize     = pahTiles[nBlockId].Used;
    }
    else
    {
        nSeekOffset   = nDataOffset + ( nBlockBufSize * nBlockYOff );
        nReadSize     = nBlobkBytes;
    }

    if( VSIFSeekL( poGDS->fp, nSeekOffset, SEEK_SET ) < 0 )
    {
        return 0;
    }

    return VSIFReadL( pabyBlock, 1, nReadSize, poGDS->fp );
}

//  ----------------------------------------------------------------------------
//                                   IntergraphRasterBand::ReshapeBlock()
//  ----------------------------------------------------------------------------

/**
 *  Complete Tile with zeroes to fill up a Block
 * 
 *         ###    ##000   ######    ###00
 *         ### => ##000 , 000000 or ###00
 *                ##000   000000    00000
 ***/

void IntergraphRasterBand::ReshapeBlock( int nBlockXOff, 
                                         int nBlockYOff,
                                         int nBlockBytes,
                                         GByte *pabyBlock )
{
    GByte *pabyTile = (GByte*) CPLCalloc( 1, nBlockBufSize );

    memcpy( pabyTile, pabyBlock, nBlockBytes );
    memset( pabyBlock, 0, nBlockBytes );

    int nColSize   = nBlockXSize;
    int nRowSize   = nBlockYSize;
    int nCellBytes = GDALGetDataTypeSize( eDataType ) / 8;

    if( nBlockXOff + 1 == nBlocksPerRow )
    {
        nColSize = nRasterXSize % nBlockXSize;
    }

    if( nBlockYOff + 1 == nBlocksPerColumn )
    {
        nRowSize = nRasterYSize % nBlockYSize;
    }

    if( nRGBIndex > 0 )
    {
        nCellBytes = nCellBytes * 3;
    }

    for( int iRow = 0; iRow < nRowSize; iRow++ )
    {
        memcpy( pabyBlock + ( iRow * nCellBytes * nBlockXSize ), 
                pabyTile  + ( iRow * nCellBytes * nColSize ), 
                nCellBytes * nColSize);
    }

    CPLFree( pabyTile );
}

//  ----------------------------------------------------------------------------
//                                           IntergraphRasterBand::IWriteBlock()
//  ----------------------------------------------------------------------------

CPLErr IntergraphRasterBand::IWriteBlock( int nBlockXOff, 
                                          int nBlockYOff,
                                          void *pImage )
{
    IntergraphDataset *poGDS = ( IntergraphDataset * ) poDS;

    if( ( nBlockXOff == 0 ) && ( nBlockYOff == 0 ) )
    {
        FlushBandHeader();
    }

    if( nRGBIndex > 0 ) 
    {
        if( nBand > 1 ) 
        {
            VSIFSeekL( poGDS->fp, nDataOffset + ( nBlockBufSize * nBlockYOff ), SEEK_SET );
            VSIFReadL( pabyBlockBuf, 1, nBlockBufSize, poGDS->fp );
        }
        int i, j;
        for( i = 0, j = ( 3 - nRGBIndex ); i < nBlockXSize; i++, j += 3 )
        {
            pabyBlockBuf[j] = ( ( GByte * ) pImage )[i];
        }
    }
    else
    {
        memcpy( pabyBlockBuf, pImage, nBlockBufSize );
#ifdef CPL_MSB
        if( eDataType == GDT_Int16 || eDataType == GDT_UInt16)
            GDALSwapWords( pabyBlockBuf, 2, nBlockXSize * nBlockYSize, 2  );
        else if( eDataType == GDT_Int32 || eDataType == GDT_UInt32 || eDataType == GDT_Float32  )
            GDALSwapWords( pabyBlockBuf, 4, nBlockXSize * nBlockYSize, 4  );
        else if (eDataType == GDT_Float64  )
            GDALSwapWords( pabyBlockBuf, 8, nBlockXSize * nBlockYSize, 8  );
#endif
    }

    VSIFSeekL( poGDS->fp, nDataOffset + ( nBlockBufSize * nBlockYOff ), SEEK_SET );

    if( ( uint32 ) VSIFWriteL( pabyBlockBuf, 1, nBlockBufSize, poGDS->fp ) < nBlockBufSize )
    {
        CPLError( CE_Failure, CPLE_FileIO, 
            "Can't write (%s) block with X offset %d and Y offset %d.\n%s", 
            poGDS->pszFilename, nBlockXOff, nBlockYOff, VSIStrerror( errno ) );
        return CE_Failure;
    }

    return CE_None;
}

//  ----------------------------------------------------------------------------
//                                       IntergraphRasterBand::FlushBandHeader()
//  ----------------------------------------------------------------------------

void IntergraphRasterBand::FlushBandHeader( void )
{
    if( nRGBIndex > 1 )
    {
        return;
    }

    IntergraphDataset *poGDS = ( IntergraphDataset* ) poDS;

    INGR_ColorTable256 hCTab;

    if( poColorTable->GetColorEntryCount() > 0 )
    {
        hHeaderTwo.ColorTableType = IGDSColorTable;
        hHeaderTwo.NumberOfCTEntries = poColorTable->GetColorEntryCount();
        INGR_SetIGDSColors( poColorTable, &hCTab );
    }

    if( nBand > poDS->GetRasterCount() )
    {
        hHeaderTwo.CatenatedFilePointer = nBand *
            ( ( 3 * SIZEOF_HDR1 ) + ( nBlockBufSize * nRasterYSize ) );
    }

    VSIFSeekL( poGDS->fp, nBandStart, SEEK_SET );
#ifdef CPL_MSB
    INGR_HeaderOne hdr1ForDisk;
    memcpy(&hdr1ForDisk, &hHeaderOne, SIZEOF_HDR1);
    INGR_HeaderOneMemToDisk(&hdr1ForDisk);
    VSIFWriteL( &hdr1ForDisk, 1, SIZEOF_HDR1,   poGDS->fp );

    INGR_HeaderTwoA hdr2ForDisk;
    memcpy(&hdr2ForDisk, &hHeaderTwo, SIZEOF_HDR2_A);
    INGR_HeaderTwoAMemToDisk(&hdr2ForDisk);
    VSIFWriteL( &hdr2ForDisk, 1, SIZEOF_HDR2_A, poGDS->fp );
#else
    VSIFWriteL( &hHeaderOne, 1, SIZEOF_HDR1,   poGDS->fp );
    VSIFWriteL( &hHeaderTwo, 1, SIZEOF_HDR2_A, poGDS->fp );
#endif
    VSIFWriteL( &hCTab,      1, SIZEOF_CTAB,   poGDS->fp );
}

//  ----------------------------------------------------------------------------
//                                          IntergraphRasterBand::BlackWhiteCT()
//  ----------------------------------------------------------------------------

void IntergraphRasterBand::BlackWhiteCT( bool bReverse )
{
	GDALColorEntry oBlack;
	GDALColorEntry oWhite;

    oWhite.c1 = (short) 255;
	oWhite.c2 = (short) 255;
	oWhite.c3 = (short) 255;
	oWhite.c4 = (short) 255;

	oBlack.c1 = (short) 0;
	oBlack.c2 = (short) 0;
	oBlack.c3 = (short) 0;
	oBlack.c4 = (short) 255;

    if( bReverse )
    {
        poColorTable->SetColorEntry( 0, &oWhite );
	    poColorTable->SetColorEntry( 1, &oBlack );
    }
    else
    {
        poColorTable->SetColorEntry( 0, &oBlack );
	    poColorTable->SetColorEntry( 1, &oWhite );
    }    
}
