#pragma once
#include <array>
#include <string>
#include <algorithm>
#include <cmath>
namespace orbit33 {
constexpr int presetCount = 14000;
constexpr std::array<const char*,20> categories {{
 "Grand Keys", "Electric Keys", "Organs", "Bells", "Mallets", "Plucks", "Guitars", "Basses", "Sub Basses", "Analog Leads",
 "Digital Leads", "Warm Pads", "Atmospheres", "Strings", "Brass", "Winds", "Choirs", "Synth Keys", "Textures", "FX" }};
// Stable factory program mapping: 20 families x 100 recipes x 7 editions.
struct Patch { int engine; float shape, attack, decay, sustain, release, tone, detune, reverb, echo, delay, motion; };
inline Patch patch(int index) {
 index=std::clamp(index,0,presetCount-1); int family=index/700, variant=index%100, edition=(index%700)/100;
 static const std::array<Patch,20> base {{
 {1,.24f,.003f,.8f,.15f,.4f,6000,2,.15f,.04f,.3f,0},
 {1,.68f,.004f,1.2f,.2f,.5f,4200,4,.2f,.08f,.3f,.1f},
 {2,.45f,.008f,.12f,.9f,.12f,7500,3,.15f,.01f,.2f,0},
 {3,.4f,.002f,2,.05f,1.8f,9000,6,.35f,.15f,.42f,0},
 {3,.08f,.002f,.5f,.02f,.4f,6500,1,.15f,.05f,.22f,0},
 {0,.25f,.002f,.35f,.05f,.25f,7000,8,.18f,.22f,.27f,0},
 {4,.3f,.004f,.7f,.12f,.18f,4000,3,.12f,.04f,.15f,0},
 {0,.6f,.004f,.3f,.65f,.12f,1800,5,.01f,.01f,.18f,0},
 {5,.12f,.004f,.2f,.8f,.18f,450,0,0,0,.2f,0},
 {0,.7f,.008f,.18f,.7f,.22f,6500,12,.15f,.16f,.3f,.04f},
 {1,.85f,.01f,.35f,.6f,.35f,8000,8,.2f,.25f,.32f,.08f},
 {0,.35f,.8f,1,.75f,2.5f,2200,15,.5f,.12f,.45f,.15f},
 {6,.45f,1.5f,1.8f,.8f,3.5f,3500,20,.65f,.25f,.6f,.3f},
 {7,.65f,.35f,.8f,.85f,1.2f,5000,9,.35f,.06f,.35f,.05f},
 {7,.85f,.06f,.3f,.7f,.4f,3800,5,.2f,.04f,.25f,0},
 {5,.35f,.09f,.3f,.8f,.4f,3200,3,.25f,.08f,.35f,.08f},
 {6,.75f,.45f,.6f,.8f,1.8f,2800,11,.5f,.1f,.45f,.12f},
 {4,.65f,.01f,.5f,.3f,.6f,5500,7,.25f,.2f,.3f,.04f},
 {6,.2f,.8f,2,.6f,3,6000,22,.6f,.3f,.5f,.35f},
 {3,.9f,.005f,.9f,.12f,2,8500,25,.6f,.4f,.65f,.5f}
 }};
 auto p=base[family];
 const float x=float(variant%10)/9, y=float(variant/10)/9;
 p.shape=std::clamp(p.shape*.55f+x*.4f,0.f,1.f);
 p.tone=std::clamp(p.tone*(.5f+1.2f*y),120.f,14000.f);
 p.attack*=.6f+1.4f*y; p.decay*=.55f+1.3f*x;
 p.release=std::min(5.f,p.release*(.6f+1.1f*y)); p.detune+=x*7;
 p.reverb=std::min(.8f,p.reverb*(.6f+.7f*y));
 p.shape=std::min(1.f,p.shape+edition*.012f);
 p.detune+=edition*.8f;p.tone=std::min(14000.f,p.tone*(1+edition*.04f));
 return p;
}
inline std::string name(int i) {
 i=std::clamp(i,0,presetCount-1);
 static const char* colors[]={"Royal","Midnight","Velvet","Crystal","Golden","Neon","Vintage","Lunar","Electric","Deep"};
 static const char* forms[]={"Glow","Silk","Prism","Aura","Pulse","Dream","Horizon","Bloom","Ember","Crown"};
 int v=i%100; const char* editions[]={"Core","Air","Dust","Glass","Night","Wide","Solar"};
 return std::string(categories[i/700])+" / "+colors[v/10]+" "+forms[v%10]+" - "+editions[(i%700)/100];
}
// Additive oscillators: omit harmonics above Nyquist to limit aliasing.
inline float oscillator(int engine, double phase, float shape, double frequency, double rate) {
 constexpr double pi=3.14159265358979323846; float out=0, norm=0;
 for(int h=1;h<=24 && h*frequency<rate*.45;++h) {
  float a=0;
  switch(engine) {
   case 0: a=(h==1?1.f:shape/h); break;
   case 1: a=std::exp(-h*(.7f-shape*.55f)) * ((h%2)?1.f:.65f); break;
   case 2: a=(h==1?1.f:(h==2?.6f:(h==4?shape:.0f))); break;
   case 3: a=(h==1?1.f:(h==3?shape*.8f:(h==7?shape*.4f:0))); break;
   case 4: a=(h%2?1.f:-.3f)*std::pow(float(h),-1.5f+shape*.5f); break;
   case 5: a=(h==1?1.f:(h==2?shape*.4f:0)); break;
   case 6: a=std::exp(-std::abs(h-(2+shape*7))*.65f)/float(h)+ (h==1?.6f:0); break;
   default: a=(h%2?1.f:shape)*1.f/h; break;
  }
  out+=a*float(std::sin(phase*h*2*pi)); norm+=std::abs(a);
 }
 return out/std::max(1.f,norm);
}
}
