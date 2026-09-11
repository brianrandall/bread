import assert from 'node:assert/strict';
import {Solver,compile,fuzz} from '../dist/engine.js';
globalThis.sampleRate=48000;
let Worklet;
globalThis.AudioWorkletProcessor=class{constructor(){this.port={postMessage(){}};}};
globalThis.registerProcessor=(name,type)=>{Worklet=type;};
await import('../dist/worklet.js');
const processor=new Worklet();processor.port.onmessage({data:{state:new Solver(compile(fuzz())).export(),request:1,controls:{demo:true}}});
let energy=0;for(let block=0;block<375;block++){const left=new Float32Array(128),right=new Float32Array(128);assert.equal(processor.process([[]],[[left,right]]),true);for(let i=0;i<128;i++){assert.ok(Number.isFinite(left[i]));assert.ok(Math.abs(left[i])<=1);assert.equal(left[i],right[i]);energy+=left[i]**2;}}
assert.ok(energy>1);assert.equal(processor.s.failures,0);
processor.port.onmessage({data:{state:new Solver(compile(fuzz())).export(),request:2,controls:{demo:false,mix:0}}});const input=new Float32Array(128).fill(.25),output=new Float32Array(128);processor.process([[input]],[[output]]);assert.equal(output[100],.25);
processor.port.onmessage({data:{controls:{battery:4.5}}});for(let block=0;block<50;block++)processor.process([[]],[[new Float32Array(128)]]);assert.ok(Math.abs(processor.battery-4.5)<.01);assert.equal(processor.s.battery,processor.battery);
console.log('AudioWorklet processing PASS; bounded demo audio, stereo duplication, graph change, dry bypass, battery starvation');
let sample=0;
function wetEnergy(battery){processor.port.onmessage({data:{controls:{battery,mix:1,demo:false}}});let sum=0;for(let b=0;b<750;b++){const input=Float32Array.from({length:128},()=>{const i=sample++;return .0875*(Math.sin(i*2*Math.PI*110/48000)+Math.sin(i*2*Math.PI*220/48000));}),out=new Float32Array(128);processor.process([[input]],[[out]]);if(b>375)for(const x of out){assert.ok(Number.isFinite(x));sum+=x*x;}}return sum;}
const fresh=wetEnergy(9),starved=wetEnergy(1),recovered=wetEnergy(9);assert.ok(fresh>1&&recovered>fresh*.8);assert.ok(starved<fresh*.001);console.log('Wet dual-tone supply sweep PASS: 9 V audible, 1 V starved, 9 V recovered.');
