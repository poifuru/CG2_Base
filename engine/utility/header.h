#pragma once
#ifndef __HLSL_VERSION
#include <Windows.h>  // HLSL側では __HLSL_VERSION が定義されてないのでスキップできる
#endif
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <cassert>
#include <filesystem>	//ファイルやディレクトリに関する操作を行うためのライブラリ
#include <fstream>	//ファイルに書いたり読んだりするライブラリ
#include <chrono>	//時間を扱うライブラリ
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#include <strsafe.h>	
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")	
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <wrl.h>
using namespace Microsoft::WRL;
#define DIRECTINPUT_VERSION		0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#pragma	comment(lib, "dinput8.lib")
#pragma	comment(lib, "dxguid.lib")