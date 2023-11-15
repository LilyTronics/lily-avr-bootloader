/*
 * Fifo
 */


#include "fifo.h"


Fifo::Fifo(void) {
    m_lock = 0;
    m_data_count = 0;
    m_write_index = 0;
    m_read_index = 0;
}


uint8_t Fifo::put(uint8_t data_byte) {
    uint8_t result = 0;
    while (m_lock);
    m_lock = 1;

    if (m_data_count < FIFO_SIZE) {
        m_fifo[m_write_index] = data_byte;
        m_data_count++;
        m_write_index++;
        if (m_write_index >= FIFO_SIZE) {
            m_write_index = 0;
        }
        result = 1;
    }

    m_lock = 0;
    return result;
}


uint8_t Fifo::get(uint8_t& data_byte) {
    uint8_t result = 0;
    while (m_lock);
    m_lock = 1;

    if (m_data_count > 0) {
        data_byte = m_fifo[m_read_index];
        m_data_count--;
        m_read_index++;
        if (m_read_index >= FIFO_SIZE) {
            m_read_index = 0;
        }
        result = 1;
    }

    m_lock = 0;
    return result;
}
