#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//エンジンに使うヘッダーファイル群
#include "header/Engine.h"
#include "engine/utility/function.h"
#include "engine/camera/DebugCamera.h"
#include "engine/2d/Sprite.h"
#include "engine/3d/sphereModel.h"
#include "engine/utility/globalVariables.h"
#include "engine/utility/struct.h"
#include "engine/utility/Math.h"
#include "engine/3d/Model.h"
#include "externals.h"
#pragma warning(pop)
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#include <format>// C++20のformat() 文字列整形
#include <chrono>	//時間を扱うライブラリ
#include <sstream>// stringstream

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

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker () {
		ComPtr<IDXGIDebug> debug;
		if (SUCCEEDED (DXGIGetDebugInterface1 (0, IID_PPV_ARGS (debug.GetAddressOf ())))) {
			debug->ReportLiveObjects (DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects (DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects (DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

/*コメントスペース*/
//05_03の16ページからスタート

//ウィンドウサイズを表す構造体にクライアント領域を入れる
RECT wrc = { 0, 0, kClientWidth, kClientHeight };

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) {
	//COMの初期化
	CoInitializeEx (0, COINIT_MULTITHREADED);

	//誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	//main関数が始まってすぐに登録すると良い
	SetUnhandledExceptionFilter (ExportDump);

	D3DResourceLeakChecker leakCheck;

	std::ofstream logStream = Logtext ();

	WNDCLASS wc{};
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	wc.lpszClassName = L"LE2B_22_マスヤ_ゴウ";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle (nullptr);
	//カーソル
	wc.hCursor = LoadCursor (nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass (&wc);

	//クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect (&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウを生成
	HWND hwnd = CreateWindow (
		wc.lpszClassName,		//利用するクラス名
		L"CG2",					//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
		CW_USEDEFAULT,			//表示x座標(Windowsに任せる)
		CW_USEDEFAULT,			//表示y座標(WindowsOSに任せる)
		wrc.right - wrc.left,	//ウィンドウ横幅
		wrc.bottom - wrc.top,	//ウィンドウ縦幅
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		wc.hInstance,			//インスタンスハンドル
		nullptr					//オプション
	);

#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED (D3D12GetDebugInterface (IID_PPV_ARGS (&debugController)))) {
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer ();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation (true);
	}
#endif // DEBUG

	//ウィンドウを表示
	ShowWindow (hwnd, SW_SHOW);

	//DXGIファクトリーの生成
	ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//HRESULTはWindows系のエラーコードであり、
	//関数が成功したかどうかをSUCCEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory (IID_PPV_ARGS (&dxgiFactory));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert (SUCCEEDED (hr));

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference (i,
		 DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS (&useAdapter)) !=
		 DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3 (&adapterDesc);
		assert (SUCCEEDED (hr)); //取得できないのは一大事
		//ソフトウェアアダプタでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Log (logStream, ConvertString (std::format (L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;	//ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプタが見つからなかったので起動できない
	assert (useAdapter != nullptr);

	//dxcCompilerを初期化
	ComPtr<IDxcUtils> dxcUtils = nullptr;
	ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	hr = DxcCreateInstance (CLSID_DxcUtils, IID_PPV_ARGS (&dxcUtils));
	assert (SUCCEEDED (hr));
	hr = DxcCreateInstance (CLSID_DxcCompiler, IID_PPV_ARGS (&dxcCompiler));
	assert (SUCCEEDED (hr));

	//後のincludeに対応するための設定を作る
	ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler (&includeHandler);
	assert (SUCCEEDED (hr));

	/*入力デバイスの初期化処理*/
	//DirectInputの初期化
	ComPtr<IDirectInput8> directInput = nullptr;
	hr = DirectInput8Create (wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert (SUCCEEDED (hr));

	//キーボードデバイスの生成
	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	hr = directInput->CreateDevice (GUID_SysKeyboard, &keyboard, NULL);
	assert (SUCCEEDED (hr));

	//入力データ形式のセット
	hr = keyboard->SetDataFormat (&c_dfDIKeyboard);	//標準形式
	assert (SUCCEEDED (hr));

	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel (hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert (SUCCEEDED (hr));

	//マウスデバイスの生成
	ComPtr<IDirectInputDevice8> mouse = nullptr;
	hr = directInput->CreateDevice (GUID_SysMouse, &mouse, NULL);
	assert (SUCCEEDED (hr));

	//入力データ形式のセット
	hr = mouse->SetDataFormat (&c_dfDIMouse);
	assert (SUCCEEDED (hr));

	//排他制御レベルのセット
	hr = mouse->SetCooperativeLevel (hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert (SUCCEEDED (hr));

	//デバイス生成
	ComPtr<ID3D12Device> device = nullptr;
	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelString[] = { "12.2", "12.1", "12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof (featureLevels); ++i) {
		hr = D3D12CreateDevice (useAdapter.Get (), featureLevels[i], IID_PPV_ARGS (&device));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED (hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Log (logStream, std::format ("FeatureLevel : {}\n", featureLevelString[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert (device != nullptr);
	//初期化完了のログを出す
	Log (logStream, "Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED (device->QueryInterface (IID_PPV_ARGS (&infoQueue)))) {
		//ヤバいエラー時に止まる
		infoQueue->SetBreakOnSeverity (D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity (D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			//https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof (denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof (severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter (&filter);

		//解放
		infoQueue->Release ();
	}
#endif // _DEBUG

	//コマンドキューを生成する
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue (&commandQueueDesc, IID_PPV_ARGS (&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert (SUCCEEDED (hr));

	//コマンドアロケータを生成する
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&commandAllocator));
	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert (SUCCEEDED (hr));

	//コマンドリストを生成する
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get (), nullptr,
									IID_PPV_ARGS (&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert (SUCCEEDED (hr));

	//スワップチェーンを生成する
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;		//画面の幅,ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;	//画面の高さ,ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色の形式
	swapChainDesc.SampleDesc.Count = 1;	//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;	//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//モニタにうつしたら、中身を破棄
	//コマンドキュー,ウィンドウハンドル,設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd (commandQueue.Get (), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf ()));
	assert (SUCCEEDED (hr));

	//ディスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap (device.Get (), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap (device.Get (), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//swapChainからResourceを引っ張ってくる
	ComPtr<ID3D12Resource> swapChainResources[5] = { nullptr };
	hr = swapChain->GetBuffer (0, IID_PPV_ARGS (&swapChainResources[0]));
	//うまく取得出来なければ起動できない
	assert (SUCCEEDED (hr));
	hr = swapChain->GetBuffer (1, IID_PPV_ARGS (&swapChainResources[1]));
	assert (SUCCEEDED (hr));

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;		//2Dテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart ();
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[5];
	//まず1つ目を作る。1つ目は最初の所に作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView (swapChainResources[0].Get (), &rtvDesc, rtvHandles[0]);
	//2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device->CreateRenderTargetView (swapChainResources[1].Get (), &rtvDesc, rtvHandles[1]);

	MSG msg{};

	//初期値0でFenceを作る
	ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence (fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&fence));
	assert (SUCCEEDED (hr));

	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
	assert (fenceEvent != nullptr);

	//DiscriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;	//0から始まる
	descriptorRange[0].NumDescriptors = 1;		//数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//-----RootSignatureをモデルとスプライト用の2つ作る-----//
	//***モデル用***//
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureModel{};
	descriptionRootSignatureModel.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameter[4] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	rootParameter[0].Descriptor.ShaderRegister = 0;

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameter[1].Descriptor.ShaderRegister = 1;						//レジスタ番号とバインド
	descriptionRootSignatureModel.pParameters = rootParameter;				//ルートパラメータ配列へのポインタ
	descriptionRootSignatureModel.NumParameters = _countof (rootParameter);	//配列の長さ

	//DescriptorTable
	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DiscriptorTableを使う
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameter[2].DescriptorTable.pDescriptorRanges = descriptorRange;	//Tableの中身の配列を指定
	rootParameter[2].DescriptorTable.NumDescriptorRanges = _countof (descriptorRange);	//Tableで利用する数

	//平行光源用のCBV
	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].Descriptor.ShaderRegister = 3;

	//***スプライト用***//


	//-----------------------------------------------------//

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof (inputElementDescs);

	//BlendStateの設定(何個か作る)
	const int kBlendDescNum = 6;
	D3D12_BLEND_DESC blendDesc[kBlendDescNum]{};
	//すべての色の要素を書き込む
	blendDesc[0].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//共通設定なのでfor文で回す
	for (int i = 1; i < kBlendDescNum; i++) {
		blendDesc[i].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
		blendDesc[i].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc[i].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc[i].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	}

	//---個別の設定---//
	//アルファブレンド
	blendDesc[1].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[1].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[1].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	//加算合成
	blendDesc[2].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[2].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[2].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

	//減算合成
	blendDesc[3].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[3].RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;
	blendDesc[3].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

	//乗算合成
	blendDesc[4].RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	blendDesc[4].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[4].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;

	//スクリーン合成
	blendDesc[5].RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	blendDesc[5].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[5].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	//--------------//


	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのmipmapを使う
	staticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
	descriptionRootSignatureModel.pStaticSamplers = staticSamplers;
	descriptionRootSignatureModel.NumStaticSamplers = _countof (staticSamplers);

	//シリアライズしてバイナリにする
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature (&descriptionRootSignatureModel,
									  D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED (hr)) {
		Log (logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer ()));
		assert (false);
	}

	//バイナリを元に作成
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature (0, signatureBlob->GetBufferPointer (),
									  signatureBlob->GetBufferSize (), IID_PPV_ARGS (&rootSignature));
	assert (SUCCEEDED (hr));

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	ComPtr<IDxcBlob> vertexShaderBlob = CompilerShader (L"engine/shader/Object3d.VS.hlsl",
														L"vs_6_0", dxcUtils.Get (), dxcCompiler.Get (), includeHandler.Get (), logStream);
	assert (vertexShaderBlob != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlob = CompilerShader (L"engine/shader/Object3d.PS.hlsl",
													   L"ps_6_0", dxcUtils.Get (), dxcCompiler.Get (), includeHandler.Get (), logStream);
	assert (pixelShaderBlob != nullptr);

	//DepthStencilTextureをウィンドウサイズで作成
	ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource (device.Get (), kClientWidth, kClientHeight);

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap (device.Get (), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		//Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;		//2dTexture
	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView (depthStencilResource.Get (), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart ());

	//depthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPieplineStateDesc{};
	graphicsPieplineStateDesc.pRootSignature = rootSignature.Get ();//RootSignature
	graphicsPieplineStateDesc.InputLayout = inputLayoutDesc;		//InputLayout
	graphicsPieplineStateDesc.VS = { vertexShaderBlob->GetBufferPointer (),
	vertexShaderBlob->GetBufferSize () };							//VertexShader
	graphicsPieplineStateDesc.PS = { pixelShaderBlob->GetBufferPointer (),
	pixelShaderBlob->GetBufferSize () };							//PixelShader
	graphicsPieplineStateDesc.BlendState = blendDesc[1];			//BlendState
	graphicsPieplineStateDesc.RasterizerState = rasterizerDesc;		//RastarizerState

	//書き込むRTVの情報
	graphicsPieplineStateDesc.NumRenderTargets = 1;
	graphicsPieplineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPieplineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むのかの設定(気にしなくて良い)
	graphicsPieplineStateDesc.SampleDesc.Count = 1;
	graphicsPieplineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPieplineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPieplineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState (&graphicsPieplineStateDesc,
											  IID_PPV_ARGS (&graphicsPipelineState));
	assert (SUCCEEDED (hr));

	//サウンドの導入
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	hr = XAudio2Create (&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert (SUCCEEDED (hr));

	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice (&masterVoice);

	//音声の読み込み
	SoundData soundData1 = SoundLoadWave ("Resources/Sounds/Alarm01.wav");

	//PSOを生成する
	//PSO->Generate(device, hr);

#pragma region Plane
	Model* plane = new Model (device.Get (), "Resources/plane", "plane", true);
#pragma endregion

#pragma region bunny
	Model* bunny = new Model (device.Get (), "Resources/bunny", "bunny", false);
#pragma endregion

#pragma region Teapot
	Model* teapot = new Model (device.Get (), "Resources/teapot", "teapot", false);
#pragma endregion

#pragma region Fence
	Model* Fence = new Model (device.Get (), "Resources/fence", "fence", true);
#pragma endregion

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource = CreateBufferResource (device.Get (), sizeof (DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレス取得
	dierctionalLightResource->Map (0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//実際に書き込み
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = static_cast<float>(kClientWidth);
	viewport.Height = static_cast<float>(kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//シザー矩形
	D3D12_RECT scissorRect{};
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

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
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//Textureを呼んで転送する
	DirectX::ScratchImage mipImages[5];
	mipImages[0] = LoadTexture ("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata0 = mipImages[0].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource0 = CreateTextureResource (device.Get (), metadata0);
	ComPtr<ID3D12Resource> intermediateResource0 = UploadTextureData (textureResource0, mipImages[0], device.Get (), commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescSphere{};
	srvDescSphere.Format = metadata0.format;
	srvDescSphere.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescSphere.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescSphere.Texture2D.MipLevels = UINT (metadata0.mipLevels);

	mipImages[1] = LoadTexture (plane->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata1 = mipImages[1].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource1 = CreateTextureResource (device.Get (), metadata1);
	ComPtr<ID3D12Resource> intermediateResource1 = UploadTextureData (textureResource1, mipImages[1], device.Get (), commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescPlane{};
	srvDescPlane.Format = metadata1.format;
	srvDescPlane.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescPlane.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescPlane.Texture2D.MipLevels = UINT (metadata1.mipLevels);

	mipImages[2] = LoadTexture (bunny->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages[2].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource (device.Get (), metadata2);
	ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData (textureResource2, mipImages[2], device.Get (), commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescBunny{};
	srvDescBunny.Format = metadata2.format;
	srvDescBunny.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescBunny.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescBunny.Texture2D.MipLevels = UINT (metadata2.mipLevels);

	mipImages[3] = LoadTexture (teapot->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata3 = mipImages[3].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource3 = CreateTextureResource (device.Get (), metadata3);
	ComPtr<ID3D12Resource> intermediateResource3 = UploadTextureData (textureResource3, mipImages[3], device.Get (), commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescTeapot{};
	srvDescTeapot.Format = metadata3.format;
	srvDescTeapot.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescTeapot.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescTeapot.Texture2D.MipLevels = UINT (metadata3.mipLevels);

	mipImages[4] = LoadTexture (Fence->GetModelData().material.textureFilePath);
	const DirectX::TexMetadata& metadata4 = mipImages[4].GetMetadata ();
	ComPtr<ID3D12Resource> textureResource4 = CreateTextureResource (device.Get (), metadata4);
	ComPtr<ID3D12Resource> intermediateResource4 = UploadTextureData (textureResource4, mipImages[4], device.Get (), commandList);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescFence{};
	srvDescFence.Format = metadata4.format;
	srvDescFence.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescFence.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDescFence.Texture2D.MipLevels = UINT (metadata4.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU[5];
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU[5];
	textureSrvHandleCPU[0] = GetCPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 1);
	textureSrvHandleGPU[0] = GetGPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 1);

	textureSrvHandleCPU[1] = GetCPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 2);
	textureSrvHandleGPU[1] = GetGPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 2);

	textureSrvHandleCPU[2] = GetCPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 3);
	textureSrvHandleGPU[2] = GetGPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 3);

	textureSrvHandleCPU[3] = GetCPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 4);
	textureSrvHandleGPU[3] = GetGPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 4);

	textureSrvHandleCPU[4] = GetCPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 5);
	textureSrvHandleGPU[4] = GetGPUDescriptorHandle (srvDescriptorHeap.Get (), descriptorSizeSRV, 5);
	//SRVの生成
	device->CreateShaderResourceView (textureResource0.Get (), &srvDescSphere, textureSrvHandleCPU[0]);
	device->CreateShaderResourceView (textureResource1.Get (), &srvDescPlane, textureSrvHandleCPU[1]);
	device->CreateShaderResourceView (textureResource2.Get (), &srvDescBunny, textureSrvHandleCPU[2]);
	device->CreateShaderResourceView (textureResource3.Get (), &srvDescTeapot, textureSrvHandleCPU[3]);
	device->CreateShaderResourceView (textureResource4.Get (), &srvDescFence, textureSrvHandleCPU[4]);

	//BGM再生
	SoundPlayWave (xAudio2.Get (), soundData1);

	/********入力デバイス組********/
	//キーボード情報の取得開始
	keyboard->Acquire ();
	//全キーの入力状態を取得する
	BYTE key[256] = {};

	//マウス
	mouse->Acquire ();
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
	Sprite* sprite = new Sprite (device.Get());
	sprite->Initialize ({ 0.0f, 0.0f, 0.0f }, { 640.0f, 360.0f });

	SphereModel* sphere = new SphereModel (device.Get (), 16);
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

	//------------------------------------------//
	//				ImGuiの初期化					//
	//------------------------------------------//
	IMGUI_CHECKVERSION ();
	ImGui::CreateContext ();
	ImGui::StyleColorsDark ();
	ImGuiIO& io = ImGui::GetIO ();
	ImFont* fontJP = io.Fonts->AddFontFromFileTTF (
	"Resources/AppliMincho/PottaOne-Regular.ttf", 17.0f, nullptr,
	io.Fonts->GetGlyphRangesJapanese ());
	io.FontDefault = fontJP;
	ImGui_ImplWin32_Init (hwnd);
	ImGui_ImplDX12_Init (device.Get (),
	swapChainDesc.BufferCount,
	rtvDesc.Format,
	srvDescriptorHeap.Get (),
	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart (),
	srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart ()
	);

	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		else {
			//入力デバイスの状態を取得
			//キーボード
			hr = keyboard->GetDeviceState (sizeof (key), key);
			if (FAILED (hr)) {
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
					keyboard->Acquire ();
				}
			}

			//前フレームの状態を保存
			bool isTabDown = (key[DIK_TAB] & 0x80);
			static bool wasTabDown = false;

			//マウス
			hr = mouse->GetDeviceState (sizeof (DIMOUSESTATE), &mouseState);
			if (FAILED (hr)) {
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
					mouse->Acquire ();
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

			// //フレームの先頭をImGuiに伝えてあげる
			ImGui_ImplDX12_NewFrame ();
			ImGui_ImplWin32_NewFrame ();
			ImGui::NewFrame ();

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
					debugCamera->Updata (hwnd, hr, keyboard, key, mouse, &mouseInput);
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

			//ImGuiの内部コマンドを生成する
			ImGui::Render ();

			//=======コマンド君達=======//
			//これから書きこむバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex ();
			//TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex].Get ();
			//遷移前(現在)のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//TransitionBarrierを張る
			commandList->ResourceBarrier (1, &barrier);
			//描画先のRTVを指定する
			commandList->OMSetRenderTargets (1, &rtvHandles[backBufferIndex], false, nullptr);
			//描画先のRTVとDSVを設定する
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart ();
			commandList->OMSetRenderTargets (1, &rtvHandles[backBufferIndex], false, &dsvHandle);
			//指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };	//青っぽい色。RGBAの順
			commandList->ClearRenderTargetView (rtvHandles[backBufferIndex], clearColor, 0, nullptr);
			//指定した深度で画面全体をクリアする
			commandList->ClearDepthStencilView (dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			//ImGui描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get () };
			commandList->SetDescriptorHeaps (1, descriptorHeaps);
			/*三角形描画！*/
			commandList->RSSetViewports (1, &viewport);					//Viewportを設定
			commandList->RSSetScissorRects (1, &scissorRect);			//Scissorを設定
			//RootSignatureを設定。PSOに設定しているけど別途設定が必要
			commandList->SetGraphicsRootSignature (rootSignature.Get ());
			commandList->SetPipelineState (graphicsPipelineState.Get ());		//PSOを設定
			//描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
			Fence->Draw (commandList.Get (), textureSrvHandleGPU[4], dierctionalLightResource.Get ());
			if (useSphere) {
				sphere->Draw (commandList.Get (), textureSrvHandleGPU[0], dierctionalLightResource.Get ());
			}
			if (usePlane) {
				plane->Draw (commandList.Get (), textureSrvHandleGPU[1], dierctionalLightResource.Get ());
			}
			if (useModel) {
				bunny->Draw (commandList.Get (), textureSrvHandleGPU[2], dierctionalLightResource.Get ());
			}
			if (useTeapot) {
				teapot->Draw (commandList.Get (), textureSrvHandleGPU[3], dierctionalLightResource.Get ());
			}
			if (useSprite) {
				sprite->Draw (commandList.Get(), textureSrvHandleGPU[0]);
			}
			//実際のImGui描画コマンドを詰む
			ImGui_ImplDX12_RenderDrawData (ImGui::GetDrawData (), commandList.Get ());
			//画面に描く処理はすべて終わり、画面に映すので、状態を遷移
			//今回はRenderTargetからPresentにする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrierを張る
			commandList->ResourceBarrier (1, &barrier);
			//コマンドリストの内容を確定させる。すべてのコマンドを詰んでからCloseすること
			hr = commandList->Close ();
			assert (SUCCEEDED (hr));

			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList.Get () };
			commandQueue->ExecuteCommandLists (1, commandLists);
			//GPUとOSに画面の交換を行うように通知する
			swapChain->Present (1, 0);
			//Fenceの値を更新する
			fenceValue++;
			//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal (fence.Get (), fenceValue);
			//Fenceの値が指定したSignal値にたどり着いているか確認する
			//GetCompletedValueの初期値はFence作成時に渡した初期値
			if (fence->GetCompletedValue () < fenceValue) {
				//指定した値にたどり着いていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion (fenceValue, fenceEvent);
				//イベント待つ
				WaitForSingleObject (fenceEvent, INFINITE);
			}
			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset ();
			assert (SUCCEEDED (hr));
			hr = commandList->Reset (commandAllocator.Get (), nullptr);
			assert (SUCCEEDED (hr));
		}
	}

	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown ();
	ImGui_ImplWin32_Shutdown ();
	ImGui::DestroyContext ();

	//------------解放処理-------------//
	// ================================
	//  入力系
	keyboard.Reset ();
	mouse.Reset ();
	directInput.Reset ();

	// ================================
	// GPU系
	CloseHandle (fenceEvent);

	// ================================
	// サウンド系
	xAudio2.Reset ();
	SoundUnload (&soundData1);  // バッファ解放

	// ================================
	// D3D12系（必要ならリソース解放）

	// ================================
	// COM系
	CoUninitialize ();

#ifdef _DEBUG
	ComPtr<ID3D12DebugDevice> debugDevice;
	device.As (&debugDevice); // 変換
	debugDevice->ReportLiveDeviceObjects (D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL);
#endif

	// ================================
	// ウィンドウ
	DestroyWindow (hwnd); // ← CloseWindow より DestroyWindow の方が自然（WM_DESTROY 発生）
	return 0;
};