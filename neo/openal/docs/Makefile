## ------------------------------------------------
## Makefile for openal/docs/


## DocBook stuff
DB2PS  =  db2ps
DB2PDF =  db2pdf
DB2HTML = db2html

## TexInfo stuff
MAKEINFO  = makeinfo



## Targets
OALSPEC = oalspecs
DB_OBJS = $(OALSPEC).ps  $(OALSPEC)/index.html

TEXI_OBJS = openal.info

.SUFFIXES: .texi .info

## Rules
%.ps   : %.sgml
	$(DB2PS) $<

%/index.html : %.sgml
	$(DB2HTML) $<


## Fixme - config.h
%.info : %.texi ../config.h
	$(MAKEINFO) $<



## ----------------
## Standard targets
default: all

all:	render

render: $(DB_OBJS)

texi:   $(TEXI_OBJS)


## --------------
## Cronjob target
update: specs annotated full 


## ----------------------------------------------------------
## Specification - prototypes, tokens, explanation
##
specs-ps:
	-rm -f  $(OALSPEC)-specs.ps
	cp ent-marks-specification.sgml ent-marks.sgml
	$(DB2PS) $(OALSPEC).sgml
	mv $(OALSPEC).ps $(OALSPEC)-specs.ps
	zip -9 $(OALSPEC)-specs.ps.zip $(OALSPEC)-specs.ps

specs-html:
	-rm -rf $(OALSPEC)-specs/
	cp ent-marks-specification.sgml ent-marks.sgml
	$(DB2HTML) $(OALSPEC).sgml
	mv $(OALSPEC)    $(OALSPEC)-specs
	-rm  $(OALSPEC)-specs.html.zip
	zip -r9 $(OALSPEC)-specs.html.zip   $(OALSPEC)-specs/

specs: specs-ps specs-html


## -----------------------------------------------------------
## Annotated Version - including annotation seections
##  (not Programmer's Guide, no examples)
annotated-ps:
	-rm -f  $(OALSPEC)-annote.ps
	cp ent-marks-annotated.sgml ent-marks.sgml
	$(DB2PS) $(OALSPEC).sgml
	mv $(OALSPEC).ps $(OALSPEC)-annote.ps
	zip -9 $(OALSPEC)-annote.ps.zip $(OALSPEC)-annote.ps

annotated-html:
	-rm -rf $(OALSPEC)-annote/
	cp ent-marks-annotated.sgml ent-marks.sgml
	$(DB2HTML) $(OALSPEC).sgml
	mv $(OALSPEC)    $(OALSPEC)-annote
	-rm  $(OALSPEC)-annote.html.zip
	zip -r9  $(OALSPEC)-annote.html.zip $(OALSPEC)-annote/

annotated: annotated-ps annotated-html

## ----------------------------------------------------------
## FULL Version - include's the RFC's and everything else
##
full-ps:
	-rm -f  $(OALSPEC)-full.ps
	cp ent-marks-full.sgml ent-marks.sgml
	$(DB2PS) $(OALSPEC).sgml
	mv $(OALSPEC).ps $(OALSPEC)-full.ps
	zip -9 $(OALSPEC)-full.ps.zip $(OALSPEC)-full.ps

full-html:
	-rm -rf $(OALSPEC)-full/
	cp ent-marks-full.sgml ent-marks.sgml
	$(DB2HTML) $(OALSPEC).sgml
	mv $(OALSPEC)    $(OALSPEC)-full
	-rm  $(OALSPEC)-full.html.zip
	zip -r9 $(OALSPEC)-full.html.zip   $(OALSPEC)-full/

full: full-ps full-html


## ----------------------------------------------------------
## ZIP - create archive
##



clean:  db_clean texi_clean

db_clean:
	-rm -f $(OALSPEC).dvi
	-rm -f $(OALSPEC).aux
	-rm -f $(OALSPEC).log
	-rm -f $(OALSPEC).tex
	-rm -rf $(OALSPEC).junk
	-rm -f \#*\#
	-rm -f *~

texi_clean:
	@echo "Not done."

install:
	@echo "Not done."

distclean: clean
	-rm -rf \
	$(OALSPEC)-full/	\
	$(OALSPEC)-full.ps	\
	$(OALSPEC)-annote/	\
	$(OALSPEC)-annote.ps	\
	$(OALSPEC)-specs/	\
	$(OALSPEC)-specs.ps	\
	*.zip


