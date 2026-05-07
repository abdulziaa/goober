/**
 * General Operations Over Basic Embedded Radio (GOOBER) - 1.00.0
 * JavaScript Adaptation
 * 
 * Note: This implementation matches the C implementation in goober.c/goober.h,
 * which uses a fixed 5-byte header.
 */

/**
 * Calculates a 16-bit Fletcher Checksum.
 * @param {Uint8Array} data The data to checksum.
 * @param {number} count The number of bytes to include in the checksum calculation.
 * @returns {number} The 16-bit checksum.
 */
export function fletcher16(data, count) {
    let sum1 = 0;
    let sum2 = 0;

    for (let index = 0; index < count; ++index) {
        sum1 = (sum1 + data[index]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }

    return (sum2 << 8) | sum1;
}

export class GooberPacket {
    constructor() {
        this.devId = 0;
        
        // DEV_MODE bits
        this.transmissionMode = 1; // 1: simplex, 2: half-duplex, 3: full-duplex
        this.intentBit = 0;        // 1: Transmitter intends to send another packet
        this.sizeBit = 0;          // Note: In goober.c, this bit is set but not used for logic
        this.checksumBit = 0;      // Note: In goober.c, this bit is set but not used for logic
        this.commandOnlyBit = 0;   // Note: In goober.c, this bit is set but not used for logic

        this.seqId = 0;
        this.msgCls = 1;
        this.payload = new Uint8Array(0); // Payload Data
    }

    /**
     * Gets the DEV_MODE byte representation.
     * @returns {number} The composed DEV_MODE byte.
     */
    get devMode() {
        let mode = 0;
        mode |= (this.transmissionMode & 0x03);
        mode |= ((this.intentBit & 0x01) << 2);
        mode |= ((this.sizeBit & 0x01) << 3);
        mode |= ((this.checksumBit & 0x01) << 4);
        mode |= ((this.commandOnlyBit & 0x01) << 5);
        return mode;
    }

    /**
     * Sets the DEV_MODE bits from a given byte.
     * @param {number} mode The DEV_MODE byte.
     */
    set devMode(mode) {
        this.transmissionMode = mode & 0x03;
        this.intentBit = (mode >> 2) & 0x01;
        this.sizeBit = (mode >> 3) & 0x01;
        this.checksumBit = (mode >> 4) & 0x01;
        this.commandOnlyBit = (mode >> 5) & 0x01;
    }

    /**
     * Serializes the GooberPacket into a Uint8Array.
     * Following goober.c implementation (5-byte header).
     * @returns {Uint8Array} The serialized GOOBER packet.
     */
    serialize() {
        const payloadDataSize = this.payload.length;
        const totalLength = 5 + payloadDataSize;

        const buffer = new ArrayBuffer(totalLength);
        const view = new DataView(buffer);
        const bytes = new Uint8Array(buffer);

        // Populate Header (5 bytes)
        view.setUint8(0, this.devId);
        view.setUint8(1, this.devMode);
        view.setUint8(2, this.seqId);
        view.setUint8(3, this.msgCls);
        view.setUint8(4, payloadDataSize);

        // Populate Payload
        if (payloadDataSize > 0) {
            bytes.set(this.payload, 5);
        }

        return bytes;
    }

    /**
     * Deserializes a buffer into a GooberPacket.
     * Following goober.c implementation (5-byte header).
     * @param {Uint8Array|ArrayBuffer} buffer The serialized packet buffer.
     * @returns {GooberPacket} The deserialized GooberPacket instance.
     */
    static deserialize(buffer) {
        let view, bytes;
        if (buffer instanceof ArrayBuffer) {
            view = new DataView(buffer);
            bytes = new Uint8Array(buffer);
        } else if (buffer instanceof Uint8Array) {
            view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
            bytes = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        } else {
            throw new Error("Buffer must be an ArrayBuffer or Uint8Array");
        }

        if (bytes.length < 5) {
            throw new Error("Buffer too short to be a valid GOOBER packet (need at least 5 bytes)");
        }

        const packet = new GooberPacket();
        packet.devId = view.getUint8(0);
        packet.devMode = view.getUint8(1);
        packet.seqId = view.getUint8(2);
        packet.msgCls = view.getUint8(3);
        const payloadLength = view.getUint8(4);

        if (bytes.length < 5 + payloadLength) {
            throw new Error(`Buffer length (${bytes.length}) does not match expected size (${5 + payloadLength})`);
        }

        if (payloadLength > 0) {
            // Copy payload data
            packet.payload = new Uint8Array(bytes.buffer, bytes.byteOffset + 5, payloadLength).slice();
        } else {
            packet.payload = new Uint8Array(0);
        }

        return packet;
    }
}
