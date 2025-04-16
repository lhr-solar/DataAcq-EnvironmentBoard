/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FS3000_H
#define __FS3000_H

#define FS3000_I2C_ADDRESS 0x28 // default FS3000 base address is 0x28

extern uint8_t airflow_high_byte; // raw airflow data from FS3000, high byte
extern uint8_t airflow_low_byte;  // raw airflow data from FS3000, low byte
extern uint16_t airflow_count;    // raw airflow data from FS3000, combined as 12-bit integer
extern uint8_t airflow_result;    // measured airflow value in m/s
extern uint8_t airflow_checksum;  // checksum for airflow received from FS3000

#endif
