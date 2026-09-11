#include "../Source/Circuit.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
void require(bool x,const char* message){if(!x){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
 bread::Graph divider;divider.parts={{"R1","resistor",10000},{"R2","resistor",10000}};divider.wires={{"input","R1.1"},{"R1.2","output"},{"output","R2.1"},{"R2.2","ground"}};
 bread::Solver s;auto c=bread::compile(divider);s.reset(c.net,48000);require(std::abs(s.step(1)-.5)<1e-4,"divider voltage");
 divider.wires.pop_back();s.reset(bread::compile(divider).net,48000);require(s.step(1)>.99,"disconnect changes network");
 divider.wires.push_back({"battery","ground"});bool rejected=false;try{bread::compile(divider);}catch(...){rejected=true;}require(rejected,"supply short rejected");
 bread::Graph rc;rc.parts={{"R","resistor",10000},{"C","capacitor",1e-6}};rc.wires={{"input","R.1"},{"R.2","output"},{"output","C.1"},{"C.2","ground"}};
 s.reset(bread::compile(rc).net,48000);double y=0;for(int i=0;i<480;++i)y=s.step(1);require(std::abs(y-(1-std::exp(-1)))<.002,"RC time constant");
 auto f=bread::compile(bread::fuzz());s.reset(f.net,48000);std::cout<<"Q1 collector "<<s.v[f.nodes["Q1.c"]]<<" Q2 collector "<<s.v[f.nodes["Q2.c"]]<<'\n';
 double energy=0;auto t=std::chrono::steady_clock::now();for(int i=0;i<48000;++i){double out=s.step(.06*std::sin(i*6.283185307*110/48000));require(std::isfinite(out)&&std::abs(out)<20,"finite bounded fuzz");energy+=out*out;}
 require(energy>1,"fuzz produces audio");std::cout<<"Failures "<<s.failures<<" energy "<<energy<<"\n";require(s.failures<100,"solver convergence");std::cout<<"48k samples: "<<std::chrono::duration<double>(std::chrono::steady_clock::now()-t).count()<<" seconds; RMS "<<std::sqrt(energy/48000)<<" failures "<<s.failures<<'\n';
 // Verify a transistor change actually affects the solved circuit.
 auto changed=bread::fuzz();changed.parts[1].value=30;bread::Solver other;auto changedNet=bread::compile(changed);other.reset(changedNet.net,48000);require(std::abs(other.v[changedNet.nodes["Q1.c"]]-s.v[f.nodes["Q1.c"]])>.001,"transistor beta changes bias");
 std::cout<<"PASS\n";
}
