/**
* @file goober.c
* @author Abdul Zia (abdul@abdulzia.com)
* @brief General Operations Over Basic Embedded Radio Packet Serializer/Deserializer
* @version 1.0.0
* @date 2025-12-06
* 
* @copyright Copyright (c) 2025 Abdul Zia. All rights reserved.
* 
*/

#include "goober.h"

/**
 * @brief 16 Bit Fletcher Checksum. Fastest one that I could easily find online.
 * 
 * @param buf Pointer to buffer of bytes to have checksum calculated over. 
 * @param len Length of the buffer of bytes to have checksum calculated over.
 * @return uint16_t The 16 bit fletcher checksum.
 */
uint16_t fletcher16(const uint8_t *buf, size_t len) { // see https://en.wikipedia.org/wiki/Fletcher%27s_checksum#:~:text=%25%200xff)%3B-,Optimizations,-%5Bedit%5D
	uint32_t c0, c1;

	for (c0 = c1 = 0; len > 0; ) {
		size_t blocklen = len;
		if (blocklen > 5802) {
			blocklen = 5802;
		}
		len -= blocklen;
		do {
			c0 = c0 + *buf++;
			c1 = c1 + c0;
		} while (--blocklen);
		c0 = c0 % 255;
		c1 = c1 % 255;
   }
   return (c1 << 8 | c0);
}

/**
* @brief Deserialize a buffer of bytes into a GOOBER packet header and payload.
* 
* @param incoming_buffer Pointer to the incoming buffer of bytes to be deserialized.
* @param incoming_buffer_size Size of the incoming buffer in bytes.
* @param header Pointer to the header struct to be populated.
* @param payload Pointer to the payload buffer to be populated.
* @param payload_buffer_size Size of the allocated payload buffer in bytes.
* @param payload_size Pointer to the size of the payload in bytes.
* @return int Error code. 0 if successful, otherwise a negative error code.
*/
int goober_deserialize(uint8_t *incoming_buffer, size_t incoming_buffer_size, goober_header_t *header, uint8_t *payload, size_t payload_buffer_size, size_t *payload_size)
{
    if (incoming_buffer == NULL || incoming_buffer_size == 0 || header == NULL || payload == NULL || payload_size == NULL) { // is the input invalid?
        return GOOBER_ERROR_INVALID_INPUT;
    }

    if (incoming_buffer_size > (GOOBER_HEADER_SIZE + MAX_PAYLOAD_LENGTH)) { // is the packet too large to be a valid GOOBER packet?
        return GOOBER_ERROR_PACKET_TOO_LARGE;
    }

    if (incoming_buffer_size < GOOBER_HEADER_SIZE) { // is the packet too small to be a valid GOOBER packet?
        return GOOBER_ERROR_PACKET_TOO_SMALL;
    }

    header->dev_id = incoming_buffer[0];
    header->dev_mode = incoming_buffer[1];
    header->seq_id = incoming_buffer[2];
    header->msg_cls = incoming_buffer[3];
    header->payload_length = incoming_buffer[4];

    if (header->payload_length > MAX_PAYLOAD_LENGTH) { // is the payload length too large?
        return GOOBER_ERROR_INVALID_PAYLOAD_LENGTH;
    }

    if (incoming_buffer_size < (GOOBER_HEADER_SIZE + header->payload_length)) { // is the packet smaller than expected?
        return GOOBER_ERROR_PAYLOAD_LENGTH_MISMATCH_TOO_SMALL;
    }

    if (incoming_buffer_size > (GOOBER_HEADER_SIZE + header->payload_length)) { // is the packet larger than expected?
        return GOOBER_ERROR_PAYLOAD_LENGTH_MISMATCH_TOO_LARGE;
    }

    if (header->payload_length == 0) { // is the payload length 0?
        *payload_size = 0;
        return 0; // zero payload is valid, just don't copy anything
    }

    if (payload_buffer_size < header->payload_length) { // is the payload buffer too small to contain the payload?
        return GOOBER_ERROR_PAYLOAD_BUFFER_TOO_SMALL;
    }

    memcpy(payload, incoming_buffer + GOOBER_HEADER_SIZE, header->payload_length); // copy the payload into the payload buffer
    *payload_size = header->payload_length;

    return 0; // success!
}

/**
 * @brief Serialize a GOOBER packet header and payload into a buffer of bytes.
 * 
 * @param header Pointer to the header struct to be serialized.
 * @param payload_buffer Pointer to the payload buffer to be serialized.
 * @param payload_size Size of the payload buffer in bytes.
 * @param serialized_buffer Pointer to the buffer to be populated with the serialized packet.
 * @param serialized_buffer_size Size of the allocated serialized buffer in bytes.
 * @param serialized_packet_size Pointer to the size of the serialized packet in bytes.
 * @return int Error code. 0 if successful, otherwise a negative error code.
 */
int goober_serialize(goober_header_t header, uint8_t *payload_buffer, size_t payload_size, uint8_t *serialized_buffer, size_t serialized_buffer_size, size_t *serialized_packet_size)
{
    if (payload_buffer == NULL || serialized_buffer == NULL || serialized_packet_size == NULL) { // is the input invalid?
        return GOOBER_ERROR_INVALID_INPUT;
    }

    if (payload_size > MAX_PAYLOAD_LENGTH) { // is the payload too large to be a valid GOOBER packet?
        return GOOBER_ERROR_INVALID_PAYLOAD_LENGTH;
    }

    if (serialized_buffer_size < GOOBER_HEADER_SIZE) { // is the allocated serialized buffer too small to contain the header?
        return GOOBER_ERROR_SERIALIZED_BUFFER_INVALID;
    }

    if (serialized_buffer_size < GOOBER_HEADER_SIZE + payload_size) { // is the allocated serialized buffer too small to contain the packet?
        return GOOBER_ERROR_SERIALIZED_BUFFER_TOO_SMALL;
    }

    serialized_buffer[0] = header.dev_id;
    serialized_buffer[1] = header.dev_mode;
    serialized_buffer[2] = header.seq_id;
    serialized_buffer[3] = header.msg_cls;
    serialized_buffer[4] = payload_size;

    if (payload_size == 0) { // is the payload length 0?
        *serialized_packet_size = GOOBER_HEADER_SIZE;
        return 0; // zero payload is valid, just don't copy anything
    }

    memcpy(serialized_buffer + GOOBER_HEADER_SIZE, payload_buffer, payload_size); // copy the payload into the serialized buffer
    *serialized_packet_size = GOOBER_HEADER_SIZE + payload_size;

    return 0; // success!
}

uint8_t goober_device_mode(uint8_t transmission_mode, bool intent_bit, bool size_bit, bool checksum_bit, bool command_only_bit)
{
    uint8_t dev_mode = 0;
    
    // Set transmission mode (bits 0-1)
    dev_mode |= (transmission_mode & 0x03);
    
    // Set intent bit (bit 2)
    if (intent_bit) {
        dev_mode |= (1 << 2);
    }
    
    // Set size bit (bit 3)
    if (size_bit) {
        dev_mode |= (1 << 3);
    }
    
    // Set checksum bit (bit 4)
    if (checksum_bit) {
        dev_mode |= (1 << 4);
    }
    
    // Set command-only bit (bit 5)
    if (command_only_bit) {
        dev_mode |= (1 << 5);
    }
    
    // Bits 6-7 are reserved and remain 0
    
    return dev_mode;
}