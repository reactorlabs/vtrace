R := R

.PHONY: all build check clean document install uninstall install-devtools

all: clean document build check install

build: document
	$(R) CMD build .

check: build
	$(R) CMD check vtrace*tar.gz

clean:
	rm -f vtrace*tar.gz
	rm -fr vtrace.Rcheck
	rm -rf src/*.o src/*.so

document: install-devtools
	$(R) --quiet -e 'devtools::document()'

install: clean
	$(R) CMD INSTALL .

uninstall:
	$(R) --quiet -e 'remove.packages("vtrace")'

install-devtools:
	$(R) --quiet -e 'if (!require("devtools")) install.packages("devtools", repos="https://cloud.r-project.org")'
