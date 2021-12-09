#include <backends/cxxrtl/cxxrtl.h>
#include "build/sim_soc.h"
#include "log.h"

namespace cxxrtl_design {

struct spiflash_model : public bb_p_spiflash__model {
	struct {
		int bit_count = 0;
		int byte_count = 0;
		uint8_t curr_byte = 0;
	} state, state_next;

	bool eval() override {
		state_next = state;
		if (posedge_p_csn__o()) {
			state_next.bit_count = 0;
			state_next.byte_count = 0;
			log("SPI: end\n");
		} else if (posedge_p_clk__o() && !p_csn__o) {
			state_next.curr_byte = (state_next.curr_byte << 1) | p_d__o.bit(0);
			if ((++state_next.bit_count) == 8) {
				log("SPI: %02x\n", state_next.curr_byte);
				++state_next.byte_count;
				state_next.bit_count = 0;
			}
		}
		return true;
	}

	bool commit() override {
		bool changed = bb_p_spiflash__model::commit();
		state = state_next;
		return changed;
	}

	void reset() override {};
	~spiflash_model() {};
};

std::unique_ptr<bb_p_spiflash__model> bb_p_spiflash__model::create(std::string name, metadata_map parameters, metadata_map attributes) {
	return std::make_unique<spiflash_model>();
}

};
