JOBS?=4

MPCOUTPUTDIR=output

help:

pre:
	@echo "Testing that TTLIBS_ROOT environment variable set to toolchain output"
	test -d "$(TTLIBS_ROOT)"
	@echo "Testing that TEAMTALK_ROOT environment variable is set to repository's"
	@echo "root directory, i.e. source env.sh in root directory"
	test -d "$(TEAMTALK_ROOT)"

ace: pre
	$(eval ACE_ROOT=$(TTLIBS_ROOT)/ACE/share/ace)
	$(eval MPC_ROOT=$(TTLIBS_ROOT)/MPC)
	$(eval TEAMTALKLIB_ROOT=$(TEAMTALK_ROOT)/Library/TeamTalkLib)

iosgen: ace
	export ACE_ROOT=$(ACE_ROOT) MPC_ROOT=$(MPC_ROOT) && \
	$(ACE_ROOT)/bin/mwc.pl -expand_vars -value_project macros+=NDEBUG -type gnuace -into $(MPCOUTPUTDIR)

iosgendbg: ace
	export ACE_ROOT=$(ACE_ROOT) MPC_ROOT=$(MPC_ROOT) && \
	$(ACE_ROOT)/bin/mwc.pl -expand_vars -value_project macros+=_DEBUG -value_project macros+=DEBUG -type gnuace -into $(MPCOUTPUTDIR)

iosbuild: ace
	export ACE_ROOT=$(ACE_ROOT) MPC_ROOT=$(MPC_ROOT) && \
	cd $(MPCOUTPUTDIR) &&
	make -f GNUmakefile.tt5dll_ios depend clean -j$(JOBS) && \
	make -f GNUmakefile.tt5dll_ios -j$(JOBS)

iosprobuild: ace
	export ACE_ROOT=$(ACE_ROOT) MPC_ROOT=$(MPC_ROOT) && \
	cd $(MPCOUTPUTDIR) &&
	make -f GNUmakefile.tt5prodll_ios depend clean -j$(JOBS) && \
	make -f GNUmakefile.tt5prodll_ios -j$(JOBS)

androidgen: iosgen

androiddbggen: iosgendbg

androidbuild: ace
	export ACE_ROOT=$(ACE_ROOT) MPC_ROOT=$(MPC_ROOT) TEAMTALKLIB_ROOT=$(TEAMTALKLIB_ROOT) && \
	cd $(MPCOUTPUTDIR) && \
	make -f GNUmakefile.tt5dll_android depend clean -j$(JOBS) && \
	make -f GNUmakefile.tt5dll_android -j$(JOBS) && \
	make -f GNUmakefile.tt5prodll_android depend clean -j$(JOBS) && \
	make -f GNUmakefile.tt5prodll_android -j$(JOBS)
