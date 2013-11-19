#include <mex.h>
#include <stdio.h>
#include <zmq.h>
#include <string.h>
#include <assert.h>
#include "protocol.h"

////////////////////////////////////////////////////////////
// Global variables
void *context;
void *zsocket;
uint32_T socket_timeout;
zmq_pollitem_t poll_items [1];
char *endpoint;

////////////////////////////////////////////////////////////
// Prototypes
static void close_socket();
bool client_init(const mxArray *config);
bool client_request(const mxArray *msg_array);

////////////////////////////////////////////////////////////
// Initialise the client
bool client_init(const mxArray *config)
{
	if (context != NULL) {
		// We have already been initialised
		printf("Client: INIT called again\n");
		close_socket();
	}

	// Initialise ZMQ
	context = zmq_ctx_new();
	zsocket = zmq_socket(context, ZMQ_REQ);
	if (zsocket == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:failure",
		                   "Failed to create ZMQ socket:\n%s", zmq_strerror(errno));
	}
	mexAtExit(close_socket);
	int linger = 0;
	zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));

	// Load config settings
	socket_timeout = getConfigField<uint32_T>(config, "timeout", "uint32");

	// Connect the socket
	endpoint = getConfigString(config, "endpoint");
	printf("Opening ZMQ socket %s\n", endpoint);
	int rc = zmq_connect(zsocket, endpoint);
	if (rc != 0) {
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:failure",
		                   "Failed to connect ZMQ socket:\n%s", zmq_strerror(errno));
	}
	poll_items[0].socket = zsocket;

	return true;
}

////////////////////////////////////////////////////////////
// Send a request and receive the response
bool client_request(const mxArray *req_array, mxArray *resp_array)
{
	int rc, nevents;

	if (context == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:need_init",
		                   "REQUEST was called before INIT");
	}

	// Obtain the data to be sent
	if (!mxIsClass(req_array, "uint8")) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Message must have type uint8");
	}
	uint8_T *req_data = (uint8_T*) mxGetData(req_array);
	mwSize req_size = mxGetNumberOfElements(req_array);

	// Send the message
	//printf("Sending request to server ...\n");
	poll_items[0].events = ZMQ_POLLOUT;
	nevents = zmq_poll(poll_items, 1, socket_timeout);
	if (nevents == -1) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:error",
		                   "Failed to poll socket: %s", zmq_strerror(errno));
	} else if (nevents == 0) {
		// Timeout
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:timeout",
		                   "Timeout waiting for socket to be ready.");
	} else {
		rc = zmq_send(zsocket, req_data, req_size*sizeof(uint8_T), 0);
		//printf("Sent %i bytes.\n", rc);
		if (rc != req_size*sizeof(uint8_T)) {
			mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:error",
			                   "Failed to send message: %s", zmq_strerror(errno));
		}
	}

	// Wait for the response
	//printf("Waiting for response ...\n");
	poll_items[0].events = ZMQ_POLLIN;
	nevents = zmq_poll(poll_items, 1, socket_timeout);
	if (nevents == -1) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:error",
		                   "Failed to poll socket: %s", zmq_strerror(errno));
	} else if (nevents == 0) {
		// Timeout

		// The old socket is confused. Need to tear it down and open a new one.
		zmq_close(zsocket);

		zsocket = zmq_socket(context, ZMQ_REQ);
		int linger = 0;
		zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));
		if (zsocket == NULL) {
			mexErrMsgIdAndTxt( "MATLAB:client_communicate:failure",
			                   "Failed to create ZMQ socket:\n%s", zmq_strerror(errno));
		}

		int rc = zmq_connect(zsocket, endpoint);
		if (rc != 0) {
			mexErrMsgIdAndTxt( "MATLAB:client_communicate:failure",
			                   "Failed to connect ZMQ socket:\n%s", zmq_strerror(errno));
		}
		poll_items[0].socket = zsocket;

		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:timeout",
		                   "Timeout waiting for job server to respond.");
	} else {
		// Initialise a zmq_msg_t structure to receive the message
		zmq_msg_t resp_zmsg;
		rc = zmq_msg_init(&resp_zmsg);
		assert(rc == 0);

		// Receive the message
		int resp_size = zmq_msg_recv(&resp_zmsg, zsocket, 0);
		assert(resp_size > 0);
		//printf("Client received %i bytes\n", resp_size);
		uint8_T *resp_data = (uint8_T *)zmq_msg_data(&resp_zmsg);

		// Allocate memory on the MATLAB side
		uint8_T *resp_matlab_data = (uint8_T *)mxCalloc(resp_size, sizeof(uint8_T));

		// Copy the memory
		memcpy(resp_matlab_data, resp_data, resp_size*sizeof(uint8_T));

		// Link the MATLAB memory to the return value
		mxSetData(resp_array, resp_matlab_data);
		mxSetM(resp_array, 1);
		mxSetN(resp_array, resp_size);

		// Free the message
		zmq_msg_close(&resp_zmsg);

		return true;
	}
}

////////////////////////////////////////////////////////////
// Close the socket
static void close_socket()
{
	printf("client: closing ZMQ socket\n");

	if (endpoint != NULL) {
		mxFree(endpoint);
	}

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
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:invalidNumInputs",
		                   "Usage: [has_msg, msg] = client_communicate(mode, data)");
	}
	if (nlhs != 2) {
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:invalidNumOutputs",
		                   "Need two return values.\n"
		                   "Usage: [has_msg, msg] = client_communicate(mode, data)");
	}

	// Check the mode
	if ( (mxGetM(prhs[0]) != 1)
	     || (mxGetN(prhs[0]) != 1)
	     || !mxIsClass(prhs[0], "uint32") ) {
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:invalidInputs",
		                   "Usage: client_communicate(mode, data)\n"
		                   "Mode argument must be uint32");
	}

	// Initialise the data return value
	plhs[1] = mxCreateNumericMatrix(0, 0, mxUINT8_CLASS, mxREAL);

	// Check the mode, and run the appropriate function
	uint32_T *mode = (uint32_T *)mxGetData(prhs[0]);
	bool success;

	switch (*mode) {
	case CLIENT_INIT:
		success = client_init(prhs[1]);
		break;
	case CLIENT_REQUEST:
		success = client_request(prhs[1], plhs[1]);
		break;
	default:
		mexErrMsgIdAndTxt( "MATLAB:client_communicate:invalidInputs",
		                   "Usage: client_communicate(mode, data)\n"
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
