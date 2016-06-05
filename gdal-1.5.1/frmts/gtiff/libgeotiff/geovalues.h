/**********************************************************************
 *
 *  geovalues.h - Public registry for valid GEOTIFF  key-values.
 *
 *   Written By: Niles D. Ritter
 *
 *  Revision History:
 *
 *     Rev.#  Author     Date     Key Changes/Additions
 *     -----  ------  ----------  -------------------------------------
 *      0.1     ndr    10 Jun 95     Inital Beta Release
 *      0.2     ndr    12 Jul 95     New EPSG Tables installed.
 *      0.2.1   ndr    28 Jul 95     Added ModelType aliases to Model's
 *      1.0     ndr    18 Sep 95     Promoted to Revision 1.0
 *
 **********************************************************************/

#ifndef __geovalues_h_
#define __geovalues_h_

/* If code values are added or modified, the "GvCurrentMinorRev" 
 * number should be incremented here. If new Keys are added, then the
 * GvCurrentRevision number should be incremented instead, and the
 * GvCurrentMinorRev should be reset to zero (see "geokeys.h").
 *
 * In addition, any changes here should be reflected in "geo_names.c"
 *
 */
 
#define GvCurrentMinorRev  0  /* First Major Rev EPSG Code Release  */


/*
 * Universal key values -- defined for consistency
 */
#define KvUndefined         0
#define KvUserDefined   32767

#ifdef ValuePair
#  undef ValuePair
#endif
#define ValuePair(name,value)    name = value,

/*
 * The section numbers refer to the GeoTIFF Specification section
 * in which the code values are documented.
 */
 
/************************************************************
 *         6.3.1 GeoTIFF General Codes
 ************************************************************/

/* 6.3.1.1 Model Type Codes */
typedef enum {
	ModelTypeProjected  = 1,  /* Projection Coordinate System */
	ModelTypeGeographic = 2,  /* Geographic latitude-longitude System */
	ModelTypeGeocentric = 3,   /* Geocentric (X,Y,Z) Coordinate System */
	ModelProjected  = ModelTypeProjected,   /* alias */
	ModelGeographic = ModelTypeGeographic,  /* alias */
	ModelGeocentric = ModelTypeGeocentric   /* alias */
} modeltype_t;

/* 6.3.1.2 Raster Type Codes */
typedef enum {
	RasterPixelIsArea   = 1,  /* Standard pixel-fills-grid-cell */
	RasterPixelIsPoint  = 2   /* Pixel-at-grid-vertex */
} rastertype_t;

typedef enum {
#  include "epsg_gcs.inc"
  geographic_end
} geographic_t;

typedef enum {
#  include "epsg_datum.inc"
   geodeticdatum_end
} geodeticdatum_t;

typedef enum {
#  include "epsg_units.inc"
   Unit_End
} geounits_t;

typedef enum {
#  include "epsg_ellipse.inc"
    ellipsoid_end
} ellipsoid_t;

typedef enum {
#  include "epsg_pm.inc"
   primemeridian_end
} primemeridian_t;

typedef enum {
#  include "epsg_pcs.inc"
   pcstype_end
} pcstype_t;

typedef enum {
#  include "epsg_proj.inc"
   projection_end
} projection_t;

typedef enum {
#  include "geo_ctrans.inc"
   coordtrans_end
} coordtrans_t;

typedef enum {
#  include "epsg_vertcs.inc"
   vertcs_end
} vertcstype_t;


typedef enum {
	VDatumBase = 1
} vdatum_t;

#endif /* __geovalues_h_ */

