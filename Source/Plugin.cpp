#include "Plugin.h"
using namespace juce;
String graphJSON(const bread::Graph& g){
 auto* root=new DynamicObject;root->setProperty("version",1);root->setProperty("name",String(g.name));
 Array<var> parts,wires;
 for(auto& p:g.parts){auto* o=new DynamicObject;o->setProperty("id",String(p.id));o->setProperty("type",String(p.type));o->setProperty("value",p.value);o->setProperty("x",p.x);o->setProperty("y",p.y);parts.add(var(o));}
 for(auto& w:g.wires){auto* o=new DynamicObject;o->setProperty("a",String(w.a));o->setProperty("b",String(w.b));o->setProperty("x",w.x);o->setProperty("y",w.y);wires.add(var(o));}
 root->setProperty("components",parts);root->setProperty("connections",wires);return JSON::toString(var(root));
}
bread::Graph parseGraph(const String& text){
 var root;auto result=JSON::parse(text,root);if(result.failed())throw std::runtime_error(result.getErrorMessage().toStdString());
 if(!root.isObject()||!root["components"].isArray()||!root["connections"].isArray())throw std::runtime_error("Expected components and connections arrays");
 if(root.hasProperty("version")&&(int)root["version"]!=1)throw std::runtime_error("Unsupported circuit version");
 bread::Graph g;g.name=root.getProperty("name","Imported circuit").toString().toStdString();
 for(auto& p:*root["components"].getArray())g.parts.push_back({p["id"].toString().toStdString(),p["type"].toString().toStdString(),(double)p["value"],(float)p.getProperty("x",300),(float)p.getProperty("y",200)});
 for(auto& w:*root["connections"].getArray()){
  if(w.isArray()&&w.size()==2)g.wires.push_back({w[0].toString().toStdString(),w[1].toString().toStdString()});
  else if(w.isObject())g.wires.push_back({w["a"].toString().toStdString(),w["b"].toString().toStdString(),(float)w.getProperty("x",0),(float)w.getProperty("y",0)});
  else throw std::runtime_error("Invalid connection");
 }
 bread::compile(g);return g;
}
BreadProcessor::BreadProcessor():AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),params(*this,nullptr,"controls",{
 std::make_unique<AudioParameterFloat>(ParameterID{"drive",1},"Input",NormalisableRange<float>(0.01f,2.0f,0.001f,0.4f),0.15f),
 std::make_unique<AudioParameterFloat>(ParameterID{"level",1},"Output",NormalisableRange<float>(0.0f,1.0f),0.25f),
 std::make_unique<AudioParameterFloat>(ParameterID{"mix",1},"Mix",NormalisableRange<float>(0.0f,1.0f),1.0f)
}){graph=bread::fuzz();submit(graph);}
bool BreadProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return (l.getMainOutputChannelSet()==AudioChannelSet::mono()||l.getMainOutputChannelSet()==AudioChannelSet::stereo())&&l.getMainInputChannelSet()==l.getMainOutputChannelSet();}
bread::Graph BreadProcessor::snapshot(){ScopedLock lock(graphLock);return graph;}
String BreadProcessor::submit(const bread::Graph& g){
 try{ScopedLock lock(graphLock);auto c=bread::compile(g);std::array<bread::Solver,2> built;for(auto& s:built)s.reset(c.net,rate.load());graph=g;{SpinLock::ScopedLockType m(mailbox);pending=built;ready=true;}++revision;return {};}
 catch(const std::exception& e){return e.what();}
}
void BreadProcessor::prepareToPlay(double sr,int){rate=sr;submit(snapshot());dcX[0]=dcX[1]=dcY[0]=dcY[1]=0;}
void BreadProcessor::processBlock(AudioBuffer<float>& b,MidiBuffer&){
 ScopedNoDenormals noDenormals;
 {SpinLock::ScopedTryLockType lock(mailbox);if(lock.isLocked()&&ready){old=active;active=pending;ready=false;fade=128;}}
 const float drive=params.getRawParameterValue("drive")->load(),level=params.getRawParameterValue("level")->load(),mix=params.getRawParameterValue("mix")->load();
 float peak=0;double hp=std::exp(-6.283185307*12/rate.load());
 for(int i=0;i<b.getNumSamples();++i){for(int ch=0;ch<b.getNumChannels();++ch){double dry=b.getSample(ch,i);double wet=active[ch].step(dry*drive);if(fade){double t=1-fade/128.0;wet=t*wet+(1-t)*old[ch].step(dry*drive);}double filtered=wet-dcX[ch]+hp*dcY[ch];dcX[ch]=wet;dcY[ch]=filtered;float out=(float)std::clamp(filtered*level*mix+dry*(1-mix),-1.0,1.0);b.setSample(ch,i,out);peak=std::max(peak,std::abs(out));}if(fade)--fade;}
 meter=peak;failures=active[0].failures;
}
void BreadProcessor::getStateInformation(MemoryBlock& data){auto state=params.copyState();state.setProperty("circuit",graphJSON(snapshot()),nullptr);if(auto xml=state.createXml())copyXmlToBinary(*xml,data);}
void BreadProcessor::setStateInformation(const void* data,int size){if(auto xml=getXmlFromBinary(data,size)){auto state=ValueTree::fromXml(*xml);if(state.isValid()&&state.hasProperty("circuit")){try{auto error=submit(parseGraph(state["circuit"].toString()));if(error.isEmpty())params.replaceState(state);}catch(...){}}}}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new BreadProcessor;}
