#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PAYLOAD_LENGTH 250
#define GOOBER_HEADER_SIZE 5

enum goober_error_t {
    GOOBER_ERROR_PACKET_TOO_LARGE = -1,
    GOOBER_ERROR_PACKET_TOO_SMALL = -2,
    GOOBER_ERROR_INVALID_PAYLOAD_LENGTH = -3,
    GOOBER_ERROR_PAYLOAD_LENGTH_MISMATCH_TOO_SMALL = -4,
    GOOBER_ERROR_PAYLOAD_LENGTH_MISMATCH_TOO_LARGE = -5,
    GOOBER_ERROR_SERIALIZED_BUFFER_INVALID = -6,
    GOOBER_ERROR_SERIALIZED_BUFFER_TOO_SMALL = -7,
    GOOBER_ERROR_PAYLOAD_BUFFER_TOO_SMALL = -8,
    GOOBER_ERROR_HEADER_PAYLOAD_LENGTH_MISMATCH = -9,
    GOOBER_ERROR_INVALID_INPUT = -99,
};

typedef struct goober_header {
    uint8_t dev_id;
    uint8_t dev_mode;
    uint8_t seq_id;
    uint8_t msg_cls;
    uint8_t payload_length;
} goober_header_t;

int goober_deserialize(uint8_t *incoming_buffer, size_t incoming_buffer_size, goober_header_t *header, uint8_t *payload, size_t payload_buffer_size, size_t *payload_size);
int goober_serialize(goober_header_t header, uint8_t *payload_buffer, size_t payload_size, uint8_t *serialized_buffer, size_t serialized_buffer_size, size_t *serialized_packet_size);
uint8_t goober_device_mode(uint8_t transmission_mode, bool intent_bit, bool size_bit, bool checksum_bit, bool command_only_bit);