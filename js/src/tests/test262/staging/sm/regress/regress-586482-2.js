/*
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/licenses/publicdomain/
 */

/*---
includes: [sm/non262.js, sm/non262-shell.js]
flags:
  - noStrict
description: |
  pending
esid: pending
---*/
var expect = true;
var actual;

var checkCaller = function(me) {
    var caller = arguments.callee.caller;
    var callerIsMethod = (caller === me['doThing']);
    actual = callerIsMethod;
};

Object.prototype.doThing = function() {
    checkCaller(this);
};

["dense"].doThing();

assert.sameValue(expect, actual, "ok");

reportCompare(0, 0);
