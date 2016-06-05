/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.28
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.gdal.gdal;

public class MajorObject {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected MajorObject(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(MajorObject obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      throw new UnsupportedOperationException("C++ destructor does not have public access");
    }
    swigCPtr = 0;
  }

  protected static long getCPtrAndDisown(MajorObject obj) {
    if (obj != null) obj.swigCMemOwn= false;
    return getCPtr(obj);
  }

  public String GetDescription() {
    return gdalJNI.MajorObject_GetDescription(swigCPtr);
  }

  public void SetDescription(String pszNewDesc) {
    gdalJNI.MajorObject_SetDescription(swigCPtr, pszNewDesc);
  }

  public java.util.Hashtable GetMetadata_Dict(String pszDomain) {
    return gdalJNI.MajorObject_GetMetadata_Dict(swigCPtr, pszDomain);
  }

  public java.util.Vector GetMetadata_List(String pszDomain) {
    return gdalJNI.MajorObject_GetMetadata_List(swigCPtr, pszDomain);
  }

  public int SetMetadata(java.util.Hashtable papszMetadata, String pszDomain) {
    return gdalJNI.MajorObject_SetMetadata__SWIG_0(swigCPtr, papszMetadata, pszDomain);
  }

  public int SetMetadata(String pszMetadataString, String pszDomain) {
    return gdalJNI.MajorObject_SetMetadata__SWIG_1(swigCPtr, pszMetadataString, pszDomain);
  }

}
