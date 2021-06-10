#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ANDROID_LOGWRITE

#include <catch2/catch.hpp>
#include <QtGui/QGuiApplication>

#include <QtDebug>
#include <QTimer>

int main(int argc, char** argv)
{
    // global setup...
    QGuiApplication app(argc, argv); //Needed for QT on Android

    qInfo() << "Catch2 Application wrapper started ";


    Catch::Session session; // There must be exactly one instance

    // writing to session.configData() here sets defaults
    // this is the preferred way to set them

    int returnCode = session.applyCommandLine( argc, argv );
    if( returnCode != 0 ) // Indicates a command line error
    {
        qCritical() << "Catch2 initialization from commandline arguments failed";
        return returnCode;
    }

    // writing to session.configData() or session.Config() here
    // overrides command line args
    // only do this if you know you need to

    // numFailed is clamped to 255 as some unices only use the lower 8 bits.
    // This clamping has already been applied, so just return it here
    // You can also do any post run clean-up here
    int numFailed = session.run();
    qInfo() << "Catch2 finished running. Failed tests: " << numFailed;


    //return app.exec(); // This keeps the UI running intil explicitly closed. Not needed, but might be useful
    return numFailed;


}
