#define CATCH_CONFIG_RUNNER  // Tell Catch2 we provide main
#include <catch2/catch_session.hpp>  // Or the main Catch2 header

#include "cpp-toolbox/logger/thread_logger.hpp"  // Your logger header

int main(int argc, char* argv[])
{
  fprintf(stderr, "[Custom Main] Starting test run...\n");

  Catch::Session session;  // Create Catch2 session

  // Optional: Apply command line arguments, reporters, etc.
  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0)  // Indicates a command line error
    return returnCode;

  // Run the tests
  int numFailed = session.run();

  fprintf(stderr,
          "[Custom Main] Test run finished. Calling logger shutdown...\n");
  toolbox::logger::thread_logger_t::shutdown();  // <-- CALL SHUTDOWN HERE
  fprintf(stderr, "[Custom Main] Logger shutdown called. Exiting.\n");

  // Return the number of failed tests
  return numFailed;
}