#pragma once

#ifndef __HLSL_VERSION
#include <Windows.h>  // HLSL側では __HLSL_VERSION が定義されてないのでスキップできる
#endif

//***Windows固有のもの***//
//ミニダンプ生成、シンボル解決、スタックトレース取得などのデバッグ支援ライブラリ。
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

//安全な文字列操作（バッファオーバーフロー防止）。
#include <strsafe.h>	
//**********************//