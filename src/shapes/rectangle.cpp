
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2015 Marwan Abdellah.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */


// shapes/rectangle.cpp*
#include "stdafx.h"
#include "shapes/rectangle.h"
#include "rectangle.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{

// Rectangle Method Definitions
Rectangle::Rectangle(const Transform *o2w, const Transform *w2o, bool ro,
                     float ht, float x, float y)
    : Shape(o2w, w2o, ro) {
    height = ht;
    xlength = x;
    ylength = y;
}


BBox Rectangle::ObjectBound() const {
    return BBox(Point(-xlength/2, -ylength/2, height),
                Point(xlength/2, ylength/2, height));
}


// Checks if the ray _r_ intersects with the rectangle or not, and if yes,
// it returns the intersection point along the ray _tHit_
bool Rectangle::Intersect(const Ray &r, float *tHit, float *rayEpsilon,
                          DifferentialGeometry *dg) const {
    // Transform _Ray_ to object space
    Ray ray;
    (*WorldToObject)(r, &ray);

    // Compute plane intersection for the rectangle
    if (fabsf(ray.d.z) < 1e-7) return false;
    float thit = (- ray.o.z) / ray.d.z;
    if (thit < ray.mint || thit > ray.maxt)
        return false;

    // See if hit point is inside the rectangle
    Point phit = ray(thit);
    if (phit.x > xlength/2 || phit.x < -xlength/2 ||
            phit.y > ylength/2 || phit.y < -ylength/2)
        return false;

    float u = phit.x/xlength + 0.5f;
    float v = phit.y/ylength + 0.5f;

    Vector dpdu(xlength, 0, 0);
    Vector dpdv(0, ylength, 0);
    Normal dndu(0,0,0), dndv(0,0,0);

    // Initialize _DifferentialGeometry_ from parametric information
    *dg = DifferentialGeometry((*ObjectToWorld)(phit),
                               (*ObjectToWorld)(dpdu),
                               (*ObjectToWorld)(dpdv),
                               (*ObjectToWorld)(dndu),
                               (*ObjectToWorld)(dndv),
                               u, v, this);

    // Update _tHit_ for quadric intersection
    *tHit = thit;
    return true;
}


// Checks if the ray intersects the rectangle or not.
bool Rectangle::IntersectP(const Ray &r) const {
    // Transform _Ray_ to object space
    Ray ray;
    (*WorldToObject)(r, &ray);

    // Compute plane intersection for the rectangle
    if (fabsf(ray.d.z) < 1e-7) return false;
    float thit = (- ray.o.z) / ray.d.z;
    if (thit < ray.mint || thit > ray.maxt)
        return false;

    // See if hit point is inside the rectangle
    Point phit = ray(thit);
    if (phit.x > xlength/2.f || phit.x < -xlength/2.f ||
        phit.y > ylength/2.f || phit.y < -ylength/2.f)
        return false;
    return true;
}


float Rectangle::Area() const {
    return xlength * ylength;
}


Point Rectangle::Sample(float u1, float u2, Normal *Ns) const {
    Point p;
    p.x = xlength * (u1 - 0.5);
    p.y = ylength * (u2 - 0.5);
    p.z = height;
    *Ns = Normalize((*ObjectToWorld)(Normal(0,0,1)));
    if (ReverseOrientation) *Ns *= -1.f;
    return (*ObjectToWorld)(p);
}


Rectangle *CreateRectangleShape(const Transform *o2w, const Transform *w2o,
                                bool reverseOrientation, const ParamSet &params) {
    float height = params.FindOneFloat("height", 0.f);
    float x = params.FindOneFloat("x", 1.f);
    float y = params.FindOneFloat("y", 1.f);
    return new Rectangle(o2w, w2o, reverseOrientation, height, x, y);
}


bool Rectangle::Projects(const Point &p, Point &ps, Normal &ns) const
{
    /// Get the point in the local coordinates of the shape
        Point Pobj;
        (*WorldToObject)(p, &Pobj);

        if (Pobj.x <= xlength/2 && Pobj.x >= -xlength/2 &&
                Pobj.y <= ylength/2 && Pobj.y >= -ylength/2 && Pobj.z > 0) {
            ps.x = Pobj.x;
            ps.y = Pobj.y;
            ps.z = 0;

            ps = (*ObjectToWorld)(ps);
            ns = Normalize((*ObjectToWorld)(Normal(0,0,1)));
            return true;
        }

        return false;
}

}

//    // Transform the point _p_ to the object space _Pobj_
//    Point Pobj;
//    (*WorldToObject)(p, &Pobj);
//    if (Pobj.x > xlength / 2.f || Pobj.x < -xlength / 2.f ||
//        Pobj.y > ylength / 2.f || Pobj.y < -ylength / 2.f)
//        return false;

//    // Return the position of the sample on the light source.
//    ps.x = p.x; ps.y = p.y; ps.z = 0;
//    ns = Normalize((*ObjectToWorld)(Normal(0,0,1)));
//    return true;
//}
