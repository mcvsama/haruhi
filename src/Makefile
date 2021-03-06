# vim:ts=4

# Override these variables in Makefile.local.

# Show verbose compilation commands?
VERBOSE			:= 0
# How many threads use for compilation?
THREADS			:= $(shell grep processor /proc/cpuinfo|wc -l)
# Architecture:
ARCH			:= $(shell uname -m)
# Enable profiling?
PROFILING		:= 0
# Debugging enabled?
DEBUG			:= 0
# Where Qt is installed?
QT_PREFIX		:= /usr
QT_INCLUDE_PATH	:= $(QT_PREFIX)/include/qt
QT_MOC_PATH		:= $(QT_PREFIX)/bin/moc
# Profile name:
PROFILE			:=
# Version:
VERSION			:= alpha0.4dev
# Features?
HARUHI_FEATURES	:= HARUHI_IEEE754
# HARUHI_FEATURES can hold following strings:
#   HARUHI_SSE1				- use to force generation of SSE1 instructions.
#   HARUHI_SSE2				- use to force generation of SSE2 instructions.
#   HARUHI_SSE3				- use to force generation of SSE3 instructions.
# Above strings will be added automatically when flags like -msseXX are passed to GCC.
#   HARUHI_IEEE754			- use if compiler uses IEEE-754 compatible CPU, enabled by default.
#   HARUHI_ASSERTS			- enables dynamic assertions
#   HARUHI_ASSERTS_FATAL	- makes assertion failures fatal (self kill).

CXX				= g++
DEPCC			= $(CXX)
CXXFLAGS		+= -fPIC -O4 -g -c -std=c++17 -Wall -Wall -Wextra -Wunused  -Wunused-function -Wunused-label -Wnoexcept -fstrict-aliasing -Wstrict-aliasing=3 -fnothrow-opt -pedantic-errors -Wshadow-compatible-local
CXXFLAGS		+= -pthread -rdynamic -I.
CXXFLAGS		+= -finline -march=native -mtune=native -falign-functions=4 -mpreferred-stack-boundary=4 -funroll-loops -mfpmath=sse,387 -msse4.1 -fomit-frame-pointer
CXXFLAGS		+= -I$(QT_INCLUDE_PATH)
SO				= $(CXX) -shared
SOFLAGS			=
LD				= $(CXX)
LDFLAGS			= -rdynamic
AR				= ar
MOC				= $(QT_MOC_PATH)
LIBS			+= m dl pthread boost_system
PKGCONFIGS		+= jack alsa fftw3 uuid Qt5Core Qt5Gui Qt5Xml Qt5Svg
CXXFLAGS_s		:= $(CXXFLAGS)
ifeq ($(OPTIMIZE),debug)
HARUHI_FEATURES	+= HARUHI_ASSERTS
HARUHI_FEATURES	+= HARUHI_ASSERTS_FATAL
CXXFLAGS		+= -O2 -ggdb3 -fno-inline -fno-eliminate-unused-debug-types -D_GLIBCXX_DEBUG
else
CXXFLAGS		+= -DQT_NO_DEBUG
endif
CXXFLAGS		+= -DCXXFLAGS='"$(CXXFLAGS_s)"' $(foreach feature, $(HARUHI_FEATURES), -D$(feature))
LDFLAGS			+= $(foreach lib, $(LIBS), -l$(lib))
LDFLAGS			+= $(shell pkg-config --libs $(PKGCONFIGS))
CXXFLAGS		+= $(shell pkg-config --cflags $(PKGCONFIGS))

.PHONY: all dep help clean distclean release

HEADERS =
SOURCES =
OBJECTS =
NODEP_OBJECTS =
MOCSRCS =
TARGETS =

#### Helpers ####

ifeq ($(shell tput colors), -1)
	_s = ""
	_l = ""
else
	_s = "[1;34m"
	_l = "[0m"
endif

#### Makefiles ####

-include Makefile.local

ifneq ($(VERBOSE),1)
MAKEFLAGS += --silent
endif

ifneq ($(THREADS),)
MAKEFLAGS += -j$(THREADS)
endif

ifeq ($(PROFILING),1)
CXXFLAGS += -pg -fno-omit-frame-pointer
LDFLAGS += -pg
endif

ifeq ($(PROFILING),1)
PROFILING_s = -profiling
else
PROFILING_s =
endif

ifeq ($(DEBUG),1)
DEBUG_s = -debug
else
DEBUG_s =
endif

ifneq ($(PROFILE),)
PROFILE_s = -$(PROFILE)
else
PROFILE_s =
endif

dist			= $(ARCH)$(PROFILING_s)$(DEBUG_s)$(PROFILE_s)
distdir			:= build/$(dist)
depsdir			= $(distdir)/deps
objsdir			= $(distdir)/objects
mocsdir			= $(distdir)/mocs
reldir			= release/$(dist)
relname			= haruhi-$(VERSION)-$(dist)
reldir_pkg		= $(reldir)/$(relname)
prepdir			= $(shell mkdir -p "$$(dirname "$(strip $(1))")")
mkmocs			= $(foreach file, $(1), $(mocsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).moc.cc)
mkdeps			= $(foreach file, $(1), $(depsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).dep)
mkobjs			= $(foreach file, $(1), $(objsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)
mkmocobjs		= $(foreach file, $(1), $(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)

include Makefile.sources

#### Commit ID support ####

ifneq ($(VERSION_FILE),)
GIT_AVAILABLE := $(shell which git 1>/dev/null && echo OK)
VERSION_FILE_OBJECT := $(call mkobjs, $(VERSION_FILE))

ifeq ($(GIT_AVAILABLE),OK)
COMMIT			= $(shell cat .commit 2>/dev/null)
COMMIT_REAL		= $(shell git rev-parse HEAD 2>/dev/null)
BRANCH			= $(shell cat .branch 2>/dev/null)
BRANCH_REAL		= $(shell git branch --no-color 2>/dev/null | sed -e '/^[^*]/d' -e 's/* \(.*\)/\1/')

ifeq ($(COMMIT),$(COMMIT_REAL))
COMMIT_FILE		=
else
COMMIT_FILE		= .commit
endif

ifeq ($(BRANCH),$(BRANCH_REAL))
BRANCH_FILE		=
else
BRANCH_FILE		= .branch
endif

.PHONY: .commit .branch

.commit:
	@if [ "$(COMMIT)" != "$(COMMIT_REAL)" ]; then \
		echo " Saving commit ID"; \
		echo "$(COMMIT_REAL)" >$@; \
	 fi

.branch:
	@if [ "$(BRANCH)" != "$(BRANCH_REAL)" ]; then \
		echo " Saving branch name"; \
		echo "$(BRANCH_REAL)" >$@; \
	 fi

$(VERSION_FILE): $(COMMIT_FILE) $(BRANCH_FILE)
	@echo "// Do not edit - automatically generated." >$@
	@echo "namespace Haruhi { namespace Version {" >>$@
	@echo "	const char* commit = \"$(COMMIT_REAL)\";" >>$@
	@echo "	const char* branch = \"$(BRANCH_REAL)\";" >>$@
	@echo "	const char* version = \"$(VERSION)\";" >>$@
	@echo "} }" >>$@

$(VERSION_FILE_OBJECT): $(VERSION_FILE)
endif # GIT_AVAILABLE == OK
endif # VERSION_FILE != ""

#### Rules ####

DEPFILES := $(call mkdeps, $(subst $(VERSION_FILE),,$(SOURCES)))
MAINDEPFILE := $(depsdir)/Makefile.dep

all: $(MAINDEPFILE) $(DEPFILES) $(TARGETS)

dep: $(DEPFILES)

help:
	@echo 'Available targets:'
	@echo '  all        Compiles program.'
	@echo '  dep        Generates dependencies.'
	@echo '  clean      Cleans source tree and dep files'
	@echo '  distclean  Cleans dist directory.'
	@echo '  release    Creates release.'
	@echo '  help       Shows this help.'

clean:
	@rm -f $(MOCSRCS)
	@rm -f $(OBJECTS)
	@rm -f $(MOCOBJS)
	@rm -f $(DEPFILES)
	@rm -f $(MAINDEPFILE)

release:
	@echo Making release: $(dist)
	@make clean distclean all
	@rm -rf $(reldir)
	@mkdir -p $(reldir_pkg)
	@mkdir -p $(reldir_pkg)/share
	@cp -Ra ../share/config $(reldir_pkg)/share/
	@mkdir -p $(reldir_pkg)/share/images/16
	@mkdir -p $(reldir_pkg)/share/images/22
	@cp -Ra ../share/images/16/*.png $(reldir_pkg)/share/images/16/
	@cp -Ra ../share/images/22/*.png $(reldir_pkg)/share/images/22/
	@cp -Ra ../share/images/*.png $(reldir_pkg)/share/images/
	@cp $(distdir)/haruhi.run $(reldir_pkg)/haruhi
ifneq ($(DEBUG),1)
	@strip $(reldir_pkg)/haruhi
endif
	@cd $(reldir) && tar zcf $(relname).tar.gz $(relname) && cd -

$(MAINDEPFILE): Makefile.sources $(DEPFILES)
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	@rm -f $@
	@echo "# DO NOT EDIT!!! (all changed will be lost!)" >> $@
	@echo "# This file has been generated automatically by Makefile from source files." >> $@
	@for file in $(DEPFILES); do \
		cat "$$file" >> $@; \
	 done;
	@for file in $(MOCSRCS); do \
		echo "$$(dirname "$$file")/$$(basename "$$file" .cc).o: $$file" >> $@; \
	 done;

$(depsdir)/%.dep: %.cc %.h
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(objsdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(depsdir)/%.dep: %.cc
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(objsdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(depsdir)/%.dep: %.h
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(objsdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(depsdir)/%.moc.dep: %.moc.cc
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

%.o:
	$(call prepdir, $@)
	@echo $(_s) "C++     " $(_l) $@
	$(CXX) $(CXXFLAGS) $< -o $@

%.a:
	$(call prepdir, $@)
	@echo $(_s) "AR      " $(_l) $@
	@rm -f $@
	$(AR) cr $@ $^

%.so:
	$(call prepdir, $@)
	@echo $(_s) "LD [M]  " $(_l) $@
	$(SO) $(SOFLAGS) $^ -o $@

$(mocsdir)/%.moc.cc: %.h
	$(call prepdir, $@)
	@echo $(_s) "MOC     " $(_l) $@
	$(MOC) $< -o $@

$(LINKEDS):
	$(call prepdir, $@)
	@echo $(_s) "LD      " $(_l) $@
	$(LD) -o $@ $(NODEP_OBJECTS) $(OBJECTS) $(MOCOBJS) $(LDFLAGS)

-include $(MAINDEPFILE)

