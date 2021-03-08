#include <cbuffer.h>
#include <cstdint>

struct BipBuffer
{
  /** Write data to the tail of the circular buffer.
   * Increases the position of the tail.
   * This copies data to the circular buffer using memcpy.
   * After calling you should free the data if you don't need it anymore.
   *
   * @param cb The circular buffer.
   * @param data Buffer to be written to the circular buffer.
   * @param size Size of the buffer to be added to the circular buffer in bytes.
   * @return number of bytes offered
  **/
  int offer(const uint8_t* data, int size) noexcept;

  /** Look at data at the circular buffer's head.
   * Use cbuf_usedspace to determine how much data in bytes can be read.
   * @param cb The circular buffer.
   * @return pointer to the head of the circular buffer
  **/
  uint8_t* peek() const noexcept;
  /** Release data at the head from the circular buffer.
   * Increase the position of the head.
   *
   * WARNING: this is a dangerous call if:
   *  1. You are using the returned data pointer.
   *  2. Another thread has offerred data to the circular buffer.
   *
   * @param cb The circular buffer.
   * @param size Number of bytes to release
   * @return pointer to data; NULL if we can't poll this much data
  **/
  uint8_t* poll(int size) noexcept;

  int size() const noexcept { return cbuf_size(&m_cbuf); }
  int used() const noexcept { return cbuf_usedspace(&m_cbuf); }
  int unused() const noexcept { return cbuf_unusedspace(&m_cbuf); }
  bool empty() const noexcept { return cbuf_is_empty(&m_cbuf); }

  /** Construct buffer with @size given in bytes, following the same rules
      as dictated by mmap(2), which usually means size must be a multiple
      of the systems smallest available page-size **/
  BipBuffer(int size);
  ~BipBuffer();
private:
  cbuf_t m_cbuf;
};

inline BipBuffer::BipBuffer(int size)
{
  cbuf_new(&this->m_cbuf, size);
}
inline BipBuffer::~BipBuffer()
{
  cbuf_free(&this->m_cbuf);
}

inline int BipBuffer::offer(const uint8_t* data, int size) noexcept
{
  return cbuf_offer(&this->m_cbuf, data, size);
}
inline uint8_t* BipBuffer::peek() const noexcept
{
  return cbuf_peek(&this->m_cbuf);
}
inline uint8_t* BipBuffer::poll(int size) noexcept
{
  return cbuf_poll(&this->m_cbuf, size);
}
