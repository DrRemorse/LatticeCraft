#include "chunks.hpp"

#include <library/math/baseconv.hpp>
#include <library/config.hpp>
#include <library/log.hpp>
#include "compressor.hpp"
#include "sectors.hpp"
#include "world.hpp"
#include <cassert>

using namespace library;

namespace cppcraft
{
	static const int chunk_offset = Chunks::CHUNK_SIZE * Chunks::CHUNK_SIZE;

	Chunks chunks;

	void Chunks::initChunks()
	{
		Compressor::init();
	}

	void Chunks::addSector(Sector& sector)
	{
		// generate chunk token from sector
		int tmptokenX = (sector.getX() + world.getWX()) >> CHUNK_SH;
		int tmptokenZ = (sector.getZ() + world.getWZ()) >> CHUNK_SH;

		int foundChunk = -1;

		// for each chunk
		for (unsigned int c = 0; c < chunkq.size(); c ++)
		{
			if (chunkq[c].tokenX == tmptokenX  &&
				chunkq[c].tokenZ == tmptokenZ)
			{
				// we found a matching chunk,
				foundChunk = c;
				// add this sector to this chunk's write queue
				std::vector<Sector*>& wq = chunkq[c].writeq;

				for (unsigned int i = 0; i < wq.size(); i++)
				{
					// exit if sector is already in queue
					if (wq[i] == &sector) return;
				}

				// sector was not in write queue, add it
				break;
			}
		}

		if (foundChunk == -1)
		{
			// apparently we need to create the chunk item first
			// new chunk id is vector count
			foundChunk = chunkq.size();

			Chunk newChunk;
			newChunk.tokenX = tmptokenX;
			newChunk.tokenZ = tmptokenZ;
			// add chunk to chunk queue
			chunkq.push_back(newChunk);
		}

		// finally, add sector to write queue
		chunkq[foundChunk].writeq.push_back(&sector);

	} // addSector

	std::string Chunks::getSectorString(Sector& sector)
	{
		// base32 composite of X and Z absolute world coordinates
		return BaseConv::base32((sector.getX() + world.getWX()) >> Chunks::CHUNK_SH, 5) + "-" +
			   BaseConv::base32((sector.getZ() + world.getWZ()) >> Chunks::CHUNK_SH, 5);
	}

	// forwards
	void writeChunk(int);
	void writeSector(Sector*, std::fstream&);

	void Chunks::flushChunks()
	{
		if (chunkq.size())
		{
			// write each pending chunk to disk
			for (unsigned int chunk = 0; chunk < chunkq.size(); chunk++)
			{
				writeChunk(chunkq[chunk]);
			}
			// clean out chunk queue
			chunkq.clear();
		}

	}

	void Chunks::writeChunk(Chunk& chunk)
	{
		// make chunk filename
		std::string file = world.worldFolder() + "/" + getSectorString(*chunk.writeq[0]) + ".chunk";

		// open chunk file
		std::fstream File( file.c_str(), std::ios::in | std::ios::out | std::ios::binary);

		// check if the file was opened
		if (!File)
		{
			// try creating the file
			File.open(file.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

			if (!File)
			{
				logger << Log::ERR << "Could not create file: " << file << Log::ENDL;
				return;
			}
		}

		// write each pending element
		for (unsigned int i = 0; i < chunk.writeq.size(); i++)
		{
			writeSector(*chunk.writeq[i], File);
		}

		// clean out all sectors in this chunks write queue
		chunk.writeq.clear();
	}

	void Chunks::writeSector(Sector& s, std::fstream& File)
	{
		int dx = (world.getWX() + s.getX()) & (CHUNK_SIZE - 1);
		int dz = (world.getWZ() + s.getZ()) & (CHUNK_SIZE - 1);

		int P = (1 + dx + dz * CHUNK_SIZE) * sizeof(int);
		int PL;

		// get location of data for this sector (slot)
		// relative to beginning of file
		File.seekg(P);
		// read position data
		File.read((char*) &PL, sizeof(PL));

		// if we failed to read, PL must be set to 0
		if (!File) PL = 0;

		if (PL == 0)
		{   // sector was not written! sound the alarm!
			int currentCnt;
			// get current number of sectors already written
			File.seekg(0);
			File.read( (char*) &currentCnt, sizeof(currentCnt) );

			// if we failed to read, currentCnt must be set to 0
			if (!File) currentCnt = 0;

			PL = (1 + chunk_offset) * sizeof(int) + currentCnt * sizeof(sectorblock_t);

			// put location of data
			File.seekp(P);
			File.write( (char*) &PL, sizeof(PL) );

			// increase count, and update it
			currentCnt++;
			File.seekp(0);
			File.write( (char*) &currentCnt, sizeof(currentCnt) );
		}

		// reset all state flags
		File.clear();

		// write sectorblock_t to disk
		File.seekp(PL);
		File.write( (char*) &s.getBlocks(), sizeof(sectorblock_t) );

		if (!File)
		{
			logger << Log::ERR << "Error writing sectoral data: " << PL << Log::ENDL;
			logger << Log::ERR << "Chunks in chunkq: " << chunkq.size() << Log::ENDL;
			for (unsigned int i = 0; i < chunkq.size(); i++)
				logger << Log::ERR << "Chunk " << i << " has " << chunkq[i].writeq.size() << " sectors." << Log::ENDL;

			throw std::string("failed to write sector block data");
		}

	} // writeSector

	bool Chunks::loadSector(Sector& sector, std::ifstream& File, unsigned int PL)
	{
		// sector needs to have blocks allocated
    auto& blocks = sector.getBlocks();

		File.seekg(PL);
		File.read( (char*) &blocks, sizeof(sectorblock_t) );

		if (!File.good())
		{
			logger << Log::ERR << "Sector data unreadable(2)! Sector flagged as saved." << Log::ENDL;
			//sector.generated = false;
			return false;
		}

		// if (s->blockpt->special) loadSpecialDataEx(s, File);
		return true;
	}

}
