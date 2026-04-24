#include <Novice.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cstdint>

const char kWindowTitle[] = "LC1B_01_アオキレン_タイトル";

static const int kRowHeight = 20;
static const int kColumnWidth = 60;

// ウィンドサイズ
const int kWindowWidth = 1280;
const int kWindowHeight = 720;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// 三角形のローカル座標
	Vector3 kLocalVertices[3] = {
	{ 0.0f,  1.0f, 0.0f}, // 上
	{ 1.0f, -1.0f, 0.0f}, // 右下
	{-1.0f, -1.0f, 0.0f}  // 左下
	};


	// カメラ位置
	Vector3 cameraPosition = { 0.0f, 0.0f, -5.0f };

	Vector3 rotate{};
	Vector3 translate{};



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

		rotate.y += 0.05f; // 自動でY軸回転させる

		/*---------------------------------
		移動処理
		-----------------------------*/
		// WSADキーで前後左右に移動させる
		if (keys[DIK_W]) {
			translate.z += 0.05f;
		}

		if (keys[DIK_S]) {
			translate.z -= 0.05f;
		}

		if (keys[DIK_D]) {
			translate.x += 0.05f;
		}

		if (keys[DIK_A]) {
			translate.x -= 0.05f;
		}

		/*-------------------------------
		各種行列の計算
		------------------------------*/
		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, cameraPosition);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);
		Vector3 screenVertices[3];

		for (uint32_t i = 0; i < 3; ++i) {
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawTriangle(
			int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y),
			int(screenVertices[2].x), int(screenVertices[2].y), RED, kFillModeSolid);

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

