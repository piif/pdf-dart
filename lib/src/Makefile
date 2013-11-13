DSTDIR=..
DSTLIB=${DSTDIR}/libpdflib.so

all: ${DSTLIB}

##PDFLIB_DIR=/usr/local/PDFlib-9.0.1-Linux-x86_64-C-C++/bind/c
PDFLIB_DIR=/usr/local

CFLAGS=-I${PDFLIB_DIR}/include -I/usr/local/dart/dart-sdk
LDFLAGS=-L${PDFLIB_DIR}/lib64 -lpdf -lm

${DSTLIB}: pdflib.o
	gcc -shared -Wl,-soname,libpdflib.so -o ${DSTLIB} pdflib.o ${LDFLAGS}

pdflib.o: pdflib.cpp dartNativeHelpers.h
	g++ -g -Wall -fPIC ${CFLAGS} -c pdflib.cpp

clean:
	rm *.o ${DSTLIB}
