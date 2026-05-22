#include <Novice.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cstdint>
#include<imgui.h>
#define _USE_MATH_DEFINES
#include <cmath>

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

//法線と垂直なベクトルを1つ求める
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y, vector.x, 0.0f };
	}
	return { 0.0f, -vector.z, vector.y };
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

/*
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

*/


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

	// 球体の初期設定
// 2つの球の初期化
	Sphere sphere;
	sphere.center = { 0.0f, 1.0f, 0.0f };
	sphere.radius = 0.5f;

	Plane plane;
	plane.normal = { 0.0f, 1.0f, 0.0f }; // 最初は上向き
	plane.distance = 0.0f;

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


		// ImGuiで値を調整できるようにする
		ImGui::Begin("Settings");
		ImGui::Text("Sphere");
		ImGui::DragFloat3("Center", &sphere.center.x, 0.01f);
		ImGui::DragFloat("Radius", &sphere.radius, 0.01f);

		ImGui::Separator();

		ImGui::Text("Plane");
		ImGui::DragFloat3("Normal", &plane.normal.x, 0.01f);
		plane.normal = Normalize(plane.normal);
		ImGui::DragFloat("Distance", &plane.distance, 0.01f);
		ImGui::End();

		// 衝突判定
		bool isCollide = IsCollision(sphere, plane);
		// 衝突していたら赤、していなければ白
		unsigned int sphereColor = isCollide ? RED : WHITE;
		Vector3 cameraScale = { 1.0f, 1.0f, 1.0f };


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


		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, WHITE);
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, sphereColor);

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