#pragma once
//***DirectX12関連***//
//GPUにコマンドを送るための中核API
#include <d3d12.h>// Direct3D 12 本体

//GPUデバッグ情報を得るための追加レイヤー
#include <d3d12sdklayers.h>// D3D12のデバッグレイヤー
#pragma comment(lib, "d3d12.lib")

//マルチアダプタやスワップチェイン作成
#include <dxgi1_6.h> // DXGI (GPU/モニター/スワップチェイン管理)
#pragma comment(lib, "dxgi.lib")

//DXGIデバッグ出力
#include <dxgidebug.h> // DXGI のデバッグ情報
#pragma comment(lib, "dxguid.lib")

//HLSLをコンパイルする新しいコンパイラ(DXC)
#include <dxcapi.h>// DXC (DirectXシェーダーコンパイラ)
#pragma comment(lib, "dxcompiler.lib")	
//*****************************//