#pragma once
#include <JuceHeader.h>
#include "Circuit.h"

juce::String graphJSON(const bread::Graph&);
bread::Graph parseGraph(const juce::String&);
class BreadProcessor : public juce::AudioProcessor {
public:
 BreadProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 const juce::String getName() const override{return "BREAD";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;}
 double getTailLengthSeconds() const override{return 0;}
 int getNumPrograms() override{return 1;}int getCurrentProgram() override{return 0;}
 void setCurrentProgram(int) override{} const juce::String getProgramName(int) override{return "Circuit";}
 void changeProgramName(int,const juce::String&) override{}
 bool hasEditor() const override{return true;}juce::AudioProcessorEditor* createEditor() override;
 void getStateInformation(juce::MemoryBlock&) override;void setStateInformation(const void*,int) override;
 juce::String submit(const bread::Graph&);
 bread::Graph snapshot();
 juce::AudioProcessorValueTreeState params;
 std::atomic<float> meter{0};std::atomic<unsigned> failures{0};std::atomic<int> revision{0};
private:
 juce::CriticalSection graphLock;bread::Graph graph;
 juce::SpinLock mailbox;std::array<bread::Solver,2> pending{},active{},old{};bool ready=false;
 std::atomic<double> rate{48000};int fade=0;
 double dcX[2]{},dcY[2]{};
};
