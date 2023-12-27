#ifndef MAIN_INCLUDE_CUSTOM_HITACHI_H_
#define MAIN_INCLUDE_CUSTOM_HITACHI_H_

// Constants
#define HITACHI296_NBYTES			37
#define kDefaultMessageGap			100000
#define kHitachiAcHdrMark			3300
#define kHitachiAcHdrSpace			1700
#define kHitachiAc1HdrMark			3400
#define kHitachiAc1HdrSpace			3400
#define kHitachiAcBitMark			400
#define kHitachiAcOneSpace			1250
#define kHitachiAcZeroSpace			500
#define kHitachiAcMinGap			kDefaultMessageGap  // Just a guess.
#define kHitachiAc296MaxTemp		31
#define kHitachiAc296MinTemp		16

/*Fan and Mode values*/
#define kHitachiAc296Cool                	0b0011
#define kHitachiAc296DryCool            	0b0100
#define kHitachiAc296Dehumidify          	0b0101
#define kHitachiAc296Heat                	0b0110
#define kHitachiAc296Auto                	0b0111
#define kHitachiAc296AutoDehumidifying   	0b1001
#define kHitachiAc296QuickLaundry        	0b1010
#define kHitachiAc296CondensationControl 	0b1100

#define kHitachiAc296FanSilent 				0b001
#define kHitachiAc296FanLow    				0b010
#define kHitachiAc296FanMedium 				0b011
#define kHitachiAc296FanHigh   				0b100
#define kHitachiAc296FanAuto   				0b101

#define kHitachiAc296TempAuto 				1  // Special value for "Auto" op mode.

#define kHitachiAc296PowerOn  				1
#define kHitachiAc296PowerOff 				0

typedef union HitachiAC296Protocol{
  uint8_t raw[HITACHI296_NBYTES];
  struct {
    // Byte 0~12
    uint8_t pad0[13];
    // Byte 13
    uint8_t                    :2;
    uint8_t Temp               :5;  // LSB
    uint8_t                    :1;
    // Byte 14
    uint8_t                    :8;
    // Byte 15~16
    uint8_t                    :8;
    uint8_t                    :8;
    // Byte 17~24
    uint8_t OffTimerLow        :8;  // LSB
    uint8_t /* Parity */       :8;
    uint8_t OffTimerHigh       :8;
    uint8_t /* Parity */       :8;
    uint8_t OnTimerLow         :8;  // LSB
    uint8_t /* Parity */       :8;
    uint8_t OnTimerHigh        :4;
    uint8_t OffTimerActive     :1;
    uint8_t OnTimerActive      :1;
    uint8_t                    :2;
    uint8_t /* Parity */       :8;
    // Byte 25~26
    uint8_t Mode               :4;
    uint8_t Fan                :3;
    uint8_t                    :1;
    uint8_t                    :8;
    // Byte 27~28
    uint8_t                    :4;
    uint8_t Power              :1;
    uint8_t                    :2;
    uint8_t TimerActive        :1;
    uint8_t                    :8;
    // Byte 29~34
    uint8_t pad1[6];
    // Byte 35~36
    uint8_t                    :4;
    uint8_t Humidity           :4;  // LSB
    uint8_t                    :8;
  };
}HitachiAC296Protocol_t;

extern HitachiAC296Protocol_t data_Hitachi296;

/*Function Declarations*/

void data_init_Hitachi296();
void send_Hitachi296(uint8_t repeat, bool MSBfirst);
void setTemp_Hitachi296(const uint8_t temp);
void setMode_Hitachi296(const uint8_t mode);
void setFan_Hitachi296(const uint8_t speed);
void setInvertedStates_Hitachi296(void);
uint8_t * invertBytePairs(uint8_t *ptr, const uint16_t length);

#endif /* MAIN_INCLUDE_CUSTOM_HITACHI_H_ */
