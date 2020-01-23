
```mermaid
sequenceDiagram
note over GUI,MCU: Part 1: Discover slave
GUI -x+ MCU: Read UID (0x01)
MCU -->> GUI: Acknowledge (0x07)
MCU -->>- GUI: UID
GUI -x+ MCU: Read Name (0x02)
MCU -->> GUI: Acknowledge (0x07)
MCU -->>- GUI: Name
GUI -x+ MCU: Read Version (0x03)
MCU -->> GUI: Acknowledge (0x07)
MCU -->>- GUI: Version
GUI -x+ MCU: Read Device Count (0x04)
MCU -->> GUI: Acknowledge (0x07)
MCU -->>- GUI: Device Count
note over GUI,MCU: Part 2: Read slave capabilities
loop Every Device
GUI -x+ MCU: Read Device Type (0x05)
MCU -->> GUI: Acknowledge (0x07)
MCU -->>- GUI: Device Type
opt Device Status 
GUI -x+ MCU: Read Device Status (0x06)
MCU -->>- GUI: Status Information
end
end
note over GUI,MCU: Part 3: Refresh status information
opt Device Status 
loop Every Second
loop Every Device
GUI -x+ MCU: Read Device Status (0x06)
MCU -->>- GUI: Status Information
end
end
end
```
```
> Written with [StackEdit](https://stackedit.io/).
