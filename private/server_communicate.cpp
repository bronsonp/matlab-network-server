#include <mex.h>
#include <stdio.h>
#include <zmq.h>
#include <string.h>
#include <assert.h>
#include "protocol.h"

////////////////////////////////////////////////////////////
// Global variables
uint16_T socket_port;
uint32_T socket_timeout;
void *context;
void *zsocket;
zmq_pollitem_t poll_items [1];

////////////////////////////////////////////////////////////
// Prototypes
static void close_socket();
bool server_init(const mxArray *config);
bool server_recv(const mxArray *config, mxArray *msg_array);


////////////////////////////////////////////////////////////
// Initialise the server
bool server_init(const mxArray *config)
{
	printf("SERVER INIT\n");

	if (context != NULL) {
		// We have already been initialised
		printf("Server: INIT called again\n");
		close_socket();
	}

	// Load config settings
	socket_port = getConfigField<uint16_T>(config, "port", "uint16");
	socket_timeout = getConfigField<uint32_T>(config, "timeout", "uint32");

	// Initialise ZMQ
	context = zmq_ctx_new();
	zsocket = zmq_socket(context, ZMQ_REP);
	if (zsocket == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:failure",
		                   "Failed to create ZMQ socket:\n%s", zmq_strerror(errno));
	}
	mexAtExit(close_socket);
	int linger = 0;
	zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));

	// Bind the socket
	char endpoint [30];
	snprintf(endpoint, 30, "tcp://*:%i", socket_port);
	int rc = zmq_bind(zsocket, endpoint);
	if (rc != 0) {
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:failure",
		                   "Failed to bind ZMQ socket:\n%s", zmq_strerror(errno));
	}

	// Prepare the poll items (for the RECV call)
	poll_items[0].socket = zsocket;
	poll_items[0].events = ZMQ_POLLIN;

	return true;
}

////////////////////////////////////////////////////////////
// Receive on the socket, or timeout
bool server_recv(const mxArray *config, mxArray *msg_array)
{
	int rc;

	if (context == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:failure",
		                   "RECV called before INIT");
	}

	// Initialise a zmq_msg_t structure to receive the message
	zmq_msg_t zmsg;
	rc = zmq_msg_init(&zmsg);
	assert(rc == 0);

	// Attempt to receive a message
	int num_bytes = zmq_msg_recv(&zmsg, zsocket, ZMQ_DONTWAIT);
	if (num_bytes == -1) {
		// Did not receive a message ...
		if (errno == EAGAIN) {
			// ... because none were available.
			// Wait for a message to be available.
			int nevents = zmq_poll(poll_items, 1, socket_timeout);
			if (nevents == -1) {
				zmq_msg_close(&zmsg);
				mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:error",
				                   "Failed to poll socket: %s", zmq_strerror(errno));
			} else if (nevents == 0) {
				// Timeout.
				zmq_msg_close(&zmsg);
				return false;
			} else {
				// A message is available
				num_bytes = zmq_msg_recv(&zmsg, zsocket, 0);
			}
		} else {
			// ... because polling failed
			zmq_msg_close(&zmsg);
			mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:error",
			                   "Failed to poll socket: %s", zmq_strerror(errno));
		}
	}
	//printf("Server received %i bytes\n", num_bytes);
	uint8_T *msg_zmq_data = (uint8_T *)zmq_msg_data(&zmsg);

	// Allocate memory on the MATLAB side
	uint8_T *msg_matlab_data = (uint8_T *)mxCalloc(num_bytes, sizeof(uint8_T));

	// Copy the memory
	memcpy(msg_matlab_data, msg_zmq_data, num_bytes);

	// Link the MATLAB memory to the return value
	mxSetData(msg_array, msg_matlab_data);
	mxSetM(msg_array, 1);
	mxSetN(msg_array, num_bytes);

	// Free the message
	zmq_msg_close(&zmsg);

	return true;
}

////////////////////////////////////////////////////////////
// Send on the socket
bool server_send(const mxArray *data)
{
	int rc;

	uint8_T *req_data = (uint8_T*) mxGetData(data);
	mwSize req_size = mxGetNumberOfElements(data);

	rc = zmq_send(zsocket, req_data, req_size*sizeof(uint8_T), 0);
	//printf("Sent %i bytes.\n", rc);
	if (rc != req_size*sizeof(uint8_T)) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:error",
		                   "Failed to send message: %s", zmq_strerror(errno));
	}

	return true;
}

////////////////////////////////////////////////////////////
// Close the socket
static void close_socket()
{
	printf("server: closing ZMQ socket\n");

	zmq_close(zsocket);
	zsocket = NULL;

	zmq_ctx_destroy(context);
	context = NULL;
}

////////////////////////////////////////////////////////////
// MATLAB mex function wrapper
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
	// Check for the proper number of arguments
	if (nrhs != 2) {
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:invalidNumInputs",
		                   "Usage: [has_msg, msg] = server_communicate(mode, data)");
	}
	if (nlhs != 2) {
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:invalidNumOutputs",
		                   "Need two return values.\n"
		                   "Usage: [has_msg, msg] = server_communicate(mode, data)");
	}

	// Check the mode
	if ( (mxGetM(prhs[0]) != 1)
	     || (mxGetN(prhs[0]) != 1)
	     || !mxIsClass(prhs[0], "uint32") ) {
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:invalidInputs",
		                   "Usage: server_communicate(mode, data)\n"
		                   "Mode argument must be uint32");
	}

	// Initialise the data return value
	plhs[1] = mxCreateNumericMatrix(0, 0, mxUINT8_CLASS, mxREAL);

	// Check the mode, and run the appropriate function
	uint32_T *mode = (uint32_T *)mxGetData(prhs[0]);
	bool success;

	switch (*mode) {
	case SERVER_INIT:
		success = server_init(prhs[1]);
		break;
	case SERVER_RECV:
		success = server_recv(prhs[1], plhs[1]);
		break;
	case SERVER_SEND:
		success = server_send(prhs[1]);
		break;
	default:
		mexErrMsgIdAndTxt( "MATLAB:server_communicate:invalidInputs",
		                   "Usage: server_communicate(mode, data)\n"
		                   "Invalid mode argument");

	}

	// Set the success flag
	mxLogical *successData = (mxLogical *)mxCalloc(1, sizeof(mxLogical));
	*successData = success;
	plhs[0] = mxCreateNumericMatrix(0, 0, mxLOGICAL_CLASS, mxREAL);
	mxSetData(plhs[0], successData);
	mxSetM(plhs[0], 1);
	mxSetN(plhs[0], 1);

}
