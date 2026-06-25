#pragma once

// 球
struct Sphere {
	Vector3 center;	//中心点
	float radius;	//半径
};

// 直線
struct Line {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};

// 半直線
struct Ray {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};


// 線分
struct Segment {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};

// 平面
struct Plane {
	Vector3 normal;	//法線
	float distance;	//距離
};

// 三角形
struct Triangle {
	Vector3 vertices[3];	//頂点
};

// AABB
struct AABB {
	Vector3 min;
	Vector3 max;
};