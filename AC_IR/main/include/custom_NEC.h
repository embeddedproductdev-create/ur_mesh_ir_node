#ifndef MAIN_INCLUDE_CUSTOM_NEC_H_
#define MAIN_INCLUDE_CUSTOM_NEC_H_

#include <custom_irsend.h>

#define kPeriodOffset               -5
#define kDutyDefault                50
#define kDutyMax                    100
#define kNECBits                    32
#define kNECBits                    32
#define kNecTick                    560
#define kNecHdrMarkTicks            16
#define kNecHdrMark                 kNecHdrMarkTicks * kNecTick
#define kNecHdrSpaceTicks           8
#define kNecHdrSpace                kNecHdrSpaceTicks * kNecTick
#define kNecBitMarkTicks            1
#define kNecBitMark                 kNecBitMarkTicks * kNecTick
#define kNecOneSpaceTicks           3
#define kNecOneSpace                kNecOneSpaceTicks * kNecTick
#define kNecZeroSpaceTicks          1
#define kNecZeroSpace               kNecZeroSpaceTicks * kNecTick
#define kNecRptSpaceTicks           4
#define kNecRptSpace                kNecRptSpaceTicks * kNecTick
#define kNecRptLength               4
#define kNecMinCommandLengthTicks   193
#define kNecMinCommandLength        kNecMinCommandLengthTicks * kNecTick
#define kNecMinGap                  kNecMinCommandLength - (kNecHdrMark + kNecHdrSpace + kNECBits * (kNecBitMark + kNecOneSpace) + kNecBitMark)
#define kNecMinGapTicks             kNecMinCommandLengthTicks - (kNecHdrMarkTicks + kNecHdrSpaceTicks + kNECBits * (kNecBitMarkTicks + kNecOneSpaceTicks) + kNecBitMarkTicks)

void ir_send_NEC_command(uint64_t data, uint16_t nbits, uint16_t repeat);

#endif /* MAIN_INCLUDE_CUSTOM_NEC_H_ */
