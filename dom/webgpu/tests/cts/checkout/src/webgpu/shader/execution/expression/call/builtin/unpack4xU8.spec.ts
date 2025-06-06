export const description = `
Execution tests for the 'unpack4xU8' builtin function

@const fn unpack4xU8(e: u32) -> vec4<u32>
e is interpreted as a vector with four 8-bit unsigned integer components. Unpack e into a vec4<u32>
with zero extension.
`;

import { makeTestGroup } from '../../../../../../common/framework/test_group.js';
import { AllFeaturesMaxLimitsGPUTest } from '../../../../../gpu_test.js';
import { u32, toVector, Type } from '../../../../../util/conversion.js';
import { Case } from '../../case.js';
import { allInputSources, Config, run } from '../../expression.js';

import { builtin } from './builtin.js';

export const g = makeTestGroup(AllFeaturesMaxLimitsGPUTest);

g.test('basic')
  .specURL('https://www.w3.org/TR/WGSL/#unpack4xU8-builtin')
  .desc(
    `
@const fn unpack4xU8(e: u32) -> vec4<u32>
  `
  )
  .params(u => u.combine('inputSource', allInputSources))
  .fn(async t => {
    const cfg: Config = t.params;

    const unpack4xU8 = (e: number) => {
      const result: [number, number, number, number] = [0, 0, 0, 0];
      for (let i = 0; i < 4; ++i) {
        result[i] = (e >> (8 * i)) & 0xff;
      }
      return result;
    };

    const testInputs = [0, 0x08060402, 0xffffffff, 0xfefdfcfb] as const;

    const makeCase = (e: number): Case => {
      return { input: [u32(e)], expected: toVector(unpack4xU8(e), u32) };
    };
    const cases: Array<Case> = testInputs.flatMap(v => {
      return [makeCase(v)];
    });

    await run(t, builtin('unpack4xU8'), [Type.u32], Type.vec4u, cfg, cases);
  });
