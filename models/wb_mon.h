#ifndef WB_MON_H
#define WB_MON_H

#include <backends/cxxrtl/cxxrtl.h>

#include "build/sim_soc.h"

namespace cxxrtl_design {
	void wb_mon_set_output(bb_p_wb__mon &mon, const std::string &file);
}

#endif
