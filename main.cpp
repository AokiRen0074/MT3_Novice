#include <Novice.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cstdint>
#include<imgui.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

const char kWindowTitle[] = "LC1B_01_アオキレン_タイトル";

static const int kRowHeight = 20;
static const int kColumnWidth = 60;

// ウィンドサイズ
const int kWindowWidth = 1280;
const int kWindowHeight = 720;

struct Sphere {
	Vector3 center;
	float radius;
};

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

// 平面
struct Plane {
	Vector3 normal; // 法線
	float distance; //原点からの距離
};

// 三角形
struct Triangle {
	Vector3 vertices[3];// 三つの頂点
};

// AABB判定
struct AABB {
	Vector3 min;// 最小点
	Vector3 max;// 最大点
};

// OBB
struct OBB {
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};

// 線形補完
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return {
	v1.x + (v2.x - v1.x) * t,
	v1.y + (v2.y - v1.y) * t,
	v1.z + (v2.z - v1.z) * t
	};
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;                                      // Gridの半分の幅
	const uint32_t kSubdivision = 10;                                       // 分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision); // 1つ分の長さ


	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + (xIndex * kGridEvery);

		Vector3 startWorld = { x, 0.0f, kGridHalfWidth };  // 奥側の点
		Vector3 endWorld = { x, 0.0f, -kGridHalfWidth }; // 手前側の点

		Vector3 startScreen = Transform(startWorld, viewProjectionMatrix);
		Vector3 endScreen = Transform(endWorld, viewProjectionMatrix);

		startScreen = Transform(startScreen, viewportMatrix);
		endScreen = Transform(endScreen, viewportMatrix);

		// 色
		unsigned int color = (xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;

		Novice::DrawLine(
			int(startScreen.x), int(startScreen.y),
			int(endScreen.x), int(endScreen.y),
			color
		);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);

		// ワールド座標系上の始点と終点を求める
		Vector3 startWorld = { -kGridHalfWidth, 0.0f, z }; // 左側の点
		Vector3 endWorld = { kGridHalfWidth, 0.0f, z };  // 右側の点

		// スクリーン座標系まで変換をかける
		Vector3 startScreen = Transform(startWorld, viewProjectionMatrix);
		Vector3 endScreen = Transform(endWorld, viewProjectionMatrix);
		startScreen = Transform(startScreen, viewportMatrix);
		endScreen = Transform(endScreen, viewportMatrix);

		// 色の決定
		unsigned int color = (zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;

		Novice::DrawLine(
			int(startScreen.x), int(startScreen.y),
			int(endScreen.x), int(endScreen.y),
			color
		);
	}
}

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 16;
	const float kLonEvery = (2.0f * (float)M_PI) / kSubdivision;
	const float kLatEvery = (float)M_PI / kSubdivision;

	// 緯度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;
		float nextLat = lat + kLatEvery;

		// 経度の方向に分割
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;
			float nextLon = lon + kLonEvery;

			// world座標系での a, b, c を求める
			Vector3 a, b, c;

			// 点 a
			a.x = sphere.center.x + sphere.radius * std::cos(lat) * std::cos(lon);
			a.y = sphere.center.y + sphere.radius * std::sin(lat);
			a.z = sphere.center.z + sphere.radius * std::cos(lat) * std::sin(lon);

			// 点 b
			b.x = sphere.center.x + sphere.radius * std::cos(nextLat) * std::cos(lon);
			b.y = sphere.center.y + sphere.radius * std::sin(nextLat);
			b.z = sphere.center.z + sphere.radius * std::cos(nextLat) * std::sin(lon);

			// 点 c
			c.x = sphere.center.x + sphere.radius * std::cos(lat) * std::cos(nextLon);
			c.y = sphere.center.y + sphere.radius * std::sin(lat);
			c.z = sphere.center.z + sphere.radius * std::cos(lat) * std::sin(nextLon);


			// a, b, c をScreen座標系まで変換
			Vector3 screenA = Transform(a, viewProjectionMatrix);
			screenA = Transform(screenA, viewportMatrix);

			Vector3 screenB = Transform(b, viewProjectionMatrix);
			screenB = Transform(screenB, viewportMatrix);

			Vector3 screenC = Transform(c, viewProjectionMatrix);
			screenC = Transform(screenC, viewportMatrix);


			// ab, ac で線を引く
			Novice::DrawLine(
				int(screenA.x), int(screenA.y),
				int(screenB.x), int(screenB.y),
				color
			);

			Novice::DrawLine(
				int(screenA.x), int(screenA.y),
				int(screenC.x), int(screenC.y),
				color
			);
		}
	}
}


// 分離軸をみつける
bool TestSeparatingAxis(const Vector3& axis, const OBB& obb1, const OBB& obb2) {
	
	float lengthSq = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
	if (lengthSq <= 0.00001f) return false;

	// 軸を正規化
	Vector3 nAxis = { axis.x / std::sqrt(lengthSq), axis.y / std::sqrt(lengthSq), axis.z / std::sqrt(lengthSq) };

	// 中心間の距離ベクトルを軸に投影
	Vector3 centerDiff = { obb2.center.x - obb1.center.x, obb2.center.y - obb1.center.y, obb2.center.z - obb1.center.z };
	float distance = std::abs(centerDiff.x * nAxis.x + centerDiff.y * nAxis.y + centerDiff.z * nAxis.z);

	//  OBB1の投影半径
	float r1 = obb1.size.x * std::abs(obb1.orientations[0].x * nAxis.x + obb1.orientations[0].y * nAxis.y + obb1.orientations[0].z * nAxis.z) +
		obb1.size.y * std::abs(obb1.orientations[1].x * nAxis.x + obb1.orientations[1].y * nAxis.y + obb1.orientations[1].z * nAxis.z) +
		obb1.size.z * std::abs(obb1.orientations[2].x * nAxis.x + obb1.orientations[2].y * nAxis.y + obb1.orientations[2].z * nAxis.z);

	// OBB2の投影半径
	float r2 = obb2.size.x * std::abs(obb2.orientations[0].x * nAxis.x + obb2.orientations[0].y * nAxis.y + obb2.orientations[0].z * nAxis.z) +
		obb2.size.y * std::abs(obb2.orientations[1].x * nAxis.x + obb2.orientations[1].y * nAxis.y + obb2.orientations[1].z * nAxis.z) +
		obb2.size.z * std::abs(obb2.orientations[2].x * nAxis.x + obb2.orientations[2].y * nAxis.y + obb2.orientations[2].z * nAxis.z);


	return distance > (r1 + r2);
}

// 球と球の当たり判定
bool IsCollision(const Sphere& s1, const Sphere& s2) {
	// 中心点間の距離を求める
	Vector3 diff;
	diff.x = s2.center.x - s1.center.x;
	diff.y = s2.center.y - s1.center.y;
	diff.z = s2.center.z - s1.center.z;

	// Length関数相当の距離計算
	float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

	// 距離が半径の和よりも小さければ衝突
	if (distance <= s1.radius + s2.radius) {
		return true;
	}
	return false;
}

// 球と面の当たり判定
bool IsCollision(const Sphere& sphere, const Plane& plane) {
	// 球の中心と平面の法線の内積を計算
	float dot = sphere.center.x * plane.normal.x +
		sphere.center.y * plane.normal.y +
		sphere.center.z * plane.normal.z;

	// 平面と球の中心との距離を求める
	float distance = std::abs(dot - plane.distance);

	// 距離が半径以下なら衝突
	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

// 線分と面の当たり判定
bool IsCollision(const Segment& segment, const Plane& plane) {
	//ず垂直判定を行うために、法線と線の内積を求める
	float dot = plane.normal.x * segment.diff.x +
		plane.normal.y * segment.diff.y +
		plane.normal.z * segment.diff.z;

	if (dot == 0.0f) {
		return false;
	}

	// 始点と法線の内積を求める
	float originDot = segment.origin.x * plane.normal.x +
		segment.origin.y * plane.normal.y +
		segment.origin.z * plane.normal.z;

	// tを求める
	float t = (plane.distance - originDot) / dot;

	if (t >= 0.0f && t <= 1.0f) {
		return true; // 衝突している
	}

	return false; // 衝突していない
}

// 三角形と線の当たり判定
bool IsCollision(const Triangle& triangle, const Segment& segment) {

	// 三角形の2つの辺のベクトルを求める
	Vector3 v01 = { triangle.vertices[1].x - triangle.vertices[0].x, triangle.vertices[1].y - triangle.vertices[0].y, triangle.vertices[1].z - triangle.vertices[0].z };
	Vector3 v12 = { triangle.vertices[2].x - triangle.vertices[1].x, triangle.vertices[2].y - triangle.vertices[1].y, triangle.vertices[2].z - triangle.vertices[1].z };

	//  外積で三角形の法線を求める
	Vector3 normal = Normalize(Cross(v01, v12));

	// 原点から平面までの距離を求める
	float distance = Dot(triangle.vertices[0], normal);

	// 線分と平面の衝突判定
	float dot = Dot(normal, segment.diff);
	if (dot == 0.0f) return false; // 平行

	float t = (distance - Dot(segment.origin, normal)) / dot;
	if (t < 0.0f || t > 1.0f) return false; // 線分の長さの範囲外

	// 交点 p の座標を計算
	Vector3 p = { segment.origin.x + segment.diff.x * t, segment.origin.y + segment.diff.y * t, segment.origin.z + segment.diff.z * t };

	// 各頂点から交点 p へのベクトル
	Vector3 v0p = { p.x - triangle.vertices[0].x, p.y - triangle.vertices[0].y, p.z - triangle.vertices[0].z };
	Vector3 v1p = { p.x - triangle.vertices[1].x, p.y - triangle.vertices[1].y, p.z - triangle.vertices[1].z };
	Vector3 v2p = { p.x - triangle.vertices[2].x, p.y - triangle.vertices[2].y, p.z - triangle.vertices[2].z };

	// 各辺のベクトル
	Vector3 v20 = { triangle.vertices[0].x - triangle.vertices[2].x, triangle.vertices[0].y - triangle.vertices[2].y, triangle.vertices[0].z - triangle.vertices[2].z };

	//　外積を取る
	Vector3 cross01 = Cross(v01, v0p);
	Vector3 cross12 = Cross(v12, v1p);
	Vector3 cross20 = Cross(v20, v2p);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突
	if (Dot(cross01, normal) >= 0.0f &&
		Dot(cross12, normal) >= 0.0f &&
		Dot(cross20, normal) >= 0.0f) {
		return true; // 衝突！
	}

	return false;
}

// AABBの当たり判定
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x) return false;
	if (aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y) return false;
	if (aabb1.max.z < aabb2.min.z || aabb1.min.z > aabb2.max.z) return false;
	return true; // 衝突している
}

// AABBと球の当たり判定
bool IsCollision(const AABB& aabb,const Sphere& sphere) {
	// 球の中心点とAABBの最近点を求める
	Vector3 cloosePoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)
	};

	// 最近点と球の中心点の距離を求める
	float distanceX = cloosePoint.x - sphere.center.x;
	float distanceY = cloosePoint.y - sphere.center.y;
	float distanceZ = cloosePoint.z - sphere.center.z;

	float distanceSquared = distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ;
	// 距離が半径の二乗以下なら衝突
	return distanceSquared <= (sphere.radius * sphere.radius);
}

// AABBと線分の衝突判定
bool IsCollision(const AABB& aabb, const Segment& segment) {
	float tNearX, tFarX, tNearY, tFarY, tNearZ, tFarZ;

	// --- X軸の判定 ---
	if (segment.diff.x == 0.0f) {
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x) return false;
		tNearX = -INFINITY;
		tFarX = INFINITY;
	}
	else {
		float t1 = (aabb.min.x - segment.origin.x) / segment.diff.x;
		float t2 = (aabb.max.x - segment.origin.x) / segment.diff.x;
		tNearX = (std::min)(t1, t2);
		tFarX = (std::max)(t1, t2);
	}

	// --- Y軸の判定 ---
	if (segment.diff.y == 0.0f) {
		if (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y) return false;
		tNearY = -INFINITY;
		tFarY = INFINITY;
	}
	else {
		float t1 = (aabb.min.y - segment.origin.y) / segment.diff.y;
		float t2 = (aabb.max.y - segment.origin.y) / segment.diff.y;
		tNearY = (std::min)(t1, t2);
		tFarY = (std::max)(t1, t2);
	}

	// --- Z軸の判定 ---
	if (segment.diff.z == 0.0f) {
		if (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z) return false;
		tNearZ = -INFINITY;
		tFarZ = INFINITY;
	}
	else {
		float t1 = (aabb.min.z - segment.origin.z) / segment.diff.z;
		float t2 = (aabb.max.z - segment.origin.z) / segment.diff.z;
		tNearZ = (std::min)(t1, t2);
		tFarZ = (std::max)(t1, t2);
	}


	float tmin = (std::max)((std::max)(tNearX, tNearY), tNearZ);

	float tmax = (std::min)((std::min)(tFarX, tFarY), tFarZ);


	if (tmin > tmax) {
		return false;
	}

	// --- 線の種類ごとの判定 ---
	if (tmax < 0.0f) {
		return false;
	}

	if (tmin > 1.0f) {
		return false;
	}

	return true;
}

// OBBと球の当たり判定
bool IsCollision(const OBB& obb, const Sphere& sphere) {
	Matrix4x4 obbWorldMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f,
		obb.center.x,          obb.center.y,          obb.center.z,          1.0f
	};

	// OBBのワールド行列の逆行列
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);

	// 球の中心をOBBのローカル空間に変換する
	Vector3 centerInOBBLocalSpace = Transform(sphere.center, obbWorldMatrixInverse);


	AABB aabbOBBLocal{
			.min = {-(std::abs)(obb.size.x), -(std::abs)(obb.size.y), -(std::abs)(obb.size.z)},
			.max = { (std::abs)(obb.size.x),  (std::abs)(obb.size.y),  (std::abs)(obb.size.z)}
	};

	Sphere sphereOBBLocal{
		.center = centerInOBBLocalSpace,
		.radius = sphere.radius
	};

	return IsCollision(aabbOBBLocal, sphereOBBLocal);
}

// OBBと線分の当たり判定
bool IsCollision(const OBB& obb, const Segment& segment) {
	// OBBのワールド行列を作成し、その逆行列を求める
	Matrix4x4 obbWorldMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f,
		obb.center.x,          obb.center.y,          obb.center.z,          1.0f
	};
	Matrix4x4 obbInverse = Inverse(obbWorldMatrix);

	// 線分の始点とをローカル空間に変換する
	Vector3 localOrigin = Transform(segment.origin, obbInverse);

	Vector3 end = {
		segment.origin.x + segment.diff.x,
		segment.origin.y + segment.diff.y,
		segment.origin.z + segment.diff.z
	};
	Vector3 localEnd = Transform(end, obbInverse);

	// ローカル空間での新しいSegmentを作る
	Segment localSegment;
	localSegment.origin = localOrigin;
	// 再度、差分を計算し直す
	localSegment.diff = {
		localEnd.x - localOrigin.x,
		localEnd.y - localOrigin.y,
		localEnd.z - localOrigin.z
	};

	// OBBをローカル空間のAABBとして定義する
	AABB localAABB{
		.min = {-(std::abs)(obb.size.x), -(std::abs)(obb.size.y), -(std::abs)(obb.size.z)},
		.max = { (std::abs)(obb.size.x),  (std::abs)(obb.size.y),  (std::abs)(obb.size.z)}
	};

	return IsCollision(localAABB, localSegment);
}

// OBBとOBBの当たり判定
bool IsCollision(const OBB& obb1, const OBB& obb2) {
	// 15本の軸を格納する配列
	Vector3 axes[15];

	// OBB1のローカル軸 
	axes[0] = obb1.orientations[0];
	axes[1] = obb1.orientations[1];
	axes[2] = obb1.orientations[2];

	// OBB2のローカル軸 
	axes[3] = obb2.orientations[0];
	axes[4] = obb2.orientations[1];
	axes[5] = obb2.orientations[2];

	// OBB1とOBB2のローカル軸のクロス積 
	int axisIndex = 6;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			axes[axisIndex++] = Cross(obb1.orientations[i], obb2.orientations[j]);
		}
	}

	// 15本の軸すべてに対して、分離軸になっているか
	for (int i = 0; i < 15; ++i) {
		// 分離軸が見つかったら、隙間があるので衝突していない
		if (TestSeparatingAxis(axes[i], obb1, obb2)) {
			return false;
		}
	}

	// すべての軸で影が重なっていたら、衝突している
	return true;
}

//法線と垂直なベクトルを1つ求める
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y, vector.x, 0.0f };
	}
	return { 0.0f, -vector.z, vector.y };
}

// 三角形の描画関数
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 3つの頂点をそれぞれスクリーン座標系に変換する
	Vector3 screenVertices[3];
	for (int i = 0; i < 3; ++i) {
		Vector3 ndcVertex = Transform(triangle.vertices[i], viewProjectionMatrix);
		screenVertices[i] = Transform(ndcVertex, viewportMatrix);
	}

	Novice::DrawTriangle(
		int(screenVertices[0].x), int(screenVertices[0].y),
		int(screenVertices[1].x), int(screenVertices[1].y),
		int(screenVertices[2].x), int(screenVertices[2].y),
		color,
		kFillModeWireFrame
	);
}

// 平面の描画関数
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 中心点を決める
	Vector3 center = { plane.distance * plane.normal.x, plane.distance * plane.normal.y, plane.distance * plane.normal.z };

	// 法線と垂直なベクトルを求める
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));

	perpendiculars[1] = { -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z };
	// 外積
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = { -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z };

	// 4頂点を求めてスクリーン座標に変換
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = { 2.0f * perpendiculars[index].x, 2.0f * perpendiculars[index].y, 2.0f * perpendiculars[index].z };
		Vector3 point = { center.x + extend.x, center.y + extend.y, center.z + extend.z };
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}

	// pointsをそれぞれ結んで矩形を描画する
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[2].x, (int)points[2].y, (int)points[1].x, (int)points[1].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);
	Novice::DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, color);
}

// AABBの描画関数
void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 8頂点を求める
	Vector3 vertices[8];
	vertices[0] = { aabb.min.x, aabb.min.y, aabb.min.z };
	vertices[1] = { aabb.max.x, aabb.min.y, aabb.min.z };
	vertices[2] = { aabb.max.x, aabb.max.y, aabb.min.z };
	vertices[3] = { aabb.min.x, aabb.max.y, aabb.min.z };
	vertices[4] = { aabb.min.x, aabb.min.y, aabb.max.z };
	vertices[5] = { aabb.max.x, aabb.min.y, aabb.max.z };
	vertices[6] = { aabb.max.x, aabb.max.y, aabb.max.z };
	vertices[7] = { aabb.min.x, aabb.max.y, aabb.max.z };
	// 頂点をスクリーン座標に変換
	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i) {
		Vector3 ndcVertex = Transform(vertices[i], viewProjectionMatrix);
		screenVertices[i] = Transform(ndcVertex, viewportMatrix);
	}
	// 線を引く
	int indices[12][2] = {
		{0,1}, {1,2}, {2,3}, {3,0},
		{4,5}, {5,6}, {6,7}, {7,4},
		{0,4}, {1,5}, {2,6}, {3,7}
	};
	for (int i = 0; i < 12; ++i) {
		int index1 = indices[i][0];
		int index2 = indices[i][1];
		Novice::DrawLine(
			int(screenVertices[index1].x), int(screenVertices[index1].y),
			int(screenVertices[index2].x), int(screenVertices[index2].y),
			color
		);
	}
}

// OBBの描画関数
void DrawOBB(const OBB& obb, const Matrix4x4 viewProjectionMatrix, const Matrix4x4 viewportMatrix, uint32_t color) {
	Vector3 vertices[8];

	
	for (int i = 0; i < 8; ++i) {
		float sx = (i & 1) ? obb.size.x : -obb.size.x;
		float sy = (i & 2) ? obb.size.y : -obb.size.y;
		float sz = (i & 4) ? obb.size.z : -obb.size.z;

		vertices[i] = {
			obb.center.x + sx * obb.orientations[0].x + sy * obb.orientations[1].x + sz * obb.orientations[2].x,
			obb.center.y + sx * obb.orientations[0].y + sy * obb.orientations[1].y + sz * obb.orientations[2].y,
			obb.center.z + sx * obb.orientations[0].z + sy * obb.orientations[1].z + sz * obb.orientations[2].z
		};
	}

	// 頂点をスクリーン座標に変換
	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i) {
		Vector3 ndcVertex = Transform(vertices[i], viewProjectionMatrix);
		screenVertices[i] = Transform(ndcVertex, viewportMatrix);
	}

	// 線を引く（
	int indices[12][2] = {
			{0,1}, {2,3}, {4,5}, {6,7}, // X軸方向の辺
			{0,2}, {1,3}, {4,6}, {5,7}, // Y軸方向の辺
			{0,4}, {1,5}, {2,6}, {3,7}  // Z軸方向の辺
	};
	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(
			int(screenVertices[indices[i][0]].x), int(screenVertices[indices[i][0]].y),
			int(screenVertices[indices[i][1]].x), int(screenVertices[indices[i][1]].y),
			color
		);
	}
}

// 2自ベジェ曲線の描画関数
void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
	const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	// 曲線の分割数
	const int kSubdivision = 32;

	Vector3 prevScreenPoint;

	for (int i = 0; i <= kSubdivision; ++i) {
		float t = (float)i / kSubdivision;

		Vector3 p0p1 = Lerp(controlPoint0, controlPoint1, t);
		Vector3 p1p2 = Lerp(controlPoint1, controlPoint2, t);

		Vector3 p = Lerp(p0p1, p1p2, t);

		// 点をスクリーン座標に変換
		Vector3 screenP = Transform(Transform(p, viewProjectionMatrix), viewportMatrix);

		if (i > 0) {
			Novice::DrawLine(
				(int)prevScreenPoint.x, (int)prevScreenPoint.y,
				(int)screenP.x, (int)screenP.y,
				color
			);
		}
		prevScreenPoint = screenP;
	}
}



Vector3 Project(const Vector3& v1, const Vector3& v2) {
	// 内積を計算
	float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	// v2の長さの2乗を計算
	float lengthSq = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;

	float t = dot / lengthSq;
	return { v2.x * t, v2.y * t, v2.z * t };
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	// 始点から点へのベクトル
	Vector3 v = { point.x - segment.origin.x, point.y - segment.origin.y, point.z - segment.origin.z };

	// 始点から点へのベクトルを線分の方向に投影
	Vector3 project = Project(v, segment.diff);

	// 線分上の位置を計算
	float dot = project.x * segment.diff.x + project.y * segment.diff.y + project.z * segment.diff.z;
	float lengthSq = segment.diff.x * segment.diff.x + segment.diff.y * segment.diff.y + segment.diff.z * segment.diff.z;
	float t = dot / lengthSq;

	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;

	// 最終的な座標を計算
	return { segment.origin.x + segment.diff.x * t,
			 segment.origin.y + segment.diff.y * t,
			 segment.origin.z + segment.diff.z * t };
}






// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };


	// カメラの初期設
	Vector3 cameraTranslate = { 0.0f, 1.9f, -6.49f };
	Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

	// ベジェ曲線
	Vector3 controlPoints[3] = {
		{-0.8f,0.58f,1.0f},
		{1.70f,1.0f,-0.3f },
		{0.94f,-0.7f,2.3f},
	};

	/*
	Vector3 obbRotate1 = { 0.0f, 0.0f, 0.0f };
	Vector3 obbRotate2 = { -0.05f, -2.49f, 0.15f };
	*/

	/*
	OBB obb1{
		.center = {0.0f, 0.0f, 0.0f},
		.orientations = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		.size = {0.83f, 0.26f, 0.24f}
	};

	OBB obb2{
		.center = {0.9f, 0.0f, 0.78f},
		.orientations = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		.size = {0.5f, 0.37f, 0.5f}
	};
	*/
	
	/*
	// 球体の初期設定
	Sphere sphere;
	sphere.center = { 0.0f, 1.0f, 0.0f };
	sphere.radius = 0.5f;
	*/
	
	/*
	// 線分の初期化 
	Segment segment{
			.origin = {-0.8f, -0.3f, 0.0f},
			.diff = {0.5f, 0.5f, 0.5f}
	};
	*/

	/*
	// 平面
	Plane plane;
	plane.normal = { 0.0f, 1.0f, 0.0f }; // 最初は上向き
	plane.distance = 0.0f;
	*/

	// 三角形
	/*
	Triangle triangle;
	triangle.vertices[0] = { 0.0f, 1.0f, 0.0f };
	triangle.vertices[1] = { -1.0f, -0.5f, 0.0f };
	triangle.vertices[2] = { 1.0f, -0.5f, 0.0f };
	*/

	/*
	// AABB
	AABB aabb1{
		.min = {-0.5f,-0.5f,-0.5f},
		.max{0.5f,0.5f,0.5f},
	};

	AABB aabb2{
		.min{0.2f,0.2f,0.2f},
		.max{1.0f,1.0f,1.0f},
	};
	*/

	// マウス操作用
	int mouseX = 0;
	int mouseY = 0;
	int preMouseX = 0;
	int preMouseY = 0;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		/*-------------------------------------
		カメラの設定
		--------------------------------------------*/
		preMouseX = mouseX;
		preMouseY = mouseY;

		// 現在のマウス座標を取得
		Novice::GetMousePosition(&mouseX, &mouseY);

		// マウスの移動量を計算
		float deltaX = (float)(mouseX - preMouseX);
		float deltaY = (float)(mouseY - preMouseY);

		// 各操作の感度
		float rotateSensitivity = 0.005f;
		float panSensitivity = 0.01f;
		float scrollSensitivity = 0.001f;

		if (!ImGui::GetIO().WantCaptureMouse) {
			//カメラの回転
			if (Novice::IsPressMouse(0)) {
				cameraRotate.y += deltaX * rotateSensitivity;
				cameraRotate.x += deltaY * rotateSensitivity;
			}
		}

		// カメラの平行移動
		if (Novice::IsPressMouse(1)) {

			cameraTranslate.x -= deltaX * panSensitivity;
			cameraTranslate.y += deltaY * panSensitivity;
		}

		//カメラの前進・後退
		int wheel = Novice::GetWheel();
		if (wheel != 0) {
			cameraTranslate.z += wheel * scrollSensitivity;
		}

		/*---------------------------------------------
		Imgui
		----------------------------------------------------*/
		// ImGuiで値を調整できるようにする
		ImGui::Begin("Settings");
		ImGui::Text("Bezier Control Points");
		ImGui::DragFloat3("Point 0", &controlPoints[0].x, 0.01f);
		ImGui::DragFloat3("Point 1", &controlPoints[1].x, 0.01f);
		ImGui::DragFloat3("Point 2", &controlPoints[2].x, 0.01f);
		ImGui::End();


		Vector3 cameraScale = { 1.0f, 1.0f, 1.0f };


		/*----------------------------------------------
		行列計算
		------------------------------------*/
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);

		// プロジェクション行列を計算
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);

		// ViewProjection行列
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

		// Viewport行列
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// グリッドを描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		DrawBezier(controlPoints[0], controlPoints[1], controlPoints[2], viewProjectionMatrix, viewportMatrix, 0xFF0000FF);

		Sphere p0 = { controlPoints[0], 0.01f };
		Sphere p1 = { controlPoints[1], 0.01f };
		Sphere p2 = { controlPoints[2], 0.01f };

		DrawSphere(p0, viewProjectionMatrix, viewportMatrix, 0x000000FF);
		DrawSphere(p1, viewProjectionMatrix, viewportMatrix, 0x000000FF);
		DrawSphere(p2, viewProjectionMatrix, viewportMatrix, 0x000000FF);
		
		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}