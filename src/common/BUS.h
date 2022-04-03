//
// Created by Domo2 on 4/1/2022.
//

#ifndef DAQV7_BUS_H
#define DAQV7_BUS_H

#define GLOBAL_ADDRESS 0x7F

#define HEADER_MAGIC_START 0x7E
#define HEADER_SIZE 4
#define TRAILER_SIZE 2

#define PARSER_TIMEOUT_MS 0
#define BUS_TIMEOUT_MS 0

#define HEADER_INDEX_ADDRESS 1
#define HEADER_INDEX_SIZE_LSB 2
#define HEADER_INDEX_SIZE_MSB 3

#define TRAILER_INDEX_CHECK_LSB 0
#define TRAILER_INDEX_CHECK_MSB 1

struct PacketHeader {
    size_t data_size;
    bool response_expected;
    uint8_t address;
};

struct PacketTrailer {
    uint16_t checksum;
};


struct BusOptions {
    uint8_t address;
    uint8_t pin_driver_enable;
    uint32_t baudrate;
    uint64_t parser_timeout_ms;
    uint64_t lock_timeout_ms;
};

// Writes a packet header into the passed buffer
PacketHeader generate_header(uint8_t *dest, uint16_t data_size, uint8_t dest_address, bool response_required) {
    dest[0] = HEADER_MAGIC_START;
    dest[HEADER_INDEX_ADDRESS] = (dest_address & 0x7F) | (response_required ? 0x80 : 0x00);
    dest[HEADER_INDEX_SIZE_LSB] = data_size;
    dest[HEADER_INDEX_SIZE_MSB] = data_size >> 8;

    return PacketHeader{data_size, response_required, dest_address};
}

// Writes a packet trailer into the passed buffer
PacketTrailer generate_trailer(uint8_t *dest, uint8_t *data, uint16_t data_size) {
    dest[TRAILER_INDEX_CHECK_LSB] = 0xAB;
    dest[TRAILER_INDEX_CHECK_MSB] = 0xAB;

    return PacketTrailer{0xABAB};
}

#endif //DAQV7_BUS_H
