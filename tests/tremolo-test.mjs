import assert from 'node:assert/strict';
import {compile,Solver} from '../dist/engine.js';
import {presets} from '../dist/presets.js';
const preset=presets.find(p=>p.id==='tremolo');
for(const rate of [44100,48000]){
 for(const wave of ['sine','square','triangle']){
  const graph=preset.build();Object.assign(graph.components[0],{wave,depth:1});
  const s=new Solver(compile(graph),rate,false);let min=Infinity,max=-Infinity,crossings=0,last=0;
  for(let i=0;i<rate;i++){const y=s.step(.1);assert.ok(Number.isFinite(y));if(i>rate/10){min=Math.min(min,y);max=Math.max(max,y);}if(y>.25&&last<=.25)crossings++;last=y;}
  assert.equal(s.failures,0);assert.ok(min<.005, wave+' trough');assert.ok(max>.49,wave+' peak');assert.ok(crossings===4||crossings===5);assert.ok(Math.abs(s.net.parts[0].phase)<1e-8||Math.abs(s.net.parts[0].phase-1)<1e-8);
 }
}
const graph=preset.build();graph.components[0].depth=0;
let s=new Solver(compile(graph));for(let i=0;i<4800;i++)assert.ok(Math.abs(s.step(.1)-.4995)<.001);
graph.connections=graph.connections.filter(w=>w.b!=='VCA1.cv');s=new Solver(compile(graph));assert.ok(Math.abs(s.step(.1))<1e-8,'unplugged CV closes VCA');
const original=new Solver(compile(preset.build()));for(let i=0;i<100;i++)original.step(.03);
const restored=Solver.restore(structuredClone(original.export()));for(let i=0;i<100;i++)assert.equal(original.step(.03),restored.step(.03));
globalThis.sampleRate=48000;
globalThis.AudioWorkletProcessor=class{constructor(){this.port={postMessage(){}};}};
let Processor;globalThis.registerProcessor=(name,p)=>{Processor=p;};await import('../dist/worklet.js');
const processor=new Processor();processor.port.onmessage({data:{state:new Solver(compile(preset.build())).export(),controls:{mix:0}}});
const input=Float32Array.from({length:128},(_,i)=>.1*Math.sin(i*.1)),output=new Float32Array(128);
processor.process([[input]],[[output]]);assert.deepEqual(output,input,'dry bypass is unchanged');
const phase=processor.s.net.parts.find(p=>p.kind==='lfo').phase;
processor.port.onmessage({data:{state:new Solver(compile(preset.build())).export()}});
assert.equal(processor.s.net.parts.find(p=>p.kind==='lfo').phase,phase,'edits preserve LFO phase');
for(const p of presets)assert.doesNotThrow(()=>compile(p.build()),p.name);
console.log('LFO tremolo PASS: waveforms, depth, rate, patching, persistence, dry path and phase continuity.');
