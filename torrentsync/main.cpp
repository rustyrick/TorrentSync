#include <torrentsync/utils/log/Logger.h>
#include <torrentsync/App.h>

using namespace torrentsync::utils::log;

int main()
{
    Logger::getInstance().addSink(&std::cerr,DEBUG);
    Logger::getInstance().setLogLevel(DEBUG);

    // @TODO parsing the command line arguments and environment variable
    // should let the main function initialize app with the parameters
    // necessary to load the configuration

    torrentsync::App app;
    app.runloop();

    return 0;
}
