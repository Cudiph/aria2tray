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
	mkdir -p $(DESTDIR)/$(translations) "$(DESTDIR)/usr/bin/" "$(DESTDIR)/usr/share/applications" "$(DESTDIR)/usr/share/icons"
	cp $(LINUX_RELEASEDIR)/src/aria2tray $(DESTDIR)/$(bin)
	cp -r $(LINUX_RELEASEDIR)/src/ $(DESTDIR)/$(translations)
	cp assets/aria2tray.desktop $(DESTDIR)/$(desktop_entry)
	cp assets/icon.svg $(DESTDIR)/$(icon)
	ln -s /$(icon) $(DESTDIR)/$(icon_symlink)
	
uninstall:
	rm -f /$(bin) /$(desktop_entry) /$(icon) /$(icon_symlink)
	rm -rf /$(translations)
