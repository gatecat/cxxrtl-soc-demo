#include <backends/cxxrtl/cxxrtl.h>
#include "build/sim_soc.h"

namespace cxxrtl_design {

struct gpio_model : public bb_p_gpio__model {
	void reset() override {};
	~gpio_model() {};
};

std::unique_ptr<bb_p_gpio__model> bb_p_gpio__model::create(std::string name, metadata_map parameters, metadata_map attributes) {
	return std::make_unique<gpio_model>();
}

};
