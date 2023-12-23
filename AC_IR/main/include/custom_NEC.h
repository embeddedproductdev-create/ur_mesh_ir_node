/*
 * custom_NEC.h
 *
 *  Created on: 21-Dec-2023
 *      Author: EmbeddedDevelopment
 */

#ifndef MAIN_INCLUDE_CUSTOM_NEC_H_
#define MAIN_INCLUDE_CUSTOM_NEC_H_

#include <custom_irsend.h>

const uint16_t kNecTick = 560;
const uint16_t kNecHdrMarkTicks = 16;
const uint16_t kNecHdrMark = kNecHdrMarkTicks * kNecTick;
const uint16_t kNecHdrSpaceTicks = 8;
const uint16_t kNecHdrSpace = kNecHdrSpaceTicks * kNecTick;
const uint16_t kNecBitMarkTicks = 1;
const uint16_t kNecBitMark = kNecBitMarkTicks * kNecTick;
const uint16_t kNecOneSpaceTicks = 3;
const uint16_t kNecOneSpace = kNecOneSpaceTicks * kNecTick;
const uint16_t kNecZeroSpaceTicks = 1;
const uint16_t kNecZeroSpace = kNecZeroSpaceTicks * kNecTick;
const uint16_t kNecRptSpaceTicks = 4;
const uint16_t kNecRptSpace = kNecRptSpaceTicks * kNecTick;
const uint16_t kNecRptLength = 4;
const uint16_t kNecMinCommandLengthTicks = 193;
const uint32_t kNecMinCommandLength = kNecMinCommandLengthTicks * kNecTick;
const uint32_t kNecMinGap = kNecMinCommandLength -
    (kNecHdrMark + kNecHdrSpace + kNECBits * (kNecBitMark + kNecOneSpace) +
     kNecBitMark);
const uint16_t kNecMinGapTicks =
    kNecMinCommandLengthTicks -
    (kNecHdrMarkTicks + kNecHdrSpaceTicks +
     kNECBits * (kNecBitMarkTicks + kNecOneSpaceTicks) + kNecBitMarkTicks);

void sendNEC(uint64_t data, uint16_t nbits, uint16_t repeat);

#endif /* MAIN_INCLUDE_CUSTOM_NEC_H_ */
