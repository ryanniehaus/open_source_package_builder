/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.28
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.gdal.ogr;

public class Driver {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected Driver(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Driver obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      throw new UnsupportedOperationException("C++ destructor does not have public access");
    }
    swigCPtr = 0;
  }

  protected static long getCPtrAndDisown(Driver obj) {
    if (obj != null) obj.swigCMemOwn= false;
    return getCPtr(obj);
  }

  public String getName() {
    return ogrJNI.Driver_name_get(swigCPtr);
  }

  public DataSource CreateDataSource(String name, java.util.Vector options) {
    long cPtr = ogrJNI.Driver_CreateDataSource(swigCPtr, name, options);
    return (cPtr == 0) ? null : new DataSource(cPtr, true);
  }

  public DataSource CopyDataSource(DataSource copy_ds, String name, java.util.Vector options) {
    long cPtr = ogrJNI.Driver_CopyDataSource(swigCPtr, DataSource.getCPtr(copy_ds), name, options);
    return (cPtr == 0) ? null : new DataSource(cPtr, true);
  }

  public DataSource Open(String name, int update) {
    long cPtr = ogrJNI.Driver_Open(swigCPtr, name, update);
    return (cPtr == 0) ? null : new DataSource(cPtr, true);
  }

  public int DeleteDataSource(String name) {
    return ogrJNI.Driver_DeleteDataSource(swigCPtr, name);
  }

  public boolean TestCapability(String cap) {
    return ogrJNI.Driver_TestCapability(swigCPtr, cap);
  }

  public String GetName() {
    return ogrJNI.Driver_GetName(swigCPtr);
  }

}
