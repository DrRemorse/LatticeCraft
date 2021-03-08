#include "client.hpp"

namespace server
{
  Client::Client(int self, int fd)
    : m_self(self), m_fd(fd), m_readq(16384), m_writeq(16384)
  {
    
  }
}
