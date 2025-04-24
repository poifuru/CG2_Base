#include "Matrix.h"
#include <cmath>

Vector3 Add(const Vector3& v1, const Vector3& v2) {
	return {
		v1.x + v1.x,
		v1.y + v2.y,
		v1.z + v2.z
	};
}

Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	return {
		v1.x - v1.x,
		v1.y - v2.y,
		v1.z - v2.z
	};
}

Vector3 Multiply(float scalar, const Vector3& v) {
	return {
		scalar * v.x,
		scalar * v.y,
		scalar * v.z
	};
}

float Dot(const Vector3& v1, const Vector3& v2) {
	return {
		(v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z)
	};
}

float Length(const Vector3& v) {
	return {
		sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2))
	};
}

Vector3 Normalize(const Vector3& v) {
	float nor = sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2));

	return {
		v.x / nor,
		v.y / nor,
		v.z / nor
	};
}

Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result;

	for (int i = 0; i < 4; ++i) {         // 行
		for (int j = 0; j < 4; ++j) {     // 列
			result.m[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k) { // 積の途中経過
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}

	return result;
}

//逆行列補助用の関数
float Minor(const Matrix4x4& mat, int row, int col) {
	float sub[3][3];
	int r = 0, c = 0;

	for (int i = 0; i < 4; ++i) {
		if (i == row) continue;
		c = 0;
		for (int j = 0; j < 4; ++j) {
			if (j == col) continue;
			sub[r][c] = mat.m[i][j];
			++c;
		}
		++r;
	}

	// 3x3行列の行列式を計算
	return
		sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1]) -
		sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0]) +
		sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);
}

// 4x4行列の行列式
float Determinant(const Matrix4x4& mat) {
	float det = 0.0f;
	for (int col = 0; col < 4; ++col) {
		float sign = (col % 2 == 0) ? 1.0f : -1.0f;
		det += sign * mat.m[0][col] * Minor(mat, 0, col);
	}
	return det;
}

Matrix4x4 Inverse(const Matrix4x4& m)
{
	Matrix4x4 result;
	float det = Determinant(m);

	// ゼロ除算防止（特異行列）
	if (std::fabs(det) < 1e-6f) {
		// ゼロ行列返す
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				result.m[i][j] = 0.0f;
		return result;
	}

	// 余因子行列（cofactor matrix）
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			float sign = ((i + j) % 2 == 0) ? 1.0f : -1.0f;
			result.m[j][i] = sign * Minor(m, i, j); // 転置して代入（adjugate）
		}
	}

	// 行列式で割る（スカラー倍）
	float invDet = 1.0f / det;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] *= invDet;

	return result;
}

Matrix4x4 Transpose(const Matrix4x4& m)
{
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

Matrix4x4 MakeIdentity4x4()
{
	Matrix4x4 result = { 0.0f };

	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1.0f; // 対角成分だけ 1 にする
	}

	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 result = { 0.0f };

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale)
{
	Matrix4x4 result = { 0.0f };

	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MakeRotateXMatrix(float radian)
{
	Matrix4x4 result = MakeIdentity4x4();

	result.m[1][1] = cosf(radian);
	result.m[1][2] = sinf(radian);
	result.m[2][1] = -sinf(radian);
	result.m[2][2] = cosf(radian);

	return result;
}

Matrix4x4 MakeRotateYMatrix(float radian)
{
	Matrix4x4 result = MakeIdentity4x4();

	result.m[0][0] = cosf(radian);
	result.m[0][2] = -sinf(radian);
	result.m[2][0] = sinf(radian);
	result.m[2][2] = cosf(radian);

	return result;
}

Matrix4x4 MakeRotateZMatrix(float radian)
{
	Matrix4x4 result = MakeIdentity4x4();

	result.m[0][0] = cosf(radian);
	result.m[0][1] = sinf(radian);
	result.m[1][0] = -sinf(radian);
	result.m[1][1] = cosf(radian);

	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
{
	Matrix4x4 matrix = MakeIdentity4x4();

	Matrix4x4 scaleMatrix = Multiply(matrix, MakeScaleMatrix(scale));
	Matrix4x4 rotateXMatrix = Multiply(scaleMatrix, MakeRotateXMatrix(rotate.x));
	Matrix4x4 rotateXYMatrix = Multiply(rotateXMatrix, MakeRotateYMatrix(rotate.y));
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXYMatrix, MakeRotateZMatrix(rotate.z));
	Matrix4x4 translateMatrix = Multiply(rotateXYZMatrix, MakeTranslateMatrix(translate));

	return translateMatrix;
}

Matrix4x4 MakePerspectiveFOVMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 result = { 0.0f };

	result.m[0][0] = (1 / aspectRatio) * (1 / tanf(fovY / 2));
	result.m[1][1] = 1 / tanf(fovY / 2);
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);

	return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
{
	Matrix4x4 result = MakeIdentity4x4();

	result.m[0][0] = 2 / (right - left);
	result.m[1][1] = 2 / (top - bottom);
	result.m[2][2] = 1 / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);

	return result;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 result = MakeIdentity4x4();

	result.m[0][0] = (width / 2);
	result.m[1][1] = -(height / 2);
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + (width / 2);
	result.m[3][1] = top + (height / 2);
	result.m[3][2] = minDepth;

	return result;
}