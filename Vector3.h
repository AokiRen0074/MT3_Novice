#pragma once
#include <Novice.h>

class Vector3 {
	
public:

		float x;
		float y;
		float z;

		Vector3& operator+=(const Vector3& v);
		Vector3& operator-=(const Vector3& v);
		Vector3& operator*=(float s);
		Vector3& operator/=(float s);

		// コンストラクタ
		Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

// 三次元ベクトルの加算
Vector3 Add(const Vector3& v1, const Vector3& v2);

// 三次元ベクトルの減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

// 三次元ベクトルのスカラー倍
Vector3 Multiply(float scalar, const Vector3& v);

// 三次元ベクトルの内積
float Dot(const Vector3& v1, const Vector3& v2);

// 三次元ベクトルの長さ
float Length(const Vector3& v);

// 三次元ベクトルの正規化
Vector3 Normalize(const Vector3& v);

// 三次元ベクトルの数値表示
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label);



// 単項演算子
Vector3 operator-(const Vector3& v);

// 2項演算子
Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator*(float s, const Vector3& v);
Vector3 operator/(const Vector3& v, float s);