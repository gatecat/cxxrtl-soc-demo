CXXFLAGS=-O1 -g

all: build/sim_soc

build:
	mkdir -p $@

clean:
	rm -rf build

cores_py=$(wildcard cores/*.py)

build/sim_soc.il: soc/sim_soc.py $(cores_py) build
	python -m soc.sim_soc generate $@

build/sim_blackboxes.v: build/sim_soc.il

%.o: %.cc build/sim_soc.h
	$(CXX) -I . -I $(shell yosys-config --datdir)/include $(CXXFLAGS) -o $@ $<

cores_verilog=$(wildcard cores/*.v)

build/sim_soc.cc: build/sim_soc.il build/sim_blackboxes.v $(cores_verilog)
	yosys -p "write_cxxrtl -g0 -header $@" $^

build/sim_soc.h: build/sim_soc.cc



