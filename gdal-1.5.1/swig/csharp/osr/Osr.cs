/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.31
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace OSGeo.OSR {

using System;
using System.Runtime.InteropServices;

public class Osr {

  internal class OsrObject : IDisposable {
	public virtual void Dispose() {
      
    }
  }
  internal static OsrObject theOsrObject = new OsrObject();
  protected static object ThisOwn_true() { return null; }
  protected static object ThisOwn_false() { return theOsrObject; }

  public static void UseExceptions() {
    OsrPINVOKE.UseExceptions();
  }

  public static void DontUseExceptions() {
    OsrPINVOKE.DontUseExceptions();
  }

  public static int GetWellKnownGeogCSAsWKT(string name, out string argout) {
    int ret = OsrPINVOKE.GetWellKnownGeogCSAsWKT(name, out argout);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static int GetUserInputAsWKT(string name, out string argout) {
    int ret = OsrPINVOKE.GetUserInputAsWKT(name, out argout);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static SWIGTYPE_p_p_char GetProjectionMethods() {
    IntPtr cPtr = OsrPINVOKE.GetProjectionMethods();
    SWIGTYPE_p_p_char ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_p_char(cPtr, ThisOwn_false());
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static SWIGTYPE_p_p_char GetProjectionMethodParameterList(string method, out string username) {
    IntPtr cPtr = OsrPINVOKE.GetProjectionMethodParameterList(method, out username);
    SWIGTYPE_p_p_char ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_p_char(cPtr, ThisOwn_false());
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static void GetProjectionMethodParamInfo(string method, string param, out string usrname, out string type, ref double defaultval) {
    OsrPINVOKE.GetProjectionMethodParamInfo(method, param, out usrname, out type, ref defaultval);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
  }

  public const string SRS_PT_ALBERS_CONIC_EQUAL_AREA = "Albers_Conic_Equal_Area";
  public const string SRS_PT_AZIMUTHAL_EQUIDISTANT = "Azimuthal_Equidistant";
  public const string SRS_PT_CASSINI_SOLDNER = "Cassini_Soldner";
  public const string SRS_PT_CYLINDRICAL_EQUAL_AREA = "Cylindrical_Equal_Area";
  public const string SRS_PT_BONNE = "Bonne";
  public const string SRS_PT_ECKERT_I = "Eckert_I";
  public const string SRS_PT_ECKERT_II = "Eckert_II";
  public const string SRS_PT_ECKERT_III = "Eckert_III";
  public const string SRS_PT_ECKERT_IV = "Eckert_IV";
  public const string SRS_PT_ECKERT_V = "Eckert_V";
  public const string SRS_PT_ECKERT_VI = "Eckert_VI";
  public const string SRS_PT_EQUIDISTANT_CONIC = "Equidistant_Conic";
  public const string SRS_PT_EQUIRECTANGULAR = "Equirectangular";
  public const string SRS_PT_GALL_STEREOGRAPHIC = "Gall_Stereographic";
  public const string SRS_PT_GEOSTATIONARY_SATELLITE = "Geostationary_Satellite";
  public const string SRS_PT_GOODE_HOMOLOSINE = "Goode_Homolosine";
  public const string SRS_PT_GNOMONIC = "Gnomonic";
  public const string SRS_PT_HOTINE_OBLIQUE_MERCATOR = "Hotine_Oblique_Mercator";
  public const string SRS_PT_HOTINE_OBLIQUE_MERCATOR_TWO_POINT_NATURAL_ORIGIN = "Hotine_Oblique_Mercator_Two_Point_Natural_Origin";
  public const string SRS_PT_LABORDE_OBLIQUE_MERCATOR = "Laborde_Oblique_Mercator";
  public const string SRS_PT_LAMBERT_CONFORMAL_CONIC_1SP = "Lambert_Conformal_Conic_1SP";
  public const string SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP = "Lambert_Conformal_Conic_2SP";
  public const string SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP_BELGIUM = "Lambert_Conformal_Conic_2SP_Belgium)";
  public const string SRS_PT_LAMBERT_AZIMUTHAL_EQUAL_AREA = "Lambert_Azimuthal_Equal_Area";
  public const string SRS_PT_MERCATOR_1SP = "Mercator_1SP";
  public const string SRS_PT_MERCATOR_2SP = "Mercator_2SP";
  public const string SRS_PT_MILLER_CYLINDRICAL = "Miller_Cylindrical";
  public const string SRS_PT_MOLLWEIDE = "Mollweide";
  public const string SRS_PT_NEW_ZEALAND_MAP_GRID = "New_Zealand_Map_Grid";
  public const string SRS_PT_OBLIQUE_STEREOGRAPHIC = "Oblique_Stereographic";
  public const string SRS_PT_ORTHOGRAPHIC = "Orthographic";
  public const string SRS_PT_POLAR_STEREOGRAPHIC = "Polar_Stereographic";
  public const string SRS_PT_POLYCONIC = "Polyconic";
  public const string SRS_PT_ROBINSON = "Robinson";
  public const string SRS_PT_SINUSOIDAL = "Sinusoidal";
  public const string SRS_PT_STEREOGRAPHIC = "Stereographic";
  public const string SRS_PT_SWISS_OBLIQUE_CYLINDRICAL = "Swiss_Oblique_Cylindrical";
  public const string SRS_PT_TRANSVERSE_MERCATOR = "Transverse_Mercator";
  public const string SRS_PT_TRANSVERSE_MERCATOR_SOUTH_ORIENTED = "Transverse_Mercator_South_Orientated";
  public const string SRS_PT_TRANSVERSE_MERCATOR_MI_21 = "Transverse_Mercator_MapInfo_21";
  public const string SRS_PT_TRANSVERSE_MERCATOR_MI_22 = "Transverse_Mercator_MapInfo_22";
  public const string SRS_PT_TRANSVERSE_MERCATOR_MI_23 = "Transverse_Mercator_MapInfo_23";
  public const string SRS_PT_TRANSVERSE_MERCATOR_MI_24 = "Transverse_Mercator_MapInfo_24";
  public const string SRS_PT_TRANSVERSE_MERCATOR_MI_25 = "Transverse_Mercator_MapInfo_25";
  public const string SRS_PT_TUNISIA_MINING_GRID = "Tunisia_Mining_Grid";
  public const string SRS_PT_TWO_POINT_EQUIDISTANT = "Two_Point_Equidistant";
  public const string SRS_PT_VANDERGRINTEN = "VanDerGrinten";
  public const string SRS_PT_KROVAK = "Krovak";
  public const string SRS_PP_CENTRAL_MERIDIAN = "central_meridian";
  public const string SRS_PP_SCALE_FACTOR = "scale_factor";
  public const string SRS_PP_STANDARD_PARALLEL_1 = "standard_parallel_1";
  public const string SRS_PP_STANDARD_PARALLEL_2 = "standard_parallel_2";
  public const string SRS_PP_PSEUDO_STD_PARALLEL_1 = "pseudo_standard_parallel_1";
  public const string SRS_PP_LONGITUDE_OF_CENTER = "longitude_of_center";
  public const string SRS_PP_LATITUDE_OF_CENTER = "latitude_of_center";
  public const string SRS_PP_LONGITUDE_OF_ORIGIN = "longitude_of_origin";
  public const string SRS_PP_LATITUDE_OF_ORIGIN = "latitude_of_origin";
  public const string SRS_PP_FALSE_EASTING = "false_easting";
  public const string SRS_PP_FALSE_NORTHING = "false_northing";
  public const string SRS_PP_AZIMUTH = "azimuth";
  public const string SRS_PP_LONGITUDE_OF_POINT_1 = "longitude_of_point_1";
  public const string SRS_PP_LATITUDE_OF_POINT_1 = "latitude_of_point_1";
  public const string SRS_PP_LONGITUDE_OF_POINT_2 = "longitude_of_point_2";
  public const string SRS_PP_LATITUDE_OF_POINT_2 = "latitude_of_point_2";
  public const string SRS_PP_LONGITUDE_OF_POINT_3 = "longitude_of_point_3";
  public const string SRS_PP_LATITUDE_OF_POINT_3 = "latitude_of_point_3";
  public const string SRS_PP_RECTIFIED_GRID_ANGLE = "rectified_grid_angle";
  public const string SRS_PP_LANDSAT_NUMBER = "landsat_number";
  public const string SRS_PP_PATH_NUMBER = "path_number";
  public const string SRS_PP_PERSPECTIVE_POINT_HEIGHT = "perspective_point_height";
  public const string SRS_PP_SATELLITE_HEIGHT = "satellite_height";
  public const string SRS_PP_FIPSZONE = "fipszone";
  public const string SRS_PP_ZONE = "zone";
  public const string SRS_PP_LATITUDE_OF_1ST_POINT = "Latitude_Of_1st_Point";
  public const string SRS_PP_LONGITUDE_OF_1ST_POINT = "Longitude_Of_1st_Point";
  public const string SRS_PP_LATITUDE_OF_2ND_POINT = "Latitude_Of_2nd_Point";
  public const string SRS_PP_LONGITUDE_OF_2ND_POINT = "Longitude_Of_2nd_Point";
  public const string SRS_UL_METER = "Meter";
  public const string SRS_UL_FOOT = "Foot (International)";
  public const string SRS_UL_FOOT_CONV = "0.3048";
  public const string SRS_UL_US_FOOT = "U.S. Foot";
  public const string SRS_UL_US_FOOT_CONV = "0.3048006";
  public const string SRS_UL_NAUTICAL_MILE = "Nautical Mile";
  public const string SRS_UL_NAUTICAL_MILE_CONV = "1852.0";
  public const string SRS_UL_LINK = "Link";
  public const string SRS_UL_LINK_CONV = "0.20116684023368047";
  public const string SRS_UL_CHAIN = "Chain";
  public const string SRS_UL_CHAIN_CONV = "20.116684023368047";
  public const string SRS_UL_ROD = "Rod";
  public const string SRS_UL_ROD_CONV = "5.02921005842012";
  public const string SRS_UA_DEGREE = "degree";
  public const string SRS_UA_DEGREE_CONV = "0.0174532925199433";
  public const string SRS_UA_RADIAN = "radian";
  public const string SRS_PM_GREENWICH = "Greenwich";
  public const string SRS_DN_NAD27 = "North_American_Datum_1927";
  public const string SRS_DN_NAD83 = "North_American_Datum_1983";
  public const string SRS_DN_WGS72 = "WGS_1972";
  public const string SRS_DN_WGS84 = "WGS_1984";
  public const double SRS_WGS84_SEMIMAJOR = 6378137.0;
  public const double SRS_WGS84_INVFLATTENING = 298.257223563;
}

}
