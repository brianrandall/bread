#include "Plugin.h"
using namespace juce;
class BreadEditor : public AudioProcessorEditor,private Timer {
 BreadProcessor& proc;bread::Graph graph;int rev=0,selected=-1;String first,status="Click two pins to connect. Right-click a pin to unplug a wire.";
 TextButton preset{"Fuzz preset"},load{"Open JSON"},save{"Save JSON"},addR{"+ Resistor"},addC{"+ Capacitor"},addQ{"+ NPN"},addD{"+ Diode"},remove{"Remove"};
 TextEditor value;Slider drive,level,mix;Label driveLabel,levelLabel,mixLabel;
 std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> da,la,ma;
 std::unique_ptr<FileChooser> chooser;
 Point<float> dragOffset;
 std::map<std::string,Point<float>> points(){std::map<std::string,Point<float>> p{{"input",{55,290}},{"output",{1100,290}},{"battery",{55,170}},{"ground",{55,500}}};for(auto& c:graph.parts){if(c.type=="npn"){p[c.id+".b"]={c.x-44,c.y+130};p[c.id+".c"]={c.x+44,c.y+110};p[c.id+".e"]={c.x+44,c.y+150};}else{p[c.id+".1"]={c.x-44,c.y+130};p[c.id+".2"]={c.x+44,c.y+130};}}return p;}
 void apply(){auto error=proc.submit(graph);if(error.isNotEmpty()){status=error;graph=proc.snapshot();}else status="Circuit updated";rev=proc.revision;repaint();}
 void add(std::string type,double val){auto candidate=graph;int n=1;std::string id;do{id="P"+std::to_string(n++);}while(std::any_of(graph.parts.begin(),graph.parts.end(),[&](auto& p){return p.id==id;}));graph.parts.push_back({id,type,val,450,250});apply();}
 void select(int i){selected=i;if(i>=0)value.setText(String(graph.parts[(size_t)i].value,9));repaint();}
public:
 BreadEditor(BreadProcessor& p):AudioProcessorEditor(p),proc(p),graph(p.snapshot()){
 setSize(1160,680);setWantsKeyboardFocus(true);
 for(auto* b:{&preset,&load,&save,&addR,&addC,&addQ,&addD,&remove})addAndMakeVisible(b);
 addAndMakeVisible(value);value.setInputRestrictions(24,"0123456789.eE+-");value.setTooltip("SI value: ohms, farads, NPN beta, or diode saturation current in amps. Press Enter.");
 value.onReturnKey=[this]{if(selected>=0){graph.parts[(size_t)selected].value=value.getText().getDoubleValue();apply();}};
 preset.onClick=[this]{graph=bread::fuzz();selected=-1;first.clear();apply();};
 addR.onClick=[this]{add("resistor",10000);};addC.onClick=[this]{add("capacitor",1e-7);};addQ.onClick=[this]{add("npn",180);};addD.onClick=[this]{add("diode",1e-12);};
 remove.onClick=[this]{if(selected<0)return;auto id=graph.parts[(size_t)selected].id+".";graph.parts.erase(graph.parts.begin()+selected);graph.wires.erase(std::remove_if(graph.wires.begin(),graph.wires.end(),[&](auto& w){return w.a.rfind(id,0)==0||w.b.rfind(id,0)==0;}),graph.wires.end());selected=-1;first.clear();apply();};
 load.onClick=[this]{chooser=std::make_unique<FileChooser>("Open circuit",File{},"*.json");chooser->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles,[safe=Component::SafePointer<BreadEditor>(this)](const FileChooser& fc){if(!safe)return;auto file=fc.getResult();if(!file.existsAsFile())return;try{safe->graph=parseGraph(file.loadFileAsString());safe->selected=-1;safe->first.clear();safe->apply();}catch(const std::exception& e){safe->status=e.what();safe->repaint();}});};
 save.onClick=[this]{chooser=std::make_unique<FileChooser>("Save circuit",File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("bread-circuit.json"),"*.json");chooser->launchAsync(FileBrowserComponent::saveMode|FileBrowserComponent::canSelectFiles|FileBrowserComponent::warnAboutOverwriting,[safe=Component::SafePointer<BreadEditor>(this)](const FileChooser& fc){if(!safe||fc.getResult()==File{})return;safe->status=fc.getResult().replaceWithText(graphJSON(safe->graph))?"Saved":"Could not save file";safe->repaint();});};
 auto setup=[this](Slider& s,Label& label,const String& text){addAndMakeVisible(s);addAndMakeVisible(label);label.setText(text,dontSendNotification);s.setSliderStyle(Slider::LinearHorizontal);s.setTextBoxStyle(Slider::TextBoxRight,false,55,20);};setup(drive,driveLabel,"INPUT");setup(level,levelLabel,"OUTPUT");setup(mix,mixLabel,"MIX");
 da=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.params,"drive",drive);la=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.params,"level",level);ma=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.params,"mix",mix);
 startTimerHz(20);
 }
 void resized()override{preset.setBounds(240,20,120,32);load.setBounds(370,20,110,32);save.setBounds(490,20,110,32);addR.setBounds(20,75,120,30);addC.setBounds(150,75,130,30);addQ.setBounds(290,75,110,30);addD.setBounds(410,75,110,30);value.setBounds(750,75,170,30);remove.setBounds(930,75,100,30);driveLabel.setBounds(30,580,70,24);drive.setBounds(90,580,235,30);levelLabel.setBounds(365,580,75,24);level.setBounds(435,580,235,30);mixLabel.setBounds(720,580,60,24);mix.setBounds(770,580,235,30);}
 void paint(Graphics& g)override{
 g.fillAll(Colour(0xff191d22));g.setColour(Colour(0xffefb84c));g.setFont(30);g.drawText("BREAD",20,15,200,40,Justification::centredLeft);
 g.setFont(14);g.setColour(Colours::white);g.drawText(String(graph.name),650,20,460,30,Justification::centredRight);
 g.setColour(Colour(0xffdedbd0));g.fillRoundedRectangle(100,130,970,425,12);g.setColour(Colour(0xffaaa89e));for(int x=120;x<1060;x+=18)for(int y=148;y<540;y+=18)g.fillEllipse((float)x,(float)y,3,3);
 auto pts=points();int index=0;for(auto& w:graph.wires){auto a=pts[w.a];auto b=w.b.empty()?Point<float>(w.x,w.y):pts[w.b];Path path;path.startNewSubPath(a);path.cubicTo(a.x,a.y+75,b.x,b.y+75,b.x,b.y);g.setColour(Colour::fromHSV((index++%7)/7.0f,.65f,.75f,1));g.strokePath(path,PathStrokeType(3));}
 for(size_t i=0;i<graph.parts.size();++i){auto& p=graph.parts[i];g.setColour((int)i==selected?Colour(0xffefb84c):Colour(0xff2d363d));g.fillRoundedRectangle(p.x-40,p.y+109,80,42,7);g.setColour((int)i==selected?Colours::black:Colours::white);g.setFont(14);g.drawText(String(p.id),int(p.x)-38,int(p.y)+110,76,20,Justification::centred);g.setFont(12);g.drawText(String(p.value,5),int(p.x)-38,int(p.y)+131,76,18,Justification::centred);}
 for(auto& p:pts){g.setColour(first==String(p.first)?Colours::orange:Colour(0xff455965));g.fillEllipse(p.second.x-5,p.second.y-5,10,10);g.setColour(Colour(0xff657583));g.setFont(12);g.drawText(String(p.first),int(p.second.x)-35,int(p.second.y)-24,70,18,Justification::centred);}
 g.setColour(Colours::white);g.setFont(14);g.drawText(status,20,625,1100,25,Justification::centredLeft);g.drawText("Peak "+String(proc.meter.load(),2)+"   Solver misses "+String((int)proc.failures.load()),20,652,1100,20,Justification::centredLeft);
 }
 void mouseDown(const MouseEvent& e)override{auto pts=points();for(auto& p:pts)if(e.position.getDistanceFrom(p.second)<12){selected=-1;
  if(e.mods.isPopupMenu()){for(auto& w:graph.wires)if(w.a==p.first||w.b==p.first){if(w.a==p.first)std::swap(w.a,w.b);w.b.clear();w.x=p.second.x+15;w.y=p.second.y+80;apply();return;}}
  else if(first.isEmpty()){first=p.first;status="Connect "+first+" to another pin";}else{graph.wires.push_back({first.toStdString(),p.first});first.clear();apply();}repaint();return;}
 for(size_t i=0;i<graph.parts.size();++i){auto& p=graph.parts[i];if(Rectangle<float>(p.x-40,p.y+109,80,42).contains(e.position)){select((int)i);dragOffset=e.position-Point<float>(p.x,p.y);return;}}first.clear();select(-1);}
 void mouseDrag(const MouseEvent& e)override{if(selected>=0){auto& p=graph.parts[(size_t)selected];auto pos=e.position-dragOffset;p.x=std::clamp(pos.x,150.0f,1010.0f);p.y=std::clamp(pos.y,35.0f,390.0f);repaint();}}
 void mouseUp(const MouseEvent& e)override{if(selected>=0&&e.mouseWasDraggedSinceMouseDown())apply();}
 void timerCallback()override{if(rev!=proc.revision){graph=proc.snapshot();rev=proc.revision;selected=-1;first.clear();}repaint();}
};
AudioProcessorEditor* BreadProcessor::createEditor(){return new BreadEditor(*this);}
