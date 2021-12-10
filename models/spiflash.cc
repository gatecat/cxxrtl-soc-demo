#include <backends/cxxrtl/cxxrtl.h>
#include <fstream>
#include <stdexcept>
#include "build/sim_soc.h"
#include "log.h"

namespace cxxrtl_design {

struct spiflash_model : public bb_p_spiflash__model {
	struct {
		int bit_count = 0;
		int byte_count = 0;
		uint32_t addr = 0;
		uint8_t curr_byte = 0;
		uint8_t command = 0;
		uint8_t out_buffer = 0;
	} s, sn;

	std::vector<uint8_t> data;

	spiflash_model() {
		// TODO: don't hardcode
		data.resize(16*1024*1024);
	}

	void load(const std::string &file, size_t offset) {
		std::ifstream in(file, std::ifstream::binary);
		if (offset >= data.size()) {
			throw std::out_of_range("flash: offset beyond end");
		}
		if (!in) {
			throw std::runtime_error("flash: failed to read input file!");
		}
		in.read(reinterpret_cast<char*>(data.data() + offset), (data.size() - offset));
	}

	void process_byte() {
		sn.out_buffer = 0;
		if (sn.byte_count == 0) {
			sn.addr = 0;
			sn.command = sn.curr_byte;
			if (sn.command == 0xab) {
				log("flash: power up\n");
			}
		} else {
			if (sn.command == 0x03) {
				if (sn.byte_count <= 3) {
					sn.addr |= (uint32_t(sn.curr_byte) << ((3 - sn.byte_count) * 8));
				}
				if (sn.byte_count >= 3) {
					if (sn.byte_count == 3)
						log("flash: begin read 0x%06x\n", sn.addr);
					sn.out_buffer = data.at(sn.addr);
					sn.addr = (sn.addr + 1) & 0x00FFFFFF;
				}
			}
		}
	}

	bool eval() override {
		sn = s;
		if (posedge_p_csn__o()) {
			sn.bit_count = 0;
			sn.byte_count = 0;
		} else if (posedge_p_clk__o() && !p_csn__o) {
			sn.curr_byte = (sn.curr_byte << 1U) | p_d__o.bit(0);
			sn.out_buffer = sn.out_buffer << 1U;
			if ((++sn.bit_count) == 8) {
				process_byte();
				++sn.byte_count;
				sn.bit_count = 0;
			}
		} else if (negedge_p_clk__o() && !p_csn__o) {
			p_d__i.set(((sn.out_buffer >> 7U) & 0x1U) << 1U);
		}
		return true;
	}

	bool commit() override {
		bool changed = bb_p_spiflash__model::commit();
		s = sn;
		return changed;
	}

	void reset() override {
	};
	~spiflash_model() {};
};

std::unique_ptr<bb_p_spiflash__model> bb_p_spiflash__model::create(std::string name, metadata_map parameters, metadata_map attributes) {
	return std::make_unique<spiflash_model>();
}

void spiflash_load(bb_p_spiflash__model &flash, const std::string &file, size_t offset) {
	dynamic_cast<spiflash_model&>(flash).load(file, offset);
}

};
