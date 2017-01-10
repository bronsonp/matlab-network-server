# Check that we have been started from the correct directory
if (Test-Path "+netsrv") {
  # Run the server (in the background)
  matlab -nosplash -nodisplay -nodesktop -logfile netsrv_test_server.txt -r 'netsrv.unit_test.run_server;exit'
  # Run the client but don't return until it finishes running
  matlab -nosplash -nodisplay -nodesktop -logfile netsrv_test_client.txt -wait -r 'netsrv.unit_test.run_client;exit'
  # Display the output and clean up
  type netsrv_test_server.txt
  type netsrv_test_client.txt
  rm netsrv_test_server.txt
  rm netsrv_test_client.txt
} else {
  Write-Error "Run this script from the top level of the project where the +netsrv directory is."
}
