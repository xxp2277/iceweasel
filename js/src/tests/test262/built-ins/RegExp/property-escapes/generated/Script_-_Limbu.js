// |reftest| skip -- regexp-unicode-property-escapes is not supported
// Copyright 2020 Mathias Bynens. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
author: Mathias Bynens
description: >
  Unicode property escapes for `Script=Limbu`
info: |
  Generated by https://github.com/mathiasbynens/unicode-property-escapes-tests
  Unicode v13.0.0
esid: sec-static-semantics-unicodematchproperty-p
features: [regexp-unicode-property-escapes]
includes: [regExpUtils.js]
---*/

const matchSymbols = buildString({
  loneCodePoints: [
    0x001940
  ],
  ranges: [
    [0x001900, 0x00191E],
    [0x001920, 0x00192B],
    [0x001930, 0x00193B],
    [0x001944, 0x00194F]
  ]
});
testPropertyEscapes(
  /^\p{Script=Limbu}+$/u,
  matchSymbols,
  "\\p{Script=Limbu}"
);
testPropertyEscapes(
  /^\p{Script=Limb}+$/u,
  matchSymbols,
  "\\p{Script=Limb}"
);
testPropertyEscapes(
  /^\p{sc=Limbu}+$/u,
  matchSymbols,
  "\\p{sc=Limbu}"
);
testPropertyEscapes(
  /^\p{sc=Limb}+$/u,
  matchSymbols,
  "\\p{sc=Limb}"
);

const nonMatchSymbols = buildString({
  loneCodePoints: [
    0x00191F
  ],
  ranges: [
    [0x00DC00, 0x00DFFF],
    [0x000000, 0x0018FF],
    [0x00192C, 0x00192F],
    [0x00193C, 0x00193F],
    [0x001941, 0x001943],
    [0x001950, 0x00DBFF],
    [0x00E000, 0x10FFFF]
  ]
});
testPropertyEscapes(
  /^\P{Script=Limbu}+$/u,
  nonMatchSymbols,
  "\\P{Script=Limbu}"
);
testPropertyEscapes(
  /^\P{Script=Limb}+$/u,
  nonMatchSymbols,
  "\\P{Script=Limb}"
);
testPropertyEscapes(
  /^\P{sc=Limbu}+$/u,
  nonMatchSymbols,
  "\\P{sc=Limbu}"
);
testPropertyEscapes(
  /^\P{sc=Limb}+$/u,
  nonMatchSymbols,
  "\\P{sc=Limb}"
);

reportCompare(0, 0);
