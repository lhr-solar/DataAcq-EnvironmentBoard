// TODO: write driver functions for MPL3115A2ST1 pressure sensor...

/*

From Perplexity:
Here’s the raw I2C command sequence to configure and read the MPL3115A2S sensor without interrupts, using only address and data bytes. Each step represents a complete I2C transaction (start + address + data + stop).

1. Software Reset
Goal: Reset the sensor to default settings.
Transaction:

Address: 0xC0 (write)

Data: [0x27, 0x01]

0x27: CTRL_REG2 address

0x01: Trigger reset

2. Configure Sensor Mode
Goal: Set barometer mode with 128x oversampling.
Transaction:

Address: 0xC0 (write)

Data: [0x26, 0xB8]

0x26: CTRL_REG1 address

0xB8: Active mode (SBYB=1), 128x oversampling (OS=111), barometer mode (ALT=0)

3. Enable Data Flags
Goal: Enable pressure data and data-ready flag.
Transaction:

Address: 0xC0 (write)

Data: [0x13, 0x07]

0x13: PT_DATA_CFG address

0x07: Enable pressure, temperature, and data-ready flags

4. Poll for Data Readiness
Goal: Check if pressure data is ready.
Steps:

Set STATUS register pointer:

Address: 0xC0 (write)

Data: [0x00] (STATUS register address)

Read STATUS register:

Address: 0xC1 (read)

Received Data: Check bit 2 (PTDR). If 0x04, data is ready.

Repeat steps 1–2 until PTDR=1.

5. Read Pressure Data
Goal: Fetch the 20-bit pressure value.
Steps:

Set pressure register pointer:

Address: 0xC0 (write)

Data: [0x01] (OUT_P_MSB address)

Read 3 bytes (MSB, CSB, LSB):

Address: 0xC1 (read)

Received Data: 3 bytes (e.g., [0x12, 0x34, 0x56])

Raw Byte Summary
Step	Address (Write)	Data Sent	Address (Read)	Data Received
Reset	0xC0	[0x27, 0x01]	—	—
Mode	0xC0	[0x26, 0xB8]	—	—
Flags	0xC0	[0x13, 0x07]	—	—
Poll	0xC0	[0x00]	0xC1	1 byte
Read	0xC0	[0x01]	0xC1	3 bytes
Example Pressure Calculation
If received bytes are [0x12, 0x34, 0x56]:

Combine: 0x12 << 16 | 0x34 << 8 | 0x56 = 0x123456

Shift right by 4: 0x123456 >> 4 = 0x12345 (20-bit value)

Convert to Pascals: 0x12345 * 0.25 = [result] Pa

Notes
Replace 0xC0/0xC1 with your sensor’s I2C address if different.

Add delays (~1 ms) after reset and between transactions if needed.

For repeated reads, repeat Steps 4–5.
*/