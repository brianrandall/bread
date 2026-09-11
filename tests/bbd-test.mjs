import assert from 'node:assert/strict';
import {compile,Solver} from '../dist/engine.js';
const resolver = (components, pairs) => ({ version: 1, components, connections: pairs.map(([a, b]) => ({ a, b })) });
const part = (id, type, value, extra = {}) => ({ id, type, value, x: 200, y: 200, ...extra });

function delayLine(stages, clock, load = 220000) {
 const g = resolver(
  [part('R1', 'resistor', 10000), part('U1', 'bbd', stages, { clock }), part('R2', 'resistor', load)],
  [['input', 'R1.1'], ['R1.2', 'U1.1'], ['U1.2', 'output'], ['output', 'R2.1'], ['R2.2', 'ground']]);
 return new Solver(compile(g));
}

const s = delayLine(4096, 12000);
const impulse = 20000, expect = Math.round(4096 / 12000 * 48000), found = [];
for (let i = 0; i < 50000; i++) { const y = s.step(i === impulse ? 1 : 0); if (Math.abs(y) > 0.1) { found.push(i); break; } }
assert.ok(Math.abs((found[0] ?? 0) - (impulse + expect)) <= 1, `BBD echo off by ${(found[0] ?? 0) - (impulse + expect)} samples`);
assert.equal(s.failures, 0);
for (let i = 0; i < 40000; i++) s.step(0);
assert.ok(Math.abs(s.step(0)) < 1e-6, 'delay line idles at zero');

const echo = resolver([
 part('C1', 'capacitor', 2.2e-7), part('R1', 'resistor', 68000),
 part('C2', 'capacitor', 4.7e-9), part('U1', 'bbd', 4096, { clock: 10500 }),
 part('C3', 'capacitor', 2.2e-7), part('Rfb', 'resistor', 5600),
 part('S1', 'switch', 1), part('Rdry', 'resistor', 22000),
 part('Rwet', 'resistor', 1000), part('Rload', 'resistor', 470000),
 part('Cx', 'capacitor', 4.7e-7), part('Qg', 'npn', 200),
 part('Rg1', 'resistor', 220000), part('Rg2', 'resistor', 47000),
 part('RgE', 'resistor', 1000), part('CE', 'capacitor', 1e-5),
 part('RgC', 'resistor', 10000), part('Cg', 'capacitor', 1e-7),
 part('Rout', 'resistor', 100000)],
 [
  ['input', 'C1.1'], ['C1.2', 'R1.1'], ['R1.2', 'U1.1'], ['R1.2', 'C2.1'],
  ['C2.2', 'ground'], ['U1.2', 'C3.1'], ['C3.2', 'Rwet.1'], ['C3.2', 'Rfb.1'],
  ['Rfb.2', 'S1.1'], ['S1.2', 'R1.2'], ['C1.2', 'Rdry.1'], ['Rdry.2', 'Rwet.2'],
  ['Rwet.2', 'Rload.1'], ['Rload.2', 'ground'], ['Rwet.2', 'Cx.1'],
  ['Cx.2', 'Qg.b'], ['battery', 'Rg1.1'], ['Rg1.2', 'Qg.b'], ['Qg.b', 'Rg2.1'], ['Rg2.2', 'ground'],
  ['Qg.e', 'RgE.1'], ['RgE.2', 'ground'], ['Qg.e', 'CE.1'], ['CE.2', 'ground'],
  ['battery', 'RgC.1'], ['RgC.2', 'Qg.c'], ['Qg.c', 'Cg.1'], ['Cg.2', 'output'],
  ['output', 'Rout.1'], ['Rout.2', 'ground']]);
function tailProfile(g, toneLen, toneAmp) {
 const s = new Solver(compile(g));
 let sustain = 0;
 for (let i = 0; i < toneLen; i++) { const y = s.step(toneAmp * Math.sin(2 * Math.PI * 220 * i / 48000)); sustain = Math.max(sustain, Math.abs(y)); }
 const period = Math.round(4096 / 10500 * 48000);
 const e = [], peakTail = [];
 for (let k = 0; k < 6; k++) {
  let acc = 0, pk = 0;
  for (let n = 0; n < period; n++) { const y = s.step(0); acc += y * y; pk = Math.max(pk, Math.abs(y)); }
  e.push(acc); peakTail.push(pk);
 }
 return { sustain, e, peakTail, failures: s.failures };
}
const p = tailProfile(echo, 20000, 0.2);
const echoAsLoudUnit = p.sustain > 0 && p.peakTail[0] / p.sustain > 0.35;
assert.ok(echoAsLoudUnit, `first repeat near dry level: ${p.peakTail[0].toFixed(3)} vs dry peak ${p.sustain.toFixed(3)}`);
assert.ok(p.e[4] > p.e[5] && p.e[2] > p.e[3], 'repeat energy decays through the tail');
assert.ok(p.e[0] > p.e[3] * 1.3, `repeats persist past the first drain: ${p.e.map(x => x.toFixed(3)).join(',')}`);
assert.ok(p.e[5] < p.e[0], `tail does not grow: ${p.e[0].toFixed(3)} -> ${p.e[5].toFixed(3)}`);
assert.equal(p.failures, 0);

const slap = resolver([
 part('C1', 'capacitor', 2.2e-7), part('R1', 'resistor', 68000),
 part('C2', 'capacitor', 1.5e-9), part('U1', 'bbd', 1024, { clock: 11500 }),
 part('C3', 'capacitor', 2.2e-7), part('Rfb', 'resistor', 27000),
 part('S1', 'switch', 0), part('Rdry', 'resistor', 27000),
 part('Rwet', 'resistor', 3300), part('Rload', 'resistor', 470000),
 part('Cx', 'capacitor', 4.7e-7), part('Qg', 'npn', 200),
 part('Rg1', 'resistor', 220000), part('Rg2', 'resistor', 47000),
 part('RgE', 'resistor', 1000), part('CE', 'capacitor', 1e-5),
 part('RgC', 'resistor', 10000), part('Cg', 'capacitor', 1e-7),
 part('Rout', 'resistor', 100000)],
 [
  ['input', 'C1.1'], ['C1.2', 'R1.1'], ['R1.2', 'U1.1'], ['R1.2', 'C2.1'],
  ['C2.2', 'ground'], ['U1.2', 'C3.1'], ['C3.2', 'Rwet.1'], ['C3.2', 'Rfb.1'],
  ['Rfb.2', 'S1.1'], ['S1.2', 'R1.2'], ['C1.2', 'Rdry.1'], ['Rdry.2', 'Rwet.2'],
  ['Rwet.2', 'Rload.1'], ['Rload.2', 'ground'], ['Rwet.2', 'Cx.1'],
  ['Cx.2', 'Qg.b'], ['battery', 'Rg1.1'], ['Rg1.2', 'Qg.b'], ['Qg.b', 'Rg2.1'], ['Rg2.2', 'ground'],
  ['Qg.e', 'RgE.1'], ['RgE.2', 'ground'], ['Qg.e', 'CE.1'], ['CE.2', 'ground'],
  ['battery', 'RgC.1'], ['RgC.2', 'Qg.c'], ['Qg.c', 'Cg.1'], ['Cg.2', 'output'],
  ['output', 'Rout.1'], ['Rout.2', 'ground']]);
const s3 = new Solver(compile(slap));
let sustain3 = 0;
for (let i = 0; i < 20000; i++) { const y = s3.step(0.2 * Math.sin(2 * Math.PI * 220 * i / 48000)); sustain3 = Math.max(sustain3, Math.abs(y)); }
let slapTail = 0;
for (let n = 0; n < 12000; n++) slapTail = Math.max(slapTail, Math.abs(s3.step(0)));
assert.ok(sustain3 > 0 && slapTail > sustain3 * 0.5, `slapback repeat as loud as dry: ${slapTail.toFixed(3)} vs ${sustain3.toFixed(3)}`);
assert.equal(s3.failures, 0);

assert.throws(() => compile(resolver([part('U1', 'bbd', 32, { clock: 12000 })], [])), /stage count/);
assert.throws(() => compile(resolver([part('U1', 'bbd', 1024.5, { clock: 12000 })], [])), /stage count/);
assert.throws(() => compile(resolver([part('U1', 'bbd', 1024, { clock: 100 })], [])), /clock/);
console.log('BBD PASS: echo timing, feedback repeats, mix, idle zero, stage/clock validation');