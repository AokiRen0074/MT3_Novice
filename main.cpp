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

struct Sphere {
	Vector3 center;
	float radius;
};

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
	Sphere sphere;
	sphere.center = { 0.0f, 0.0f, 0.0f };
	sphere.radius = 1.0f;


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

		// ImGui
		ImGui::Begin("Settings");
		ImGui::DragFloat3("Camera Translate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("Camera Rotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("Sphere Center", &sphere.center.x, 0.01f);
		ImGui::DragFloat("Sphere Radius", &sphere.radius, 0.01f);
		ImGui::End();

		// カメラの行列を計算

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

		// 球体を描画
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

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