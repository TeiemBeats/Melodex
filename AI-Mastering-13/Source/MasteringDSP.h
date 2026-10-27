#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace am13 {
constexpr int count = 13;
inline constexpr const char* ids[count] = {"clean","bass","tone","compress","dynamic","shape","excite","analog","width","loud","clip","limit","output"};
inline constexpr const char* names[count] = {"Clean","Bass","Tone","Compress","Dynamic","Shape","Excite","Analog","Width","Loud","Clipper","Limiter","Output"};
inline constexpr float defaults[count] = {0,0,0,0,0,0,0,0,100,0,0,100,0};
inline float gain(float db) { return std::pow(10.0f, db/20.0f); }
struct Engine {
    double rate=48000;
    std::array<float,count> values{};
    std::array<float,2> dc{}, low{}, mid{}, envelope{}, transient{};
    float detector=0, reduction=1, limiter=1, mix=1, inputGain=1, ceiling=0.89125094f;
    float smooth=0, aLow=0, aMid=0, aDC=0, attack=0, release=0, fast=0, slow=0;
    void prepare(double sr) {
        rate=sr; dc={}; low={}; mid={}; envelope={}; transient={};
        detector=0; reduction=limiter=1; mix=1; inputGain=1; ceiling=gain(-1);
        for(int i=0;i<count;++i) values[i]=defaults[i];
        smooth=1-std::exp(-1.0/(0.02*sr));
        aLow=1-std::exp(-6.283185307*160/sr); aMid=1-std::exp(-6.283185307*3200/sr);
        aDC=1-std::exp(-6.283185307*25/sr);
        attack=std::exp(-1.0/(0.012*sr)); release=std::exp(-1.0/(0.15*sr));
        fast=std::exp(-1.0/(0.002*sr)); slow=std::exp(-1.0/(0.045*sr));
    }
    void sample(float& left, float& right, const std::array<float,count>& target,
                float inputDb, float ceilingDb, bool bypass) {
        for(int i=0;i<count;++i) values[i] += smooth*(target[i]-values[i]);
        inputGain += smooth*(gain(inputDb)-inputGain);
        ceiling += smooth*(gain(ceilingDb)-ceiling);
        mix += smooth*((bypass?0.0f:1.0f)-mix);
        const float dryL=left, dryR=right;
        float x[2]={left*inputGain,right*inputGain};
        float peak=0;
        for(int c=0;c<2;++c) {
            dc[c]+=aDC*(x[c]-dc[c]); x[c]-=dc[c]*(values[0]*0.01f);
            low[c]+=aLow*(x[c]-low[c]); mid[c]+=aMid*(x[c]-mid[c]);
            x[c]+=low[c]*(gain(values[1]*0.12f)-1);
            x[c]+=(x[c]-mid[c])*(gain(values[2]*0.09f)-1);
            float a=std::abs(x[c]);
            envelope[c]=fast*envelope[c]+(1-fast)*a;
            transient[c]=slow*transient[c]+(1-slow)*a;
            float delta=std::clamp((envelope[c]-transient[c])*4.0f,-0.5f,0.5f);
            x[c]*=std::clamp(1+delta*values[5]*0.02f,0.25f,2.0f);
            const float high=x[c]-mid[c];
            const float dyn=std::max(0.0f,std::abs(high)-0.08f)*values[4]*0.04f;
            x[c]-=high*std::min(0.8f,dyn);
            peak=std::max(peak,std::abs(x[c]));
        }
        detector=(peak>detector?attack:release)*detector+(1-(peak>detector?attack:release))*peak;
        float threshold=gain(-6-values[3]*0.24f);
        float ratio=1+values[3]*0.05f;
        float gr=detector>threshold?std::pow(threshold/std::max(detector,1.e-9f),1-1/ratio):1;
        reduction+=(1-release)*(gr-reduction);
        for(int c=0;c<2;++c) {
            x[c]*=reduction;
            float high=x[c]-mid[c];
            x[c]+=(std::tanh(high*3)-high*3)*values[6]*0.0015f;
            float drive=1+values[7]*0.04f;
            float sat=std::tanh(x[c]*drive)/drive;
            x[c]+=(sat-x[c])*values[7]*0.01f;
        }
        const float m=(x[0]+x[1])*0.5f, s=(x[0]-x[1])*0.5f*values[8]*0.01f;
        x[0]=m+s; x[1]=m-s;
        float out=gain(values[9]+values[12]);
        float clipLevel=gain(-values[10]*0.06f);
        for(auto& v:x) v=std::clamp(v*out,-clipLevel,clipLevel);
        peak=std::max(std::abs(x[0]),std::abs(x[1]));
        const float needed=peak>ceiling?ceiling/std::max(peak,1.e-9f):1;
        limiter=needed<limiter?needed:release*limiter+(1-release)*needed;
        const float wet=values[11]*0.01f;
        for(auto& v:x) v*=1+wet*(limiter-1);
        left=dryL+mix*(x[0]-dryL); right=dryR+mix*(x[1]-dryR);
    }
};
}
