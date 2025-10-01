#pragma once
//#ifndef __HLSL_VERSION
//#include <Windows.h>  // HLSL側では __HLSL_VERSION が定義されてないのでスキップできる
//#endif


//#include <wrl.h>
//using namespace Microsoft::WRL;

//***ユーティリティライブラリ***//
//#include <cstdint>// 固定幅整数型 (uint32_t, int64_t など)
//#include <string>// std::string
//#include <format>// C++20のformat() 文字列整形
//#include <cassert>// assert() デバッグ用
//#include <filesystem>	//ファイルやディレクトリに関する操作を行うためのライブラリ
//#include <fstream>	//ファイルに書いたり読んだりするライブラリ
//#include <chrono>	//時間を扱うライブラリ
//#include <sstream>// stringstream
//#include <vector>// 可変長配列
//#define _USE_MATH_DEFINES
//#include <math.h>// sin, cos, M_PI 定義など
//*****************************//

////***DirectX12関連***//
////GPUにコマンドを送るための中核API
//#include <d3d12.h>// Direct3D 12 本体
//
////GPUデバッグ情報を得るための追加レイヤー
//#include <d3d12sdklayers.h>// D3D12のデバッグレイヤー
//#pragma comment(lib, "d3d12.lib")
//
////マルチアダプタやスワップチェイン作成
//#include <dxgi1_6.h> // DXGI (GPU/モニター/スワップチェイン管理)
//#pragma comment(lib, "dxgi.lib")
//
////DXGIデバッグ出力
//#include <dxgidebug.h> // DXGI のデバッグ情報
//#pragma comment(lib, "dxguid.lib")
//
////HLSLをコンパイルする新しいコンパイラ(DXC)
//#include <dxcapi.h>// DXC (DirectXシェーダーコンパイラ)
//#pragma comment(lib, "dxcompiler.lib")	
////*****************************//



