BUILDDIR := build
RELEASEDIR := dist
LINUX_BUILDDIR := $(BUILDDIR)/linux
LINUX_RELEASEDIR := $(RELEASEDIR)/linux

.PHONY: debug release clean format install uninstall

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


bin := /usr/bin/aria2tray
translations := /usr/share/aria2tray/translations
desktop_entry := /usr/share/applications/aria2tray.desktop
icon := /usr/share/icons/aria2tray.svg

install:
	mkdir -p $(DESTDIR)/$(translations)
	cp $(LINUX_RELEASEDIR)/src/aria2tray $(DESTDIR)/$(bin)
	cp -r $(LINUX_RELEASEDIR)/src/translations $(DESTDIR)/$(translations)
	cp assets/aria2tray.desktop $(DESTDIR)/$(desktop_entry)
	cp assets/icon.svg $(DESTDIR)/$(icon)
	
uninstall:
	rm -f $(bin) $(desktop_entry) $(icon) 
	rm -rf $(translations)
