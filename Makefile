SHELL := /usr/bin/bash
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


bin := usr/bin/aria2tray
translations := usr/share/aria2tray/translations
desktop_entry := usr/share/applications/aria2tray.desktop
icon := usr/share/aria2tray/aria2tray.svg
icon_symlink := usr/share/icons/aria2tray.svg

install:
	install -Dm 755 $(LINUX_RELEASEDIR)/src/aria2tray $(DESTDIR)/$(bin)
	install -Dm 644 $(LINUX_RELEASEDIR)/src/translations/* -t $(DESTDIR)/$(translations)
	install -Dm 644 assets/aria2tray.desktop $(DESTDIR)/$(desktop_entry)
	install -Dm 644 assets/icon.svg $(DESTDIR)/$(icon)
	mkdir -p "$(DESTDIR)/usr/share/icons" && ln -s /$(icon) $(DESTDIR)/$(icon_symlink)
	
uninstall:
	rm -f /$(bin) /$(desktop_entry) /$(icon) /$(icon_symlink)
	rm -rf /$(translations)
