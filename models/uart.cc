#include <backends/cxxrtl/cxxrtl.h>
#include "build/sim_soc.h"

namespace cxxrtl_design {

struct uart_model : public bb_p_uart__model {
	void reset() override {};
	~uart_model() {};
};

std::unique_ptr<bb_p_uart__model> bb_p_uart__model::create(std::string name, metadata_map parameters, metadata_map attributes) {
	return std::make_unique<uart_model>();
}

};
