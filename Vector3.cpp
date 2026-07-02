#include "Vector3.h"
#include <Novice.h>
#include <math.h>

static const int kColumnWidth = 60;
static const int kRowHeight = 20;

// 三次元ベクトルの加算
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

// 三次元ベクトルの減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

// 三次元ベクトルのスカラー倍
Vector3 Multiply(float scalar, const Vector3& v) {
	Vector3 result;
	result.x = scalar * v.x;
	result.y = scalar * v.y;
	result.z = scalar * v.z;
	return result;
}

// 三次元ベクトルの内積
float Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// 三次元ベクトルの長さ
float Length(const Vector3& v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// 三次元ベクトルの正規化
Vector3 Normalize(const Vector3& v) {
	Vector3 result;
	float length = Length(v);
	if (length != 0.0f) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}
	return result;
}

// 三次元ベクトルの数値表示
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

// 複合代入演算子
Vector3& Vector3::operator+=(const Vector3& v) {
	x += v.x; y += v.y; z += v.z;
	return *this;
}
Vector3& Vector3::operator-=(const Vector3& v) {
	x -= v.x; y -= v.y; z -= v.z;
	return *this;
}
Vector3& Vector3::operator*=(float s) {
	x *= s; y *= s; z *= s;
	return *this;
}
Vector3& Vector3::operator/=(float s) {
	x /= s; y /= s; z /= s;
	return *this;
}

// 単項演算子 (符号反転)
Vector3 operator-(const Vector3& v) {
	return { -v.x, -v.y, -v.z };
}

// 2項演算子
Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}
Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}
Vector3 operator*(const Vector3& v, float s) {
	return { v.x * s, v.y * s, v.z * s };
}
Vector3 operator*(float s, const Vector3& v) {
	return { s * v.x, s * v.y, s * v.z };
}
Vector3 operator/(const Vector3& v, float s) {
	return { v.x / s, v.y / s, v.z / s };
}