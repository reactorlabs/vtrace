R := R

SOURCEDIR := src
SOURCES := $(shell find $(SOURCEDIR) -name '*.cpp')

.PHONY: all build check clean install uninstall document lintr \
	install-devtools install-lintr

all: clean document build check install

build: document
	$(R) CMD build .

check: build
	$(R) CMD check vtrace*tar.gz

clean:
	rm -f vtrace*tar.gz
	rm -fr vtrace.Rcheck
	rm -rf src/*.o src/*.so

install: clean
	$(R) CMD INSTALL .

uninstall:
	$(R) --quiet -e 'remove.packages("vtrace")'

document: install-devtools
	$(R) --quiet -e 'devtools::document()'

lintr: install-lintr
	$(R) --quiet -e 'quit(status = length(print(lintr::lint_package())) != 0)'

install-devtools:
	$(R) --quiet -e 'if (!require("devtools")) install.packages("devtools", repos="https://cloud.r-project.org")'

install-lintr:
	$(R) --quiet -e 'if (!require("lintr")) install.packages("lintr", repos="https://cloud.r-project.org")'
