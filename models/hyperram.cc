#include <backends/cxxrtl/cxxrtl.h>
#include "build/sim_soc.h"

namespace cxxrtl_design {

struct hyperram_model : public bb_p_hyperram__model {
	void reset() override {};
	~hyperram_model() {};
};

std::unique_ptr<bb_p_hyperram__model> bb_p_hyperram__model::create(std::string name, metadata_map parameters, metadata_map attributes) {
	return std::make_unique<hyperram_model>();
}

};
