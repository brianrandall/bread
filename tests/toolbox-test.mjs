import assert from 'node:assert/strict';
import {compile,Solver,diodeModels} from '../dist/engine.js';
import {presets} from '../dist/presets.js';
for(const preset of presets){const g=preset.build(),s=new Solver(compile(g));for(let i=0;i<4800;i++)assert.ok(Number.isFinite(s.step(.5*Math.sin(i*.03))));assert.equal(s.failures,0,preset.name);assert.deepEqual(JSON.parse(JSON.stringify(g)),g);}
const volume=presets.find(p=>p.id==='volume').build();
for(const position of [0,.25,.5,.75,1]){volume.components[0].position=position;const s=new Solver(compile(volume));assert.ok(Math.abs(s.step(1)-position)<.0001);}
volume.components[0].position=.5;volume.components[0].taper='log';assert.ok(Math.abs(new Solver(compile(volume)).step(1)-1/11)<.0001);
const switchGraph={version:1,components:[{id:'S1',type:'switch',value:0},{id:'R1',type:'resistor',value:10000}],connections:[{a:'input',b:'S1.1'},{a:'S1.2',b:'output'},{a:'output',b:'R1.1'},{a:'R1.2',b:'ground'}]};
assert.equal(new Solver(compile(switchGraph)).step(1),0);switchGraph.components[0].value=1;assert.ok(new Solver(compile(switchGraph)).step(1)>.999);
const values={};for(const model of Object.keys(diodeModels)){const g=presets.find(p=>p.id==='clipper').build();g.components.find(p=>p.id==='D1').model=model;g.components.find(p=>p.id==='D2').model=model;values[model]=new Solver(compile(g)).step(4);}
assert.ok(values.schottky<values.germanium&&values.germanium<values.silicon&&values.silicon<values.led);console.log('Toolbox PASS: presets, pot endpoints/taper, switch isolation, diode ordering',values);
