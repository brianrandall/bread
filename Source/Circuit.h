#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>

namespace bread {
constexpr int maxNodes=32, maxParts=48;
struct Part { std::string id, type; double value=10000; float x=200,y=200; };
struct Wire { std::string a,b; float x=0,y=0; };
struct Graph { std::vector<Part> parts; std::vector<Wire> wires; std::string name="Untitled"; };
inline std::vector<std::string> pins(const Part& p) {
 if(p.type=="npn") return {"b","c","e"};
 return {"1","2"};
}
enum Kind { resistor, capacitor, diode, npn };
struct Element { Kind kind=resistor; int a=0,b=0,c=0; double value=0, history=0; };
struct Network {
 std::array<Element,maxParts> parts{}; int count=0,n=0,output=0;
 // Node 0=ground, 1=audio input, 2=9V battery; unknown nodes start at 3.
};
struct Compiled { Network net; std::map<std::string,int> nodes; };
inline Compiled compile(const Graph& graph) {
 if(graph.parts.size()>maxParts) throw std::runtime_error("Maximum 48 components");
 std::map<std::string,int> ids; std::vector<int> parent;
 auto add=[&](const std::string& s){ if(ids.count(s)) throw std::runtime_error("Duplicate endpoint: "+s); int i=(int)parent.size();ids[s]=i;parent.push_back(i); };
 add("ground");add("input");add("battery");add("output");
 std::map<std::string,bool> seen;
 for(auto& p:graph.parts) {
  if(p.id.empty() || p.id.find('.')!=std::string::npos || seen.count(p.id)) throw std::runtime_error("Invalid or duplicate component ID");
  seen[p.id]=true;
  if(p.type!="resistor" && p.type!="capacitor" && p.type!="diode" && p.type!="npn") throw std::runtime_error("Unsupported component: "+p.type);
  double lo=p.type=="capacitor"?1e-12:p.type=="diode"?1e-16:p.type=="npn"?1:1;
  double hi=p.type=="capacitor"?0.01:p.type=="diode"?1e-3:p.type=="npn"?1000:1e9;
  if(!std::isfinite(p.value)||p.value<lo||p.value>hi) throw std::runtime_error("Component value outside supported range: "+p.id);
  if(!std::isfinite(p.x)||!std::isfinite(p.y)) throw std::runtime_error("Invalid component position");
  for(auto& pin:pins(p)) add(p.id+"."+pin);
 }
 auto root=[&](int a){while(parent[a]!=a)a=parent[a];return a;};
 if(graph.wires.size()>256) throw std::runtime_error("Maximum 256 wires");
 for(auto& w:graph.wires){
  if(!ids.count(w.a)||(!w.b.empty()&&!ids.count(w.b))) throw std::runtime_error("Unknown wire endpoint");
  if(!w.b.empty())parent[root(ids[w.a])]=root(ids[w.b]);
 }
 std::map<int,int> mapped;
 for(int i=0;i<3;++i){int r=root(i);if(mapped.count(r))throw std::runtime_error("Cannot short input, battery or ground together");mapped[r]=i;}
 int next=3; Compiled result;
 for(auto& entry:ids){int r=root(entry.second);if(!mapped.count(r))mapped[r]=next++;result.nodes[entry.first]=mapped[r];}
 if(next>maxNodes)throw std::runtime_error("Maximum 32 electrical nodes; connect unused pins or remove parts");
 result.net.n=next;result.net.output=result.nodes["output"];
 for(auto& p:graph.parts){auto ps=pins(p);Element e;e.kind=p.type=="resistor"?resistor:p.type=="capacitor"?capacitor:p.type=="diode"?diode:npn;e.value=p.value;e.a=result.nodes[p.id+"."+ps[0]];e.b=result.nodes[p.id+"."+ps[1]];if(e.kind==npn)e.c=result.nodes[p.id+".e"];result.net.parts[result.net.count++]=e;}
 return result;
}
class Solver {
public:
 Network net{}; std::array<double,maxNodes> v{}; double rate=48000,battery=9; unsigned failures=0;
 void reset(Network n,double sampleRate){net=n;rate=sampleRate;v.fill(0);failures=0;for(int i=0;i<1024;++i)step(0);failures=0;}
 double step(double input) {
  v[0]=0;v[1]=input;v[2]=battery;
  std::array<double,maxNodes> previous=v;
  bool converged=false;
  for(int iteration=0;iteration<40;++iteration){
   double a[maxNodes][maxNodes]{};double f[maxNodes]{};
   auto branch=[&](int p,int q,double current,double g){
    if(p>=3){f[p]+=current;a[p][p]+=g;if(q>=3)a[p][q]-=g;}
    if(q>=3){f[q]-=current;a[q][q]+=g;if(p>=3)a[q][p]-=g;}
   };
   auto transfer=[&](int p,int q,int cp,int cq,double current,double g){
    if(p>=3){f[p]+=current;if(cp>=3)a[p][cp]+=g;if(cq>=3)a[p][cq]-=g;}
    if(q>=3){f[q]-=current;if(cp>=3)a[q][cp]-=g;if(cq>=3)a[q][cq]+=g;}
   };
   auto junction=[](double voltage,double is){double x=voltage/0.02585;double ex=std::exp(std::min(32.0,x));double g=is*ex/0.02585;double current=is*(ex-1);if(x>32)current+=g*(voltage-32*0.02585);return std::array<double,2>{current,g};};
   for(int i=3;i<net.n;++i){a[i][i]=1e-9;f[i]=1e-9*v[i];}
   for(int i=0;i<net.count;++i){auto& e=net.parts[i];double d=v[e.a]-v[e.b];
    if(e.kind==resistor)branch(e.a,e.b,d/e.value,1/e.value);
    if(e.kind==capacitor){double g=e.value*rate;branch(e.a,e.b,g*(d-e.history),g);}
    if(e.kind==diode){auto j=junction(d,e.value);branch(e.a,e.b,j[0],j[1]);}
    if(e.kind==npn){
     auto be=junction(v[e.a]-v[e.c],1e-14);auto bc=junction(v[e.a]-v[e.b],1e-14);
     double af=e.value/(e.value+1),ar=0.5;
     branch(e.a,e.c,be[0],be[1]);branch(e.a,e.b,bc[0],bc[1]);
     transfer(e.b,e.a,e.a,e.c,af*be[0],af*be[1]);
     transfer(e.c,e.a,e.a,e.b,ar*bc[0],ar*bc[1]);
    }
   }
   bool valid=true;
   for(int k=3;k<net.n;++k){int pivot=k;for(int r=k+1;r<net.n;++r)if(std::abs(a[r][k])>std::abs(a[pivot][k]))pivot=r;
    if(std::abs(a[pivot][k])<1e-20){valid=false;break;}
    if(pivot!=k){for(int c=k;c<net.n;++c)std::swap(a[k][c],a[pivot][c]);std::swap(f[k],f[pivot]);}
    for(int r=k+1;r<net.n;++r){double m=a[r][k]/a[k][k];for(int c=k+1;c<net.n;++c)a[r][c]-=m*a[k][c];f[r]-=m*f[k];}
   }
   if(!valid)break;
   double delta[maxNodes]{};double largest=0;
   for(int r=net.n-1;r>=3;--r){double t=-f[r];for(int c=r+1;c<net.n;++c)t-=a[r][c]*delta[c];delta[r]=t/a[r][r];if(!std::isfinite(delta[r]))valid=false;largest=std::max(largest,std::abs(delta[r]));}
   if(!valid)break;
   double scale=largest>1?1/largest:1;
   for(int r=3;r<net.n;++r)v[r]+=delta[r]*scale;
   if(largest<1e-6){converged=true;break;}
  }
  if(!converged){++failures;v=previous;return 0;}
  for(int i=0;i<net.count;++i){auto& e=net.parts[i];if(e.kind==capacitor)e.history=v[e.a]-v[e.b];}
  return v[net.output];
 }
};
inline Graph fuzz(){
 Graph g;g.name="Two-transistor silicon fuzz";
 g.parts={{"C1","capacitor",2.2e-6,210,200},{"Q1","npn",180,380,200},{"R1","resistor",33000,380,80},{"R2","resistor",100000,200,360},{"Q2","npn",250,620,200},{"R3","resistor",8200,620,80},{"R4","resistor",1000,620,360},{"C2","capacitor",20e-6,810,360},{"C3","capacitor",0.01e-6,820,200},{"R5","resistor",100000,1000,360}};
 auto w=[&](std::string a,std::string b){g.wires.push_back({a,b});};
 w("input","C1.1");w("C1.2","Q1.b");w("Q1.e","ground");w("battery","R1.1");w("R1.2","Q1.c");w("Q1.c","Q2.b");w("Q1.b","R2.1");w("R2.2","Q2.e");w("battery","R3.1");w("R3.2","Q2.c");w("Q2.e","R4.1");w("R4.2","ground");w("Q2.e","C2.1");w("C2.2","ground");w("Q2.c","C3.1");w("C3.2","output");w("output","R5.1");w("R5.2","ground");return g;
}
}
