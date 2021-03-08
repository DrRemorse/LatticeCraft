namespace cppcraft
{
  inline GridWalker::GridWalker(int bx, int by, int bz)
  {
    this->sector = Spiders::wrap(bx, by, bz);
    this->x = bx;
    this->y = by;
    this->z = bz;
  }
  inline GridWalker::GridWalker(Sector& s, int bx, int by, int bz)
    : sector(&s), x(bx), y(by), z(bz) {}
  inline GridWalker::GridWalker(const GridWalker& other)
    : sector(other.sector), x(other.x), y(other.y), z(other.z) {}
  inline GridWalker& GridWalker::operator= (const GridWalker& other) {
    sector = other.sector; x = other.x; y = other.y; z = other.z; return *this;
  }

  inline GridWalker& GridWalker::move_x(int dx)
  {
    const int ds = (x + dx) / Sector::BLOCKS_XZ;
    this->x = (x + dx) & Sector::BLOCKS_XZ-1;
    if (sector->getX() + ds >= 0 && sector->getX() + ds < sectors.getXZ())
      sector = &sectors(sector->getX()+ds, sector->getZ());
    else
      sector = nullptr;
    return *this;
  }
  inline GridWalker& GridWalker::move_y(int dy)
  {
    this->y += dy;
    if (y < 0 || y >= Sector::BLOCKS_Y) this->sector = nullptr;
    return *this;
  }
  inline GridWalker& GridWalker::move_z(int dz)
  {
    const int ds = (z + dz) / Sector::BLOCKS_XZ;
    this->z = (z + dz) & Sector::BLOCKS_XZ-1;
    if (sector->getZ() + ds >= 0 && sector->getZ() + ds < sectors.getXZ())
      sector = &sectors(sector->getX(), sector->getZ()+ds);
    else
      sector = nullptr;
    return *this;
  }
  inline GridWalker& GridWalker::move_xz(int dx, int dz)
  {
    this->move_x(dx);
    this->move_z(dz);
    return *this;
  }
  inline GridWalker& GridWalker::move(int dx, int dy, int dz)
  {
    if (dx) this->move_x(dx);
    if (dy) this->move_y(dy);
    if (dz) this->move_z(dz);
    return *this;
  }

  inline Block& GridWalker::get() const
  {
    return (*sector)(this->x, this->y, this->z);
  }
  inline bool GridWalker::set(const Block& blk) const
  {
    return Spiders::setBlock(*sector, x, y, z, blk);
  }
  inline Block GridWalker::remove() const
  {
    return Spiders::removeBlock(*sector, x, y, z);
  }
  inline Block& GridWalker::peek_above() const
  {
    return (*sector)(this->x, this->y+1, this->z);
  }
}
