#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
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
#include "Matrix.h"
#include "struct.h"
#include <vector>