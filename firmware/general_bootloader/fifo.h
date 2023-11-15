/*
 * Fifo
 */


#ifndef FIFO_H_
#define FIFO_H_

#include <avr/io.h>

#define FIFO_SIZE       10


class Fifo {
    public:
        Fifo(void);
        uint8_t put(uint8_t data_byte);
        uint8_t get(uint8_t& data_byte);

    private:
        uint8_t m_fifo[FIFO_SIZE];
        uint8_t m_lock;
        uint8_t m_data_count;
        uint8_t m_write_index;
        uint8_t m_read_index;

};

#endif /* FIFO_H_ */
