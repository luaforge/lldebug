
SUBDIRS = build

EXTRA_DIST = \
	build-scripts/config.guess \
	build-scripts/config.sub \
	build-scripts/install-sh \
	build-scripts/depcomp \
	build-scripts/missing

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck


