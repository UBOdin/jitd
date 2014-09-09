

FILES = \
	src/Datastructure

LEXERS = src/JITDLexer
PARSERS = src/JITDParser

AUTOGENFILES = \
	$(patsubst %.mly, %, $(PARSERS))\
	$(patsubst %.mll, %, $(LEXERS))

FILES += $(AUTOGENFILES)
DIRS = $(shell for i in $(FILES) ; do dirname $$i; done | sort | uniq)

OCAMLC = ocamlc $(patsubst %, -I %, $(DIRS))

C_FILES = $(patsubst %, %.cmo, $(FILES))

all: jitd

$(patsubst %, %.ml,$(LEXERS)) : %.ml : %.mll
	@echo Building Lexer $(*)
	@ocamllex $< 

$(patsubst %, %.ml,$(PARSERS)) : %.ml : %.mly
	@echo Building Parser $(*)
	@ocamlyacc $<
  
$(C_FILES) : %.cmo : %.ml
	@if [ -f $(*).mli ] ; then \
		echo Compiling Header $(*);\
		$(OCAMLC) -c $(*).mli;\
	fi
	@echo Compiling $(*)
	@$(OCAMLC) -c $<

jitd: $(patsubst %, %.cmo, $(FILES))
	@echo Building $@
	@$(OCAMLC) -o $@ $^

clean: 
	rm -f $(C_FILES)
	rm -f $(patsubst %.cmo,%.cmi,$(C_FILES))
	rm -f $(patsubst %,%.ml,$(AUTOGENFILES)) $(patsubst %,%.mli,$(PARSERS))
	rm -f jitd
	

.PHONY: all clean