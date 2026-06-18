#include "Triangle.hlsli"

PS_Input main(VS_Input input)
{
    PS_Input output;
    // 今回はカメラ行列なしの等倍で、そのまま画面に出力する
    output.sv_pos = input.pos;
    output.uv = input.uv;
    return output;
}