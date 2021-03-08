#include "compressor.hpp"

#include <library/log.hpp>
#include <library/compression/lzo.hpp>
#include "sectors.hpp"

#include <cstring>

using namespace library;

namespace cppcraft
{
	/*library::LZO compressor;
	lzo_bytep compressor_databuffer;
	
	struct compressed_datalength_t
	{
		unsigned short lzoSize;
		//unsigned short sectors;
	};*/
	
	void Compressor::init()
	{
		logger << Log::INFO << "* Initializing compressor" << Log::ENDL;
		/*
		const int compressed_max_size = Flatland::FLATLAND_SIZE + sizeof(Sector::sectorblock_t);
		
		// initialize LZO
		compressor.init(compressed_max_size);
		
		// allocate towering buffer
		compressor_databuffer = new lzo_byte[compressed_max_size];
		*/
	}
	void Compressor::cleanup()
	{
		/*
		delete[] compressor_databuffer;
		compressor_databuffer = nullptr;
		*/
	}
	
	void Compressor::load(std::ifstream& File, int PL, int x, int z)
	{
		/*
		// read datalength
		compressed_datalength_t datalength;
		
		File.seekg(PL);
		File.read( (char*) &datalength, sizeof(compressed_datalength_t) );
		
		if (datalength.lzoSize == 0)
		{
			// clear sector at (x, z)
			sectors(x, z).clear();
			
			// exit early
			return;
		}
		
		// go past first struct
		File.seekg(PL+sizeof(compressed_datalength_t));
		// read entire compressed block
		File.read((char*) compressor_databuffer, datalength.lzoSize);
		
		// decompress data
		if (compressor.decompress2a(compressor_databuffer, datalength.lzoSize) == false)
		{
			logger << Log::ERR << "Compressor::decompress(): Failed to decompress data" << Log::ENDL;
			throw std::string("Compressor::decompress(): Failed to decompress data");
		}
		
		lzo_bytep cpos = compressor.getData();
		
		// allocate flatland data
		Flatland::flatland_t* fdata = new Flatland::flatland_t[BLOCKS_XZ * BLOCKS_XZ];
		// copy over flatland struct
		memcpy(fdata, cpos, Flatland::FLATLAND_SIZE);
		// assign it to the flatland container
		sectors.flatland(x, z).assign(fdata);
		
		// move to first sectorblock
		cpos += Flatland::FLATLAND_SIZE;
		
		Sector& base = sectors(x, z);
		base.blockpt = new Sector::sectorblock_t;
		
		memcpy(base.blockpt, cpos, sizeof(Sector::sectorblock_t));
		
		// mark sector as generated
		base.gen_flags = Sector::GENERATED;
		// flag sector for mesh assembly (next stage in pipeline)
		base.updateAllMeshes();
		
		// go to next compressed sector
		//cpos += compressor.getDataLength();
		*/
	} // loadCompressedColumn
	
}
