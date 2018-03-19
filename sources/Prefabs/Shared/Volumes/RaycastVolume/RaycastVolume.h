///* OpenCL/C++ */
//#pragma once
//#ifndef OpenCL
//#include "cldk.hpp"
//#endif
//
//#include "Vector/vec.h"
//
//#define MIN(a,b) ((a) < (b) ? (a) : (b))
//#define MAX(a,b) ((a) > (b) ? (a) : (b))
//
//inline void intersectBox(float3 boxmin, float3 boxmax, float3 p, 
//	float3 dir, float &tmin, float &tmax) {
//	float3 dirfrac;
//	dirfrac.x = 1.0f / dir.x;
//	dirfrac.y = 1.0f / dir.y;
//	dirfrac.z = 1.0f / dir.z;
//
//	float t1 = (boxmin.x - p.x) * dirfrac.x;
//	float t2 = (boxmax.x - p.x) * dirfrac.x;
//	float t3 = (boxmin.y - p.y) * dirfrac.y;
//	float t4 = (boxmax.y - p.y) * dirfrac.y;
//	float t5 = (boxmin.z - p.z) * dirfrac.z;
//	float t6 = (boxmax.z - p.z) * dirfrac.z;
//
//	tmin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));
//	tmax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));
//	//if (tmax < 0) // AABB is behind the ray
//	//	return false;
//
//	//if (tmin > tmax) // The ray misses the box
//	//	return false;
//
//	//if (tmin > hInfo.z)  // The box is behind something closer
//	//	return false;
//}
//
//// n: plane normal, p0 : point on plane, l0: ray origin, l: ray direction, t : output
//inline bool intersectPlane(
//	const float3 &n, const float3 &p0, const float3 &l0, const float3 &l, 
//	float tmin, float tmax, float &t)
//{
//	// Ray Plane Intersection
//	//    (p0 - l0) . n
//	//t = ------------- 
//	//        l . n
//	// parallel when l.n close to zero
//
//
//	// assuming vectors are all normalized
//	float denom = dot(n, l);
//	if (fabs(denom) > 1e-6) {
//		float3 p0l0 = p0 - l0;
//		t = dot(p0l0, n) / denom;
//		if (t >= tmin && t <= tmax) 
//			return true;
//	}
//
//	return false;
//}
//
///*! Returns the pseudoangle between the line p1 to (infinity, p1.y) and the
//line from p1 to p2. The pseudoangle has the property that the ordering of
//points by true angle anround p1 and ordering of points by pseudoangle are the
//same The result is in the range [0, 4) (or error -1). */
///* Shamelessly taken from https://www.opengl.org/discussion_boards/showthread.php/169527-Pseudo-angles */
//inline float pseudoangle(float2 p1, float2 p2) {
//	float2 delta = p2 - p1;
//	float result;
//
//	if ((delta.x == 0) && (delta.y == 0)) {
//		return -1;
//	}
//	else {
//		result = delta.y / (fabs(delta.x) + fabs(delta.y));
//
//		if (delta.x < 0.0) {
//			result = 2.0f - result;
//		}
//		else {
//			result = 4.0f + result;
//		}
//
//	}
//
//	return result;
//}
//
//// angle relative to the x axis. 
//inline float pseudoangle2(float2 p1, float2 p2) {
//	float dx, dy, t;
//	dx = p2.x - p1.x;
//	dy = p2.y - p1.y;
//	if ((dx == 0.0) && (dy == 0.0)) {
//		return -1; // indicating error
//	}
//	else {
//		t = dy / (fabs(dx) + fabs(dy));
//		/* Now correct for quadrant -- first quadrant: [0,1] */
//		if (dx < 0.0)
//			return 2.0 - t;
//		/* Inside second or third quadrant (1,3)*/
//		else if (dy < 0.0)
//			return 4.0 + t;
//		return t;
//	}
//}