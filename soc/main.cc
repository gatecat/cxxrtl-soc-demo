#include <backends/cxxrtl/cxxrtl.h>
#include "build/sim_soc.h"
#include "models/spiflash.h"
#include "models/log.h"

using namespace cxxrtl_design;

int main(int argc, char **argv) {
	cxxrtl_design::p_sim__top top;

	spiflash_load(*top.cell_p_spiflash_2e_bb, "build/soc/bios/bios.bin", 1*1024*1024);

	top.step();
	auto tick = [&]() {
		top.p_clk.set(false);
		top.step();
		top.p_clk.set(true);
		top.step();
	};
	top.p_rst.set(true);
	tick();
	top.p_rst.set(false);

	while (1) {
		tick();
	}
	return 0;
}
