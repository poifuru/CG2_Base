#include <Windows.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <imgui.h>
#include "MagosuyaEngine.h"
#include "SceneManager.h"

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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	MagosuyaEngine* magosuya = MagosuyaEngine::GetInstance ();
	magosuya->Initialize ();

	std::unique_ptr<SceneManager> sceneManager;
	sceneManager = std::make_unique<SceneManager> (
	CameraOrganizer::GetInstance(), InputManager::GetInstance(), DxCommon::GetInstance()
	);
	sceneManager->Initialize (SceneLabel::Title);

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

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource = DxCommon::GetInstance ()->CreateBufferResource (sizeof (DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレス取得
	dierctionalLightResource->Map (0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//実際に書き込み
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, 0.0f, 1.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightData->mode = LightReflectionModel::halfLambert;
	//ライティング用の変数
	float colorLight[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while (true) {

		if (WindowsAPI::GetInstance ()->ProcessMessage ()) {
			break;
		}

		//フレーム開始
		magosuya->BeginFrame ();

		//***更新処理***//
		sceneManager->Update ();
		//*************//

		//光源のdirectionの正規化
		directionalLightData->direction = Math::Normalize (directionalLightData->direction);
		//RootSignatureをセット
		ID3D12RootSignature* standardRootSig = RootSignatureManager::GetInstance ()->GetRootSignature (
			RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::Standard3D)
		);

		DxCommon::GetInstance ()->GetCommandList ()->SetGraphicsRootSignature (standardRootSig);
		//ライティングの設定
		DxCommon::GetInstance ()->GetCommandList ()->SetGraphicsRootConstantBufferView (3, dierctionalLightResource->GetGPUVirtualAddress ());

		//***描画処理***//
		sceneManager->Draw ();
		//*************//

		//フレーム終了
		magosuya->EndFrame ();
	}
	xAudio2.Reset ();
	SoundUnload (&soundData1);  // バッファ解放
	return 0;
};