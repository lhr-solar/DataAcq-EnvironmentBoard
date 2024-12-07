# DataAcq-EnvironmentBoard
Author: Ravi Shah

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

## Schematic

## Additional Notes
I know the connectors aren't numbered right, will fix in next iteration.  
not all sensors need to be there...will use breakout in car due to sizing constraints

TODO: Add connectors/pinout guide, testpoints, bom, schematic, pcb, etc. 


