//////////////////////////////// -*- C++ -*- //////////////////////////////
//
// AUTHOR
//    Peter Shirley, Cornell University, shirley@graphics.cornell.edu 
//
// COPYRIGHT
//    Copyright (c) 1995  Peter Shirley. All rights reserved.
//
//    Permission to use, copy, modify, and distribute this software for any
//    purpose without fee is hereby granted, provided that this entire
//    notice is included in all copies of any software which is or includes
//    a copy or modification of this software and in all copies of the
//    a copy or modification of this software and in all copies of the
//    supporting documentation for such software.
//
// DISCLAIMER
//    Neither the Peter Shirley nor Cornell nor any of their
//    employees, makes any warranty, express or implied, or assumes any
//    liability or responsibility for the accuracy, completeness, or
//    usefulness of any information, apparatus, product, or process
//    disclosed, or represents that its use would not infringe
//    privately-owned rights.  
//
///////////////////////////////////////////////////////////////////////////



#ifndef MRSOLIDTEXTURE_H
#define MRSOLIDTEXTURE_H


#include <mrSurface.h>
#include <ggSolidTexture.h>

class mrSolidTexture : public mrSurface {
public:


  virtual ggBoolean shadowHit(        
             const ggRay3& r,    // ray being sent
             double time,     // time ray is sent
             double tmin, // minimum hit parameter to be searched for
             double tmax, // maximum hit parameter to be searched for
             double& t,
             ggVector3& N,  
             ggBoolean& emits,
             ggSpectrum& kEmit
             ) const;

  virtual ggBoolean viewingHit(        
             const ggRay3& r,    // ray being sent
             double time,     // time ray is sent
             double tmin, // minimum hit parameter to be searched for
             double tmax, // maximum hit parameter to be searched for
             mrViewingHitRecord&,
             ggMaterialRecord& rec
             ) const;

  //  returns true if bounding box is found
  virtual ggBoolean boundingBox(
             double time1, // low end of time range
             double time2, // low end of time range
             ggBox3& bbox)
             const;

     // Selects a point visible from x given a uv-pair.  Sometimes
     // returning a non-visible point is allowed, but not desirable.
     // Here, visible means not SELF-sahdowed.
     virtual ggBoolean selectVisiblePoint(
             const ggPoint3& x,   // viewpoint
             const ggVector3& Nx, // unit vector at x
             const ggPoint2& uv,   // input coordinate
             const double time,// time of query
             ggPoint3& on_light,  // point corresponding to uv
             double& prob)     // probability of selecting on_light
             const;
                               // assuming random uv
     mrSolidTexture (
                  mrSurface *ob_ptr,
                  ggSolidTexture *tex_ptr) 
                  {objPtr = ob_ptr;  texPtr = tex_ptr; }

  protected:
     mrSolidTexture () {}
     mrSurface *objPtr;
     ggSolidTexture *texPtr;
};


#endif
