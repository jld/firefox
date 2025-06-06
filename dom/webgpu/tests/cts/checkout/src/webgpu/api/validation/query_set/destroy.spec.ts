export const description = `
Destroying a query set more than once is allowed.
`;

import { makeTestGroup } from '../../../../common/framework/test_group.js';
import { AllFeaturesMaxLimitsGPUTest } from '../../../gpu_test.js';

export const g = makeTestGroup(AllFeaturesMaxLimitsGPUTest);

g.test('twice').fn(t => {
  const qset = t.createQuerySetTracked({ type: 'occlusion', count: 1 });

  qset.destroy();
  qset.destroy();
});

g.test('invalid_queryset')
  .desc('Test that invalid querysets may be destroyed without generating validation errors.')
  .fn(async t => {
    t.device.pushErrorScope('validation');

    const invalidQuerySet = t.createQuerySetTracked({
      type: 'occlusion',
      count: 4097, // 4096 is the limit
    });

    // Expect error because it's invalid.
    const error = await t.device.popErrorScope();
    t.expect(!!error);

    // This line should not generate an error
    invalidQuerySet.destroy();
  });
