/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.31
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace OSGeo.GDAL {

using System;
using System.Runtime.InteropServices;

public class ColorEntry : IDisposable {
  private HandleRef swigCPtr;
  protected object swigCMemOwner;
  
  protected static object ThisOwn_true() { return null; }
  protected object ThisOwn_false() { return this; }

  internal ColorEntry(IntPtr cPtr, object cMemoryOwner) {
    swigCMemOwner = cMemoryOwner;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(ColorEntry obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
  internal static HandleRef getCPtrAndDisown(ColorEntry obj, object cMemoryOwner) {
    obj.swigCMemOwner = cMemoryOwner;
    return getCPtr(obj);
  }

  ~ColorEntry() {
    Dispose();
  }

  public virtual void Dispose() {
  lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwner == null) {
        swigCMemOwner = new object();
        GdalPINVOKE.delete_ColorEntry(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public short c1 {
    set {
      GdalPINVOKE.ColorEntry_c1_set(swigCPtr, value);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      short ret = GdalPINVOKE.ColorEntry_c1_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public short c2 {
    set {
      GdalPINVOKE.ColorEntry_c2_set(swigCPtr, value);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      short ret = GdalPINVOKE.ColorEntry_c2_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public short c3 {
    set {
      GdalPINVOKE.ColorEntry_c3_set(swigCPtr, value);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      short ret = GdalPINVOKE.ColorEntry_c3_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public short c4 {
    set {
      GdalPINVOKE.ColorEntry_c4_set(swigCPtr, value);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      short ret = GdalPINVOKE.ColorEntry_c4_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public ColorEntry() : this(GdalPINVOKE.new_ColorEntry(), null) {
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
  }

}

}
