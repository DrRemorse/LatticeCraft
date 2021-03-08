#pragma once
#include "bipbuffer.hpp"

namespace server
{
  struct Client
  {
    Client(int self, int fd);

  private:
    int m_self;
    int m_fd;
    BipBuffer m_readq;
    BipBuffer m_writeq;
  };
}
