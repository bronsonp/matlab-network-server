MEXEXT=mexa64 # change this to the output of the Matlab function mexext on your system
MEX=mex -lzmq -largeArrayDims

all: server_communicate.$(MEXEXT) client_communicate.$(MEXEXT)

server_communicate.$(MEXEXT): server_communicate.cpp protocol.h
	$(MEX) server_communicate.cpp

client_communicate.$(MEXEXT): client_communicate.cpp protocol.h
	$(MEX) client_communicate.cpp

clean:
	rm -f server_communicate.$(MEXEXT)
	rm -f client_communicate.$(MEXEXT)
