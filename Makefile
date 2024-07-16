BUILDDIR := build
RELEASEDIR := dist
LINUX_BUILDDIR := $(BUILDDIR)/linux
LINUX_RELEASEDIR := $(RELEASEDIR)/linux

.PHONY: debug release clean format

debug:
	@if [[ ! -d "$(LINUX_BUILDDIR)" ]]; then meson setup $(LINUX_BUILDDIR); fi
	@ninja -C $(LINUX_BUILDDIR)

release:
	@if [[ ! -d "$(LINUX_RELEASEDIR)" ]]; then meson setup $(LINUX_RELEASEDIR) --buildtype=release; fi
	@ninja -C $(LINUX_RELEASEDIR)

clean:
	rm -rf $(LINUX_BUILDDIR)

format:
	clang-format src/*.cpp src/*.h -i --verbose
