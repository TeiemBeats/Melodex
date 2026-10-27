#include "../Source/PresetBank.h"
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
int main(){
 std::set<std::string> names,recipes;
 for(int i=0;i<14000;++i){auto p=orbit33::patch(i);names.insert(orbit33::name(i));
 std::ostringstream key;key<<p.engine<<'/'<<p.shape<<'/'<<p.tone<<'/'<<p.attack<<'/'<<p.decay<<'/'<<p.detune;recipes.insert(key.str());
 assert(p.engine>=0&&p.engine<=7&&p.attack>0&&p.release<=5&&p.shape>=0&&p.shape<=1&&p.tone<=16000);
 double energy=0;for(int t=0;t<1024;++t){auto s=orbit33::oscillator(p.engine,t*440./48000,p.shape,440,48000);assert(std::isfinite(s)&&std::abs(s)<=1.001f);energy+=s*s;}assert(energy>1);
 }
 assert(names.size()==14000&&recipes.size()==14000);
 assert(orbit33::name(-1)==orbit33::name(0));assert(orbit33::name(14000)==orbit33::name(13999));
 std::cout<<"PASS: 14000 unique named recipes, finite audible oscillator output, bounds\n";
}
