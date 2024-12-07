# DataAcq-EnvironmentBoard
Author: Ravi Shah  
Documentation: https://cloud.wikis.utexas.edu/wiki/spaces/LHRSOLAR/pages/205226271/Data+Acquisition+Environment+Board


The Data Acquisition Environment Board measures environmental data throughout the car, including relative humidity, airflow, and temperature. It interfaces with a Peripheral System-On-Module (SOM) to process data and transmit it via the Data Acquisition CAN bus.

## BOM
IBOM: Hardware/BOM/ibom.html (download and view in browser)  
Mouser Cart: https://www.mouser.com/ProjectManager/ProjectDetail.aspx?AccessID=8fef7b51e5

## Connectors
| # | Name | Type | Ideal Voltage | Notes |
| - | - | - | - | - |
| U4  | PSOM Connection | 53307-2471 Female |N/A | 3.65mm tall |
| J2  | CAN Output | MicroFit3.0_1x4xP3.00mm_PolarizingPeg_Vertical Female |+12V | 6.98mm tall |
| J3  | CAN Input | MicroFit3.0_1x4xP3.00mm_PolarizingPeg_Vertical Female | +12V | 6.98mm tall |
| J6  | Breakout | Nano-Fit_105310-xx10_2x05_P2.50mm_Vertical Female | +3.3V | 9.05mm tall |

## PCB
![image](https://github.com/user-attachments/assets/8d87c073-5e3f-42ed-a211-2bcffe3e15a8)

## Schematic
![image](https://github.com/user-attachments/assets/42e0ae49-749f-4f5a-ac19-4d762e09adc8)

## Additional Notes
- I know the connectors aren't numbered right, will fix in next iteration.  
- For production use (on the car), each main environment board will interface with a breakout board containing relevant sensors to the system being monitored. Wire harnessing should minimize the distance between the main board and the breakout to prevent noise. Main boards will be connected via EnvironmentCAN to each other and to the Data Acquisition leader board.
- Not all sensors need to be present, only necessary sensors should be soldered to the board/breakout according to system needs.
