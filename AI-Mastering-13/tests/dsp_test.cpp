#include "MasteringDSP.h"
#include <iostream>
#include <random>
#include <stdexcept>
void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
std::array<float,am13::count> neutral(){std::array<float,am13::count> p{};std::copy_n(am13::defaults,am13::count,p.begin());return p;}
int main(){
    for(double sr:{22050.,44100.,48000.,96000.,192000.}) {
        am13::Engine e;e.prepare(sr);auto p=neutral();
        for(int i=0;i<10000;++i){float a=0.3f*std::sin(float(i)*.11f),b=0.2f*std::cos(float(i)*.03f),l=a,r=b;e.sample(l,r,p,0,-1,false);require(std::abs(l-a)<1.e-6f&&std::abs(r-b)<1.e-6f,"Neutral should null below ceiling");}
        p[9]=18;p[1]=100;p[3]=70;p[7]=100;p[8]=200;
        for(int i=0;i<int(sr);++i){float l=0.9f*std::sin(float(i)*.09f),r=0.8f*std::cos(float(i)*.06f);e.sample(l,r,p,12,-1,false);require(std::isfinite(l)&&std::isfinite(r),"Non-finite signal");require(std::max(std::abs(l),std::abs(r))<=am13::gain(-1)+1.e-5f,"Sample ceiling exceeded");}
        for(int i=0;i<int(sr);++i){float l=.2f,r=-.1f;e.sample(l,r,p,12,-1,true);if(i>int(sr*.8))require(std::abs(l-.2f)<1.e-5f&&std::abs(r+.1f)<1.e-5f,"Bypass did not settle");}
        e.prepare(sr);p=neutral();p[8]=0;
        for(int i=0;i<int(sr);++i){float l=.3f,r=-.2f;e.sample(l,r,p,0,-1,false);if(i>int(sr*.8))require(std::abs(l-r)<1.e-5f,"Width zero must be mono");}
    }
    am13::Engine e;e.prepare(48000);auto p=neutral();std::mt19937 rng(17);std::uniform_real_distribution<float> audio(-4,4),pct(0,100);
    for(int i=0;i<300000;++i){if(i%256==0){for(auto& v:p)v=pct(rng);p[1]-=50;p[2]-=50;p[5]-=50;p[8]*=2;p[9]*=.18f;p[12]=p[12]*.3f-24;}float l=audio(rng),r=audio(rng);e.sample(l,r,p,0,-1,false);require(std::isfinite(l)&&std::isfinite(r),"Fuzz instability");require(std::max(std::abs(l),std::abs(r))<8,"Unexpected gain runaway");}
    e.prepare(48000);p=neutral();for(int i=0;i<100000;++i){float l=0,r=0;e.sample(l,r,p,0,-1,false);require(l==0&&r==0,"Silence must remain silent");}
    std::cout<<"PASS: neutral null, ceiling, bypass, mono, silence, parameter fuzz; 5 sample rates\n";
}
