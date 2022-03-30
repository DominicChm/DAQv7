# DAQ v7

This is yet another revision of Baja's DAQ. Conceived once I realized a wireless system would realistically be death by
a thousand paper cuts.

## Goals

- Not wireless
- No CAN
- Gonna use RS-485 (transceivers are super small and cheap) in a point to multipoint configuration.
    - Gives all the throughput of CAN (10mbit) without the annoyances (hopefully) of CAN.
    - Literally just industrial serial over differential pairs (AFAIK)
    - Still only needs two differential pairs (so total bundle is 4 wires - 5v; 485+; 485-; GND)
    - I2C isn't good enough because datarate significantly drops with length, so our max rate would be like 10kbits/sec
      or smth
- Topology will have a few main "nodes" that gather data to put on the bus
    - (preliminary) locations - Car front, ECVT, Car top (for GPS + radio...?), Dashboard, maybe secondary dash?
    - We can have up to 32 nodes with the cheap-ass MAX485 chips
- Thinking of getting either M5 or M8 connectors to wire the
  bus (https://www.digikey.com/en/products/filter/circular-connectors/436?s=N4IgjCBcoLQCxVAYygMwIYBsDOBTANCAPZQDa4ArGAGwQC6Avk0A)
    - M5 are small, relatively cheap, and have 4-pin variants
- Next steps:
    - Order transceivers modules for POC (done)
    - Proof of concept (can start with just using serial - 485 is almost a drop-in replacement)
    - Order some M5 connectors
    - Do it

# General Packet Structure (Transport Layer)

| Byte | Size | Desc. |
|---|---|---|
| 0x7E | 1 | Magic Start |
| desc+addr | 1 | Address of target device and response expected flag. `<Bit 8: RESPOND><Bits 0-7: Address>` |
| size | 2 | Size of DATA |
| DATA | ? | Transmitted Data |
| checksum | 2 | crc-16 checksum of DATA |

A response is requested from the addressed device if the MSB of the address byte is set. A response is a global
message (address `0x7F`) with the response bit (MSB) set. Effectively, the address byte is 0xFF. The bus should remain
idle once a response is requested from a device until either it is received or a timeout is reached.

### Conventions

- The address `0x7F` is reserved for global, unaddressed messages. You cannot request a response on a global message.
- The bus must have a master-client architecture.
- The master's address should be `0x00`.

# Data Format (Application Layer)

Heartbeat
