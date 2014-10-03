

FILES = \
	src/PrettyFormat\
	src/Var\
	src/Cog\
	src/Pattern\
	src/Function\
	src/JITD\
	src/CGen

LIBS = str

LEXERS = src/JITDLexer
PARSERS = src/JITDParser

AUTOGENFILES = \
	$(patsubst %.mly, %, $(PARSERS))\
	$(patsubst %.mll, %, $(LEXERS))

FILES += $(AUTOGENFILES)
DIRS = $(shell for i in $(FILES) ; do dirname $$i; done | sort | uniq)

OCAML_ARGS += $(patsubst %,%.cma,$(LIBS))

OCAMLC = ocamlc -g $(patsubst %, -I %, $(DIRS)) $(OCAML_ARGS)

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

jitd: $(patsubst %, %.cmo, $(FILES)) src/Driver.ml
	@echo Building $@
	@$(OCAMLC) -o $@ $^

clean: 
	rm -f $(C_FILES)
	rm -f $(patsubst %.cmo,%.cmi,$(C_FILES))
	rm -f $(patsubst %,%.ml,$(AUTOGENFILES)) $(patsubst %,%.mli,$(PARSERS))
	rm -f jitd src/*.{cmo,cmx,cmi,cmxi}
	make -C examples clean
	
test: jitd
	./jitd examples/btree-triggers.ds
	#@make -C examples test

.PHONY: all clean test

