BUILD_DIR=./source/build

.PHONY: all z3 distclean clean

all: libz3.a

libz3.a: z3
	ln -s -f $(BUILD_DIR)/libz3.a libz3.a

z3: source/build
	$(MAKE) -C $(BUILD_DIR) $(MAKEFLAGS) libz3.a

source/build: source
	cd source && python scripts/mk_make.py --staticlib

source:
	git clone https://git01.codeplex.com/z3 source -b unstable

distclean:
	rm -rf libz3.a source

clean:
	rm -rf libz3.a $(BUILD_DIR)