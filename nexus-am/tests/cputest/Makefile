include $(AM_HOME)/Makefile.check
.PHONY: all run clean latest $(ALL)

ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

ALL = dummy
ARCH = x86-nemu

all: $(addprefix Makefile., $(ALL))
	@echo "" $(ALL)

#? 这个目标有什么用?
$(ALL): %: Makefile.%

Makefile.%: tests/%.c latest
	@/bin/echo -e "NAME = $*\nSRCS = $<\nLIBS += klib\ninclude $${AM_HOME}/Makefile.app" > $@
	-@make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS)
	# -@rm -f Makefile.$*

# cancel rules included by $(AM_HOME)/Makefile.check
image: ;
default $(MAKECMDGOALS): all ;

clean:
	rm -rf Makefile.* build/

latest:
