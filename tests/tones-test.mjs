import assert from 'node:assert/strict';
import {TestToneSource, toneDefaults, sanitiseTone, pitchLabel} from '../dist/tones.js';
class Param {
  value=0;
  setTargetAtTime(value,time,tau){assert.ok(Number.isFinite(value));assert.ok(tau>0);this.target=value;}
}
class Node {
  constructor(){this.gain=new Param();this.frequency=new Param();this.Q=new Param();this.targets=[];this.stops=0;}
  connect(target){this.targets.push(target);}
  disconnect(){this.targets=[];}
  start(){this.started=true;}
  stop(){this.stops++;}
}
const context={sampleRate:48000,currentTime:1,
 createGain:()=>new Node(),createOscillator:()=>new Node(),createBufferSource:()=>new Node(),createBiquadFilter:()=>new Node(),
 createBuffer:(channels,size)=>{const samples=new Float32Array(size);return {getChannelData:()=>samples};}
};
assert.equal(pitchLabel(110),'A2');assert.equal(pitchLabel(440),'A4');
assert.equal(sanitiseTone({frequency:Infinity}).frequency,110);
assert.equal(sanitiseTone({frequency:0}).frequency,20);
assert.equal(sanitiseTone({frequency:20000,volume:2}).frequency,12000);
const source=new TestToneSource(context),circuit=new Node();source.connect(circuit);
assert.deepEqual(source.output.targets,[circuit]);
assert.ok(source.voices.every(v=>v.volume.targets[0]===source.output));
assert.notEqual(source.voices[0].waves.noise.oscillator.buffer,source.voices[1].waves.noise.oscillator.buffer);
assert.ok(source.sources.every(s=>s.started));
source.update([{wave:'square',frequency:111,volume:.8,enabled:true},{wave:'triangle',frequency:333,volume:.2,enabled:true}]);
assert.equal(source.voices[0].waves.square.gain.gain.target,1);
assert.equal(source.voices[0].waves.sine.gain.gain.target,0);
assert.equal(source.voices[0].waves.square.oscillator.frequency.target,111);
assert.equal(source.voices[1].waves.triangle.oscillator.frequency.target,333);
assert.equal(source.voices[0].volume.gain.target,.2);assert.equal(source.voices[1].volume.gain.target,.05);
source.update([{wave:'noise',frequency:1500,volume:.8,enabled:false},toneDefaults[1]]);
assert.equal(source.voices[0].filter.frequency.target,1500);assert.equal(source.voices[0].volume.gain.target,0);
assert.equal(source.voices[1].volume.gain.target,.0875);
source.stop();source.disconnect();assert.ok(source.sources.every(s=>s.stops===1));assert.ok(source.nodes.every(n=>n.targets.length===0));
console.log('Test oscillator controls and source lifecycle PASS (mock Web Audio context; not a browser listening test)');
