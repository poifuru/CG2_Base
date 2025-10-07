#include <Windows.h>
#include "engine/EngineCore/DxCommon.h"
#include "engine/utility/function.h"
#include "engine/utility/struct.h"
#include <format>// C++20のformat() 文字列整形
#include <chrono>	//時間を扱うライブラリ
#include <sstream>// stringstream
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#include "engine/3d/Model.h"
#include "engine/3d/SphereModel.h"
#include "engine/2d/Sprite.h"
#include "engine/camera/DebugCamera.h"

//サウンドデータの読み込み関数
SoundData SoundLoadWave (const char* filename) {
	/*1,ファイルを開く*/
	//ファイルストリームのインスタンス
	std::ifstream file;
	//wavファイルをバイナリーモードで開く
	file.open (filename, std::ios_base::binary);
	//ファイルが開けなければassert
	assert (file.is_open ());

	/*2,wavデータ読み込み*/
	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read ((char*)&riff, sizeof (riff));
	//ファイルがRIFFかチェック
	if (strncmp (riff.chunk.id, "RIFF", 4) != 0) {
		assert (0);
	}
	//タイプがWAVEかチェック
	if (strncmp (riff.type, "WAVE", 4) != 0) {
		assert (0);
	}

	//Formatチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダーの確認
	file.read ((char*)&format, sizeof (ChunkHeader));
	if (strncmp (format.chunk.id, "fmt ", 4) != 0) {
		assert (0);
	}

	//チャンク本体の読み込み
	assert (format.chunk.size <= sizeof (format.fmt));
	file.read ((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read ((char*)&data, sizeof (data));
	//JUNKチャンクを検出した場合
	if (strncmp (data.id, "JUNK", 4) != 0) {
		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg (data.size, std::ios_base::cur);
		//再読み込み
		file.read ((char*)&data, sizeof (data));
	}

	if (strncmp (data.id, "data", 4) != 0) {
		assert (0);
	}

	//Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read (pBuffer, data.size);

	//waveファイルを閉じる
	file.close ();

	//returnするための音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

//音声データを解放する関数
void SoundUnload (SoundData* soundData) {
	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

//音声再生の関数
void SoundPlayWave (IXAudio2* xAudio2, const SoundData& soundData) {
	HRESULT result;

	//波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice (&pSourceVoice, &soundData.wfex);
	assert (SUCCEEDED (result));

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer (&buf);
	result = pSourceVoice->Start ();
}



/*コメントスペース*/
//05_03の16ページからスタート



// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) {
	DxCommon* dxCommon = new DxCommon;
	dxCommon->Initialize ();

	MSG msg{};

	HRESULT hr;

	//サウンドの導入
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	hr = XAudio2Create (&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert (SUCCEEDED (hr));

	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice (&masterVoice);

	//音声の読み込み
	SoundData soundData1 = SoundLoadWave ("Resources/Sounds/Alarm01.wav");

#pragma region Plane
	Model* plane = new Model (dxCommon->GetDevice (), "Resources/plane", "plane", true);
#pragma endregion

#pragma region bunny
	Model* bunny = new Model (dxCommon->GetDevice (), "Resources/bunny", "bunny", false);
#pragma endregion

#pragma region Teapot
	Model* teapot = new Model (dxCommon->GetDevice (), "Resources/teapot", "teapot", false);
#pragma endregion

#pragma region Fence
	Model* Fence = new Model (dxCommon->GetDevice (), "Resources/fence", "fence", false);
#pragma endregion

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource = CreateBufferResource (dxCommon->GetDevice(), sizeof (DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレス取得
	dierctionalLightResource->Map (0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//実際に書き込み
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightData->mode = Light::halfLambert;

	//Transform
	Transform transform{ {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	Transform transformModel{ {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	Transform transformTeapot{ {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	Transform cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	//UVtransform用の変数
	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};

	//DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = dxCommon->GetDevice ()->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = dxCommon->GetDevice ()->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = dxCommon->GetDevice ()->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//Textureを呼んで転送する
	DirectX::ScratchImage mipImages[5];
	mipImages[0] = LoadTexture ("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata0 = mipImages[0].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource0 = CreateTextureResource (dxCommon->GetDevice (), metadata0);
	ComPtr<ID3D12Resource> intermediateResource0 = UploadTextureData (textureResource0, mipImages[0], dxCommon->GetDevice (), dxCommon->GetCommandList());

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescSphere{};
	srvDescSphere.Format = metadata0.format;
	srvDescSphere.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescSphere.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescSphere.Texture2D.MipLevels = UINT (metadata0.mipLevels);

	mipImages[1] = LoadTexture (plane->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata1 = mipImages[1].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource1 = CreateTextureResource (dxCommon->GetDevice (), metadata1);
	ComPtr<ID3D12Resource> intermediateResource1 = UploadTextureData (textureResource1, mipImages[1], dxCommon->GetDevice (), dxCommon->GetCommandList ());

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescPlane{};
	srvDescPlane.Format = metadata1.format;
	srvDescPlane.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescPlane.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescPlane.Texture2D.MipLevels = UINT (metadata1.mipLevels);

	mipImages[2] = LoadTexture (bunny->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages[2].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource (dxCommon->GetDevice (), metadata2);
	ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData (textureResource2, mipImages[2], dxCommon->GetDevice (), dxCommon->GetCommandList ());

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescBunny{};
	srvDescBunny.Format = metadata2.format;
	srvDescBunny.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescBunny.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescBunny.Texture2D.MipLevels = UINT (metadata2.mipLevels);

	mipImages[3] = LoadTexture (teapot->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata3 = mipImages[3].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource3 = CreateTextureResource (dxCommon->GetDevice (), metadata3);
	ComPtr<ID3D12Resource> intermediateResource3 = UploadTextureData (textureResource3, mipImages[3], dxCommon->GetDevice (), dxCommon->GetCommandList ());

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescTeapot{};
	srvDescTeapot.Format = metadata3.format;
	srvDescTeapot.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescTeapot.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescTeapot.Texture2D.MipLevels = UINT (metadata3.mipLevels);

	mipImages[4] = LoadTexture (Fence->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata4 = mipImages[4].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource4 = CreateTextureResource (dxCommon->GetDevice (), metadata4);
	ComPtr<ID3D12Resource> intermediateResource4 = UploadTextureData (textureResource4, mipImages[4], dxCommon->GetDevice (), dxCommon->GetCommandList ());

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescFence{};
	srvDescFence.Format = metadata4.format;
	srvDescFence.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescFence.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescFence.Texture2D.MipLevels = UINT (metadata4.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU[5];
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU[5];
	textureSrvHandleCPU[0] = GetCPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap(), descriptorSizeSRV, 1);
	textureSrvHandleGPU[0] = GetGPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 1);

	textureSrvHandleCPU[1] = GetCPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 2);
	textureSrvHandleGPU[1] = GetGPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 2);

	textureSrvHandleCPU[2] = GetCPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 3);
	textureSrvHandleGPU[2] = GetGPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 3);

	textureSrvHandleCPU[3] = GetCPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 4);
	textureSrvHandleGPU[3] = GetGPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 4);

	textureSrvHandleCPU[4] = GetCPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 5);
	textureSrvHandleGPU[4] = GetGPUDescriptorHandle (dxCommon->GetsrvDescriptorHeap (), descriptorSizeSRV, 5);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView (textureResource0.Get (), &srvDescSphere, textureSrvHandleCPU[0]);
	dxCommon->GetDevice ()->CreateShaderResourceView (textureResource1.Get (), &srvDescPlane, textureSrvHandleCPU[1]);
	dxCommon->GetDevice ()->CreateShaderResourceView (textureResource2.Get (), &srvDescBunny, textureSrvHandleCPU[2]);
	dxCommon->GetDevice ()->CreateShaderResourceView (textureResource3.Get (), &srvDescTeapot, textureSrvHandleCPU[3]);
	dxCommon->GetDevice ()->CreateShaderResourceView (textureResource4.Get (), &srvDescFence, textureSrvHandleCPU[4]);

	//BGM再生
	SoundPlayWave (xAudio2.Get (), soundData1);

	/********入力デバイス組********/
	//キーボード情報の取得開始
	dxCommon->GetKeyboard ()->Acquire ();
	//全キーの入力状態を取得する
	BYTE key[256] = {};

	//マウス
	dxCommon->GetMouse ()->Acquire ();
	/****************************/

	/*mainループで使いたい変数を入れるとこ*/
	MouseInput mouseInput{
		0, 0, 0,
		false, false,
		false, false,
		false, false,
	};
	DIMOUSESTATE mouseState = {};

	//スプライト
	Sprite* sprite = new Sprite (dxCommon->GetDevice());
	sprite->Initialize ({ 0.0f, 0.0f, 0.0f }, { 640.0f, 360.0f });

	SphereModel* sphere = new SphereModel (dxCommon->GetDevice (), 16);
	sphere->Initialize ({ 0.0f, 0.0f, 0.0f }, 1.0f);

	plane->Initialize ();
	bunny->Initialize ();
	teapot->Initialize ();
	Fence->Initialize ();

	//カメラ用
	Matrix4x4 cameraMatrix = {};
	Matrix4x4 viewMatrix = {};
	Matrix4x4 projectionMatrix = {};
	//Matrix4x4 worldViewProjectionMatrix = {};

	//デバッグカメラ
	DebugCamera* debugCamera = new DebugCamera ();
	debugCamera->Initialize ();
	bool debugMode = false;

	//テクスチャ切り替え用の変数
	bool useMonsterBall = true;
	//スプライト切り替え
	bool useSprite = false;
	//球の切り替え
	bool useSphere = false;
	//ぷれーん
	bool usePlane = false;
	//うさぎ
	bool useModel = false;
	//てぃーぽっと
	bool useTeapot = false;

	//ライティング用の変数
	float colorLight[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	/*********************************/

	

	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		else {
			//フレーム開始
			dxCommon->BeginFrame ();

			//入力デバイスの状態を取得
			//キーボード
			hr = dxCommon->GetKeyboard ()->GetDeviceState (sizeof (key), key);
			if (FAILED (hr)) {
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
					dxCommon->GetKeyboard ()->Acquire ();
				}
			}

			//前フレームの状態を保存
			bool isTabDown = (key[DIK_TAB] & 0x80);
			static bool wasTabDown = false;

			//マウス
			hr = dxCommon->GetMouse ()->GetDeviceState (sizeof (DIMOUSESTATE), &mouseState);
			if (FAILED (hr)) {
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
					dxCommon->GetMouse ()->Acquire ();
				}
			}

			// 前フレームの状態を保存
			mouseInput.prevLeft = mouseInput.left;
			mouseInput.prevRight = mouseInput.right;
			mouseInput.prevMid = mouseInput.mid;

			mouseInput.x = static_cast<float>(mouseState.lX);
			mouseInput.y = static_cast<float>(mouseState.lY);
			mouseInput.z = static_cast<float>(mouseState.lZ);

			mouseInput.left = (mouseState.rgbButtons[0] & 0x80);
			mouseInput.right = (mouseState.rgbButtons[1] & 0x80);
			mouseInput.mid = (mouseState.rgbButtons[2] & 0x80);

			

			//実際のキー入力処理はここ！
			if (!ImGui::GetIO ().WantCaptureKeyboard) {
				// 押した瞬間だけトグル
				if (isTabDown && !wasTabDown) {
					if (!debugMode) {
						debugMode = true;
					}
					else {
						debugMode = false;
					}
				}
			}
			wasTabDown = isTabDown;

			//ゲームの処理//
			//=======オブジェクトの更新処理=======//
			//カメラ
			cameraMatrix = MakeAffineMatrix (cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			viewMatrix = Inverse (cameraMatrix);
			projectionMatrix = MakePerspectiveFOVMatrix (0.45f, float (kClientWidth) / float (kClientHeight), 0.1f, 100.0f);
			if (!debugMode) {
				//worldViewProjectionMatrix = Multiply (cameraMatrix, Multiply (viewMatrix, projectionMatrix));
			}
			if (!ImGui::GetIO ().WantCaptureMouse) {
				if (debugMode) {
					debugCamera->Updata (dxCommon->GetHWND(), hr, dxCommon->GetKeyboard (), key, dxCommon->GetMouse (), &mouseInput);
					//worldViewProjectionMatrix = Multiply (debugCamera->GetProjectionMatrix(), Multiply (debugCamera->GetViewMatrix (), debugCamera->GetProjectionMatrix ()));
					viewMatrix = debugCamera->GetViewMatrix();
					projectionMatrix = debugCamera->GetProjectionMatrix ();
				}
			}

			//オブジェクト
			plane->Update (&viewMatrix, &projectionMatrix);

			bunny->Update (&viewMatrix, &projectionMatrix);

			teapot->Update (&viewMatrix, &projectionMatrix);

			sprite->Update ();
			sphere->Update (&viewMatrix, &projectionMatrix);
			Fence->Update (&viewMatrix, &projectionMatrix);
			
			//光源のdirectionの正規化
			directionalLightData->direction = Normalize (directionalLightData->direction);

			ImGui::Begin ("カメラモード:TAB");
			if (debugMode) {
				ImGui::TextColored (ImVec4 (1, 1, 0, 1), "Current Camera: Debug");
			}
			else if (!debugMode) {
				ImGui::TextColored (ImVec4 (0, 1, 0, 1), "Current Camera: Scene");
			}
			ImGui::End ();

			//ImGuiと変数を結び付ける
			// 色変更用のUI
			static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // 初期値：白

			ImGui::Begin ("setting");
			if (ImGui::CollapsingHeader ("SceneCamera")) {
				if (ImGui::Button ("Reset")) {
					cameraTransform = {
						{1.0f, 1.0f, 1.0f},
						{0.0f, 0.0f, 0.0f},
						{0.0f, 0.0f, -10.0f},
					};
				}
				ImGui::DragFloat3 ("cameraScale", &cameraTransform.scale.x, 0.01f);
				ImGui::DragFloat3 ("cameraRotate", &cameraTransform.rotate.x, 0.01f);
				ImGui::DragFloat3 ("cameraTranslate", &cameraTransform.translate.x, 0.01f);
			}
			if (ImGui::CollapsingHeader ("sphere")) {
				ImGui::Checkbox ("speher##useSphere", &useSphere);
				sphere->ShowImGuiEditor ();
			}
			if (ImGui::CollapsingHeader ("plane")) {
				ImGui::Checkbox ("Draw##plane", &usePlane);
				plane->ImGui ();
			}
			if (ImGui::CollapsingHeader ("Model")) {
				ImGui::Checkbox ("Draw##Model", &useModel);
				bunny->ImGui ();
			}
			if (ImGui::CollapsingHeader ("teapod")) {
				ImGui::Checkbox ("Draw##teapod", &useTeapot);
				teapot->ImGui ();
			}
			if (ImGui::CollapsingHeader ("Sprite")) {
				ImGui::Checkbox ("Draw##useSprite", &useSprite);
				sprite->ShowImGuiEditor ();
			}
			if (ImGui::CollapsingHeader ("light")) {
				if (ImGui::ColorEdit4 ("color", colorLight)) {
					// 色が変更されたらmaterialDataに反映
					directionalLightData->color.x = colorLight[0];
					directionalLightData->color.y = colorLight[1];
					directionalLightData->color.z = colorLight[2];
					directionalLightData->color.w = colorLight[3];
				}
				ImGui::DragFloat3 ("lightDirection", &directionalLightData->direction.x, 0.01f);
				ImGui::DragFloat ("intensity", &directionalLightData->intensity, 0.01f);
			}
			if (ImGui::CollapsingHeader ("fence")) {
				Fence->ImGui ();
			}
			ImGui::End ();
			/*ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/

			

			//=======コマンド君達=======//
			//ライティングの設定
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView (3, dierctionalLightResource->GetGPUVirtualAddress ());
			//描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
			Fence->Draw (dxCommon->GetCommandList (), textureSrvHandleGPU[4]);
			if (useSphere) {
				sphere->Draw (dxCommon->GetCommandList (), textureSrvHandleGPU[0]);
			}
			if (usePlane) {
				plane->Draw (dxCommon->GetCommandList (), textureSrvHandleGPU[1]);
			}
			if (useModel) {
				bunny->Draw (dxCommon->GetCommandList (), textureSrvHandleGPU[2]);
			}
			if (useTeapot) {
				teapot->Draw (dxCommon->GetCommandList (), textureSrvHandleGPU[3]);
			}
			if (useSprite) {
				sprite->Draw (dxCommon->GetCommandList (), textureSrvHandleGPU[0]);
			}
			//フレーム終了
			dxCommon->EndFrame ();
		}
	}

	dxCommon->Finalize ();
	xAudio2.Reset ();
	SoundUnload (&soundData1);  // バッファ解放
	return 0;
};