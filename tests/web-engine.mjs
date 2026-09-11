import assert from 'node:assert/strict';
import {compile,Solver,fuzz,importSpice} from '../dist/engine.js';
const divider=importSpice('* Voltage divider\nR1 in out 10k\nR2 out 0 10k\n.end');
let s=new Solver(compile(divider));assert.ok(Math.abs(s.step(1)-.5)<1e-4);
const rc=importSpice('R1 in out 10k\nC1 out 0 1u');s=new Solver(compile(rc));let y=0;for(let i=0;i<480;i++)y=s.step(1);assert.ok(Math.abs(y-(1-Math.exp(-1)))<.002);
const net=compile(fuzz());s=new Solver(net);console.log('Bias:',s.v[net.nodes['Q1.c']],s.v[net.nodes['Q2.c']]);const t=performance.now();let energy=0;for(let i=0;i<48000;i++){const out=s.step(.06*Math.sin(i*2*Math.PI*110/48000));assert.ok(Number.isFinite(out));energy+=out*out;}assert.equal(s.failures,0);assert.ok(energy>1);console.log('48k samples:',((performance.now()-t)/1000).toFixed(3),'s; RMS',Math.sqrt(energy/48000));
const bad=fuzz();bad.connections.push({a:'battery',b:'ground'});assert.throws(()=>compile(bad),/short/);
const dangling=fuzz();dangling.connections[0].b='';s=new Solver(compile(dangling));const disconnectedReference=new Solver(compile(dangling));for(let i=0;i<2000;i++)assert.equal(s.step(Math.sin(i*.1)),disconnectedReference.step(0));
const ff=importSpice('* FF\n.model 2N5088 npn(bf=200)\nC1 in Q1b 2.2u\nR1 vcc Q1c 33k\nR2 Q1b Q2e 100k\nQ1 Q1c Q1b 0 2N5088\nQ2 Q2c Q1c Q2e 2N5088\nR3 vcc Q2c 8.2k\nR4 Q2e 0 1k\nC2 Q2e 0 20u\nC3 Q2c pv2 10n\nP1 0 pv2 out 100k position=0.6 taper=log\n.end\n$ volume pot wiper at 60%');
assert.equal(ff.components.length,10);assert.equal(ff.components.find(p=>p.type==='npn').value,200);const fp=ff.components.find(p=>p.id==='P1');assert.equal(fp.position,.6);assert.equal(fp.taper,'log');const ffn=compile(ff);let fe=0,fs=new Solver(ffn);for(let i=0;i<48000;i++)fe+=fs.step(.2*Math.sin(i*2*Math.PI*110/48000))**2;assert.equal(fs.failures,0);assert.ok(fe>.05,'fuzz-face import should pass signal');
assert.equal(importSpice('D1 out 0 1N4148').components[0].model,'silicon');assert.equal(importSpice('D1 out 0 1N34A').components[0].model,'germanium');assert.equal(importSpice('D1 out 0 BAT41').components[0].model,'schottky');assert.equal(importSpice('D1 out 0 LED').components[0].model,'led');
const diodeModel=importSpice('.model CUST D(Is=1e-8)\nD1 out 0 CUST');assert.equal(diodeModel.components[0].value,1e-8);
const customBeta=importSpice('.model FF3 NPN(BF=120)\nQ1 out in 0 FF3');assert.equal(customBeta.components[0].value,120);
const bareQ=importSpice('Q1 out in 0');assert.equal(bareQ.components[0].value,180);
assert.throws(()=>importSpice('L1 in out 1m'),/support/);assert.throws(()=>importSpice('.subckt X\nR1 a b 1k\n.ends X'),/Flatten the netlist/);
const netlistWithDirectives=importSpice('V1 vcc 0 9\n.tran 5m\nR1 in out 10k\nR2 out 0 10k');assert.equal(netlistWithDirectives.components.length,2);
assert.equal(importSpice('C1 in out 2.2uF').components[0].value,2.2e-6,"value unit suffix F is tolerated");
assert.equal(importSpice('R1 in out 0.001').components[0].value,1,"sub-ohm wiper resistor clamps to the 1 ohm floor");
const numericNets=importSpice('* Nodes: 0=Gnd, 1=In, 2=Out, 10=VCC (+9V)\nR1 10 1 470\nR2 1 3 100k\nR3 3 2 100k\nR4 3 0 100k');
assert.ok(numericNets.connections.some(w=>w.a==='battery'||w.b==='battery'));assert.ok(numericNets.connections.some(w=>w.a==='input'||w.b==='input'));assert.ok(numericNets.connections.some(w=>w.a==='output'||w.b==='output'));
const userFuzz=importSpice(`* Classic NPN Silicon Fuzz Face Netlist
* Nodes: 0=Gnd, 1=In, 2=Out, 10=VCC (+9V)
V1 10 0 DC 9V
C1 1 3 2.2uF
Q1 4 3 0 Q_NPN_1
R1 4 3 100k
Q2 5 4 6 Q_NPN_2
R2 10 7 470
R3 7 5 8.2k
RFuzz 6 0 1k
C2 6 0 20uF
C3 7 8 0.01uF
RVol 8 2 0.001
RLoad 2 0 500k
.model Q_NPN_1 NPN(BF=90 IS=1E-14)
.model Q_NPN_2 NPN(BF=120 IS=1E-14)
.trans 10u 20ms
.end`);
assert.equal(userFuzz.components.length,11);assert.equal(userFuzz.components.find(p=>p.id==='Q1').value,90);assert.equal(userFuzz.components.find(p=>p.id==='Q2').value,120);const ufn=compile(userFuzz);let ue=0,us=new Solver(ufn);for(let i=0;i<48000;i++)ue+=us.step(.2*Math.sin(i*2*Math.PI*110/48000))**2;assert.equal(us.failures,0,'user Fuzz Face netlist solves clean');assert.ok(ue<2e1,'unbiased NPN flip should not produce fuzz-level energy');assert.equal(us.failures,0,'muted flip still solves clean');
const biasedFuzz=importSpice('* biased\n* Nodes: 0=Gnd, 1=In, 2=Out, 10=VCC\nV1 10 0 DC 9\nC1 1 3 2.2uF\nR1c 10 4 33k\nQ1 4 3 0 M1\nR1 4 3 100k\nRb 3 6 100k\nQ2 5 4 6 M2\nR3 7 5 8.2k\nR2a 10 7 470\nRFuzz 6 0 1k\nC2 6 0 20uF\nC3 7 8 0.01uF\nRVol 8 2 0.001\nRLoad 2 0 500k\n.model M1 NPN(BF=90)\n.model M2 NPN(BF=120)');const bfn=compile(biasedFuzz);const bs=new Solver(bfn);for(let i=0;i<1024;i++)bs.step(0);assert.ok(bs.v[bfn.nodes['Q2.e']]>0.5,'Q2 emitter should sit above ground');assert.ok(bs.v[bfn.nodes['Q2.c']]<8.5,'Q2 collector should pull off the rail');let be=0;for(let i=0;i<48000;i++)be+=bs.step(.2*Math.sin(i*2*Math.PI*110/48000))**2;assert.ok(be>0.5,'biased flip is audible');assert.ok(be>500*ue,'adding the NPN bias path must dominate the muted flip');
assert.throws(()=>compile({...fuzz(),components:[{id:'R1',type:'resistor',value:NaN}]}));
const batterySolver=new Solver(net);batterySolver.battery=4.5;const restored=Solver.restore(batterySolver.export());assert.equal(restored.battery,4.5);assert.ok(Number.isFinite(restored.step(.02)));
console.log('Web engine PASS');
