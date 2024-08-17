#include <stdint.h>

#define PREAMBLE0 0x01
#define PREAMBLE1 0x02
#define PREAMBLE2 0x03
#define PREAMBLE3 0x04

typedef bool (*read_byte_func)(uint8_t *byte);

int check_preamble(const uint8_t *packet) {
    if (packet) {
        return packet[0] == PREAMBLE0 &&
               packet[1] == PREAMBLE1 &&
               packet[2] == PREAMBLE2 &&
               packet[3] == PREAMBLE3;
    } else { return 0; }
}

void set_preamble(uint8_t *packet) {
    if (packet) {
        packet[0] = PREAMBLE0;
        packet[1] = PREAMBLE1;
        packet[2] = PREAMBLE2;
        packet[3] = PREAMBLE3;
    }
}

uint8_t calc_crc(const uint8_t *packet) {
    uint8_t res = 0;
    if (packet) {
        uint8_t length = packet[4];
        for (int i = 0; i < length + 7; i++) {
            res ^= packet[i];
        }
    }
    return res;
}

int check_crc(const uint8_t *packet) {
    uint8_t length = packet[4];
    if (calc_crc(packet) == packet[7 + length]) {
        return 1;
    } else {
        return 0;
    }
}

int make_packet(uint8_t *result, const uint8_t *data, const uint8_t len, const uint8_t src, const uint8_t dst) {
    if (result && data) {
        set_preamble(result);
        result[4] = len;
        result[5] = src;
        result[6] = dst;
        for (int i = 0; i < len; i++) {
            result[i + 7] = data[i];
        }
        result[7 + len] = calc_crc(result);
        return 1;
    } else {
        return 0;
    }
}

int get_data(uint8_t *result, uint8_t *length, const uint8_t *packet) {
    if (result && packet && check_preamble(packet) && check_crc(packet)) {
        *length = packet[4];
        for (int i = 0; i < *length; i++) {
            result[i] = packet[i + 7];
        }
        return 1;
    } else {
        return 0;
    }
}

uint32_t get_current_time_ms() {
// нужно реализовать в конкретной системе. В FreeRTOS нужно будет поработать с функцией xTaskGetTickCount()
    return 0;
}

int receive_message(uint8_t *buffer, uint32_t timeout_ms,
                    read_byte_func read_byte) {  // я не стал заморачиваться с проверкой отправителя и получателя, так как это снова сильно зависит от реализации
    uint32_t start_time = get_current_time_ms();
    uint32_t current_time = start_time;
    uint8_t byte;
    uint8_t packet[263];
    uint8_t length = 0;

    for (int i = 0; i < length + 8; i++) {
        packet[i] = read_byte(&byte);
        if (i == 4) { length = packet[i]; }
        current_time = get_current_time_ms();
        if (current_time >= start_time + timeout_ms) {
            return 0;
        }
    }
    return 1;
}

// сначала мы получаем весь пакет с помощью eceive_message(), потом получаем из него данные.
