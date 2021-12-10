#ifndef SPIFLASH_H
#define SPIFLASH_H

#include <backends/cxxrtl/cxxrtl.h>

#include "build/sim_soc.h"

namespace cxxrtl_design {
	void spiflash_load(bb_p_spiflash__model &flash, const std::string &file, size_t offset);
}

#endif
