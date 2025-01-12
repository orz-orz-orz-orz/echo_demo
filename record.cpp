#include <vector>
#include <deque>
#include <array>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <Eigen/Dense>
#include <cmath>

extern "C"{
    #define MINIAUDIO_IMPLEMENTATION
    #include "miniaudio.h"
}

// 產生矩陣
using std::vector;
using std::array;
using namespace Eigen;

template<int L>
struct GKFilter {
    Matrix<float, L, 1> h_hat;
    float r_mu = 1e-5;
    float var_w_hat = 1e-5;
    float var_y_hat = 0.0;
    float var_d_hat = 0.0;
    float var_v_hat = 1e-5;

    float forward(
        Vector<float, L>& x, 
        float d,
        bool update=false
    ){
        // std::cout << x << std::endl << std::endl;
        // 產生 X
        Vector<float, L> X = x.reverse();
        // RowVector<float, L> X_tr = X.transpose();
        if(update){
            // from (63)-(68)
            float r_m = r_mu + var_w_hat;
            float delta = var_v_hat / r_m;
            float y_hat = X.dot(h_hat); //(X_tr * h_hat)(0);
            float e = d - y_hat;
            float S_x = X.norm();//(X_tr * X)(0);
            Vector<float, L> u = (X * e) / (S_x + delta);
            h_hat += u;
            var_w_hat = u.squaredNorm() / L + 1e-8;
            r_mu = (1 - S_x/(S_x + delta)/ L ) * r_m;

            // from (79)-(81)
            var_d_hat = 0.1 * var_d_hat + 0.9 * d * d;
            var_y_hat = 0.1 * var_y_hat + 0.9 * y_hat * y_hat;
            var_v_hat = std::abs(var_d_hat - var_y_hat) + 1e-8;

            return e;
        } else {
            float e = d - X.dot(h_hat); //(X_tr * h_hat)(0);
            return e;
        }
    }
};



template<int L>
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    //std::cout << "read " << frameCount << " frames" << std::endl;
    //MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
    //MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);
    const ma_int16* input = (const ma_int16*) pInput; 
    ma_int16*      output = (ma_int16*) pOutput;
    GKFilter<L>*    state = (GKFilter<L>*) pDevice->pUserData;
    
    static std::vector<float> x_raw; // 用來存之前的輸出。
    static Vector<float, L> x;

    size_t N = frameCount;
    if(x_raw.size() < L+N-1){
        size_t count = L+N-1 - x_raw.size();
        x_raw.insert(x_raw.begin(), count, 0.f); // padding left;
    }
    size_t offset = x_raw.size() - (L+N-1);
    // 使用 Map 避免複製。
    Map<Matrix<float, Dynamic, L>, 0, Stride<1, 1> > X(x_raw.data() + offset, N, L, Stride<1, 1>());
    // std::cout << X << std::endl;
    // std::cout << "------" << std::endl;
    static float r = 0.0f;
    static size_t g = 0;
    for(size_t n=0; n< N; n++, g++){
        float d = ((float) input[n]) / 32767.f;
        Vector<float, L> x = X(n, all); 
        float e = state->forward(x, d, n % 10 == 0); // 每十點做一次參數重估，不然運算量太大。
        // 加一個 iir 快速濾掉高頻
        // r = 0.7 * e + 0.3 * r;
        r = e;
        ma_int16 v = std::clamp(r, -1.0f, 1.0f) * 32767;
        x_raw.push_back(r); // 加入之前輸出的點中
        output[n] = v;
    }
}

int main(int argc, const char* argv[]){
    // 一些設定
    const int L = 10; // 開太長運算速度不夠，另外不是越長越好。

    // 5-tuple state 
    GKFilter<L> state;
    ma_result result;
    ma_device_config device_config;
    ma_device device;

    device_config = ma_device_config_init(ma_device_type_duplex);
    device_config.sampleRate         = 16000;
    device_config.capture.pDeviceID  = NULL;
    device_config.capture.format     = ma_format_s16;
    device_config.capture.channels   = 1;
    device_config.playback.pDeviceID = NULL;
    device_config.playback.format    = ma_format_s16;
    device_config.playback.channels  = 1;
    device_config.dataCallback       = data_callback<L>;
    device_config.pUserData          = (void*)&state;

    result = ma_device_init(NULL, &device_config, &device);
    if (result != MA_SUCCESS) {
        return result;
    }
    // 開始播放
    ma_device_start(&device);
    std::cout << "注意，如果喇叭比你還要大聲，還是會爆音。" << std::endl;
    std::cout << "按 ENTER 鍵停止錄音。" << std::endl;
    char _ = getchar(); 

    ma_device_uninit(&device);

    return 0;

}



