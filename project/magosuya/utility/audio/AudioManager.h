#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <xaudio2.h>
#include <memory>

//チャンクヘッダ
struct ChunkHeader {
	char id[4];		//チャンク毎のID
	int32_t size;	//チャンクサイズ
};

//RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;	//"RIFF"
	char type[4];		//"WAVE"
};

//FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;	//"fmt"
	WAVEFORMATEX fmt;		//波形フォーマット
};

//オーディオデータ構造体
struct AudioData {
	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;
};

class AudioManager {
public:
	static AudioManager* GetInstance() {
		//初めて呼び出されたときに一回だけ初期化
		static AudioManager instance;
		return &instance;
	}

	void Initialze();

	AudioData AudioLoadWave(const char* filename);

	void AudioUnload(AudioData* soundData);

	void AudioPlayWave(IXAudio2* xAudio2, const AudioData& soundData);

private:
	//コンストラクタを禁止
	AudioManager() = default;
	// コピーコンストラクタと代入演算子を禁止
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;
	AudioManager(AudioManager&&) = delete;
	AudioManager& operator=(AudioManager&&) = delete;

private:
	ComPtr<IXAudio2> xAudio2_;
	std::unique_ptr<IXAudio2MasteringVoice> masterVoice_;
};