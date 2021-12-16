#include <backends/cxxrtl/cxxrtl.h>
#include "build/sim_soc.h"
#include "log.h"

#include <fstream>

namespace cxxrtl_design {

struct wb_mon : public bb_p_wb__mon {
	std::ofstream out;
	void set_output(const std::string &file) {
		out.open(file);
	}
	bool eval() override {
		if (posedge_p_clk()) {
			if (p_stb && p_cyc && p_ack) { // TODO: pipelining
				uint32_t addr = (p_adr.get<uint32_t>() << 2U);
				uint32_t data = p_we ? p_dat__w.get<uint32_t>() : p_dat__r.get<uint32_t>();
				out << stringf("%08x,%c,", addr, p_we ? 'W' : 'R');

				for (int i = 3; i >= 0; i--) {
					if (p_sel.bit(i))
						out << stringf("%02x", (data >> (8 * i)) & 0xFF);
					else
						out << "__";
				}
				out << std::endl;
			}
		}
		return true;
	}
	void reset() override {};
	~wb_mon() {};
};

std::unique_ptr<bb_p_wb__mon> bb_p_wb__mon::create(std::string name, metadata_map parameters, metadata_map attributes) {
	return std::make_unique<wb_mon>();
}

void wb_mon_set_output(bb_p_wb__mon &mon, const std::string &file) {
	dynamic_cast<wb_mon&>(mon).set_output(file);
}

};
