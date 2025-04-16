/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SHT45_H
#define __SHT45_H

#define SHT45_I2C_ADDRESS 0xFD // default SHT45 base address is 0xFD
#define SHT45_CRC_POLY 0x31    // polynomial used to calculate checksum for SHT45 (from datasheet)
#define SHT45_CRC_INITIAL 0xFF // initial CRC value for SHT45 (from datasheet)

extern uint16_t temp_ticks; // raw temp data received from SHT45
extern uint8_t temp_degC;   // temp value in celsius

extern uint16_t rh_ticks;    // raw rh data received from SHT45
extern uint8_t rh_percentRH; // rh value as percentage

extern uint16_t checksum_temp; // checksum for temp received from SHT45
extern uint16_t checksum_rh;   // checksum for rh received from SHT45

#endif
