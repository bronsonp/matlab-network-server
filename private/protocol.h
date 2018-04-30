#pragma once

////////////////////////////////////////////////////////////
// Mode constants for server
#define SERVER_INIT 0
#define SERVER_RECV 1
#define SERVER_SEND 2

////////////////////////////////////////////////////////////
// Mode constants for client
#define CLIENT_INIT 0
#define CLIENT_REQUEST 1

////////////////////////////////////////////////////////////
// Helper function for fetching fields from Matlab structures
template<class type>
type getConfigField(const mxArray *structure, const char *fieldname, const char *type_name)
{
	// Check that structure is what it should be
	if (!mxIsStruct(structure)) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "2nd argument must be a config structure.");
	}

	// Retrieve the field
	const mxArray *field = mxGetField(structure, 0, fieldname);

	// Check that the field exists
	if (field == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Missing config field: %s", fieldname);
	}

	// Check the type
	if ( (mxGetM(field) != 1)
	     || (mxGetN(field) != 1)
	     || !mxIsClass(field, type_name) ) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Expected config field %s to have type: %s", fieldname, type_name);
	}

	// Get the value
	type *pvalue = (type *)mxGetData(field);
	if (pvalue == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Failed to retrieve data for %s", fieldname);
	}

	// Return iy
	return *pvalue;
}

////////////////////////////////////////////////////////////
// Helper function for fetching strings from Matlab structures
// You must free the return value, with mxFree
char *getConfigString(const mxArray *structure, const char *fieldname)
{
	// Check that structure is what it should be
	if (!mxIsStruct(structure)) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "2nd argument must be a config structure.");
	}

	// Retrieve the field
	const mxArray *field = mxGetField(structure, 0, fieldname);

	// Check that the field exists
	if (field == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Missing config field: %s", fieldname);
	}

	// Check the type
	if ( !mxIsClass(field, "char") ) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Expected config field %s to be a string", fieldname);
	}

	// Get the value
	char *mxString = mxArrayToString(field);
	if (mxString == NULL) {
		mexErrMsgIdAndTxt( "MATLAB:zmq_communicate:invalidInputs",
		                   "Failed to retrieve data for %s", fieldname);
	}

	// Copy out of Matlab's memory (in case the underlying data is freed by Matlab)
	char *value = new char [1+strlen(mxString)];
	strcpy(value, mxString);

	// Return it
	return value;
}
