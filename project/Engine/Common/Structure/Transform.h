#pragma once
// EulerTransform
struct EulerTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// Quaternion
struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};