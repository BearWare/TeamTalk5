#include <catch2/catch_all.hpp>

#include "test/TTUnitTest.h"

//#include <QtGui/QGuiApplication> //If we ever want to include a GUI
#include <QtDebug>

#if defined(__ANDROID__)

#include <QtAndroid>
#include <QDir> // for file copy from APK
#include <QDirIterator> // for file copy from APK
#include <QStandardPaths> // for file copy from APK


/*  The struct below is registered as a listener for Catch2 events on Android
 *  This is needed because Catch logs to stdout/stderr and on Android that output is redirected to /dev/null
 *  As a result, you don't see the typically output of failed assertions, etc you'd get on Linux or Windows
 *  This listener is a poor-mans solution to that and logs the output via QT logging
 *  This output is visible in the QT Creator Application output when run from QT Creator
 */

Catch::TestCaseInfo* currentTestCase;

struct MyListener : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor

    // The whole test run, starting and ending
    void testRunStarting( Catch::TestRunInfo const& testRunInfo ) override
    {
        Q_UNUSED(testRunInfo)
        //qInfo() << "Start of test run " << QString::fromStdString(testRunInfo.name);

    }

    // Test cases starting and ending
    void testCaseStarting( Catch::TestCaseInfo const& testInfo ) override
    {
        Q_UNUSED(testInfo)
        //qInfo() << "...Starting test case " << QString::fromStdString(testInfo.name);
    }

    void testCaseEnded( Catch::TestCaseStats const& testCaseStats ) override
    {
        Q_UNUSED(testCaseStats)
        //This method is not called in case an assertion fails
        //qInfo() << "...Test case ended";
    }

    // Sections starting and ending
    void sectionStarting( Catch::SectionInfo const& sectionInfo ) override
    {
        Q_UNUSED(sectionInfo)
        //qInfo() << "sectionStarting";
    }

    void sectionEnded( Catch::SectionStats const& sectionStats ) override
    {
        Q_UNUSED(sectionStats)
        //qInfo() << "sectionEnded";
    }

    // Assertions before/ after
    void assertionStarting( Catch::AssertionInfo const& assertionInfo ) override
    {
        Q_UNUSED(assertionInfo)
        //qInfo() << "assertionStarting";
    }

    void skipTest( Catch::TestCaseInfo const& testInfo ) override
    {
        // A test is being skipped (because it is "hidden")
        Q_UNUSED(testInfo)
        //qInfo() << "Skip test case " << QString::fromStdString(testInfo.name);
    }

    bool assertionEnded( Catch::AssertionStats const& assertionStats ) override
    {
        if (!assertionStats.assertionResult.succeeded())
        {
            qInfo() << "-------------------------------------------------------------------------------";
            qInfo() << "ASSERTION FAILED";
            qInfo() << QString::fromStdString(assertionStats.assertionResult.getSourceInfo().file) << ":" << assertionStats.assertionResult.getSourceInfo().line << " FAILED";
            qInfo() << "-------------------------------------------------------------------------------";
            if (assertionStats.assertionResult.hasMessage())
            {
                qInfo() << "Assertion message: " << QString::fromStdString(assertionStats.assertionResult.getMessage());
            }
            qInfo() << "\t" << QString::fromStdString(assertionStats.assertionResult.getExpressionInMacro());
            if (assertionStats.assertionResult.hasExpandedExpression())
            {
                qInfo() << "\tExpanded: " << QString::fromStdString(assertionStats.assertionResult.getExpandedExpression());
            }
            qInfo() << "With messages:";

            for(std::size_t i = 0; i < assertionStats.infoMessages.size(); ++i)
            {
                qInfo() << "\t" << QString::fromStdString(assertionStats.infoMessages.at(i).message);
            }
        }
        // The return value indicates if the messages buffer should be cleared
        return true;
    }

    void testRunEnded( Catch::TestRunStats const& testRunStats ) override
    {
        qInfo() << "===============================================================================";
        if (testRunStats.aborting)
        {
            qInfo() << "Test run aborting "  << QString::fromStdString(testRunStats.runInfo.name);;
        }
        else if (testRunStats.totals.testCases.failed == 0)
        {
            qInfo() << "All tests passed (" << testRunStats.totals.assertions.total() << "assertions in " << testRunStats.totals.testCases.total() << "test case)";
        }
        else
        {
            qInfo() << "test cases: " << testRunStats.totals.testCases.total() << " | " << testRunStats.totals.testCases.failed << " failed";
            qInfo() << "Assertions: " << testRunStats.totals.assertions.total() << " | " << testRunStats.totals.assertions.failed << " failed";
        }
    }

};
CATCH_REGISTER_LISTENER( MyListener )


QString copyTestDataFilesFromAPK()
{
    // get default destination location from QT
    QString destdirstring;
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    if (locations.count()  > 0)
    {
        destdirstring = locations[0];
    }

    // Ensure path ends with "/"
    destdirstring = QDir::cleanPath(destdirstring);
    if (!destdirstring.endsWith('/'))
    {
        destdirstring = destdirstring.append('/');
    }

    QDir destdir;
    if (!destdir.exists(destdirstring))
    {
        qDebug() << "Destination path for testdata doesn't exist, creating.";
        if (!destdir.mkpath(destdirstring))
        {
            qCritical() << "Cannot create the destination path for testdata: " << destdirstring;
            return "";
        }
    }

    QDir assets("assets:/testdata");
    if (!assets.exists())
    {
        qCritical() << "Testdata not found in APK assets directory. Cannot copy testdata";
    }


    QDirIterator it(assets, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();

        // Chop of "assets:/" to get the relative path to append to the destination
        QString destpath = it.filePath().right(it.filePath().length() - 8); // 8= length of "assets:/
        destpath.prepend(destdirstring);

        const QFileInfo fileInfo = it.fileInfo();
        if(!fileInfo.isHidden())
        {
            if(fileInfo.isDir())
            {
                //Create destination folder path
                destdir.mkpath(destpath);
            }
            else if(fileInfo.isFile())
            {
                //Copy/Replace file in target directory
                if (QFile::exists(destpath))
                {
                    if (!QFile::remove(destpath))
                    {
                        qCritical() << "Cannot remove existing copy of file " << destpath;
                    }
                }

                if (!QFile::copy(fileInfo.absoluteFilePath(), destpath))
                {
                    qCritical() << "Cannot copy file " << destpath;
                }

                if (!QFile(destpath).setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther | QFile::WriteOther))
                {
                    // This will result in problems in the next iteration of this method because the files cannot be replaced then.
                    // Solution is to uninstall the app
                    qWarning() << "Cannot set permissions on file copied from APK: " << destpath;
                }
            }
        }
    }


    qInfo() << "Copied testdata directory to file system. Root path: " << destdirstring;

    return destdirstring;
}


#endif


int main(int argc, char** argv)
{
    // global setup...
    //QGuiApplication app(argc, argv); //Uncomment if you want a GUI for the test case later

    // Don't use QT logging before the tests, it will make the QT auto-test fail
    // on Linux because that instruments Catch to output XML (-r xml) which QT interprets.
    // Logging from QT before the XML makes the interpretation fail
//    qInfo() << "Catch2 Application wrapper started ";

#if defined(__ANDROID__)
    QStringList permissions({"android.permission.RECORD_AUDIO",
                             "android.permission.MODIFY_AUDIO_SETTINGS",
                             "android.permission.INTERNET",
                             "android.permission.VIBRATE",
                             "android.permission.READ_EXTERNAL_STORAGE",
                             "android.permission.WRITE_EXTERNAL_STORAGE",
                             "android.permission.WAKE_LOCK",
                             "android.permission.READ_PHONE_STATE",
                             "android.permission.BLUETOOTH",
                             "android.permission.FOREGROUND_SERVICE",
                             "android.hardware.sensor.proximity"
                            });

    QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync(permissions);
    for (auto permission : permissions)
    {
        qDebug() << "Permission: " << permission << " " << ((resultHash[permission] == QtAndroid::PermissionResult::Granted) ? "Granted" : "Denied");
    }

    // Copy the file to 'testdata' dir below local files. This is the current path, so no special path needs to be set
    // files can be accessed as "testdata/file.ext"
    copyTestDataFilesFromAPK();
#endif

    // Find the SERVERIP= option on the commandline.
    // MUST BE THE LAST COMMANDLINE OPTIONS and make sure to comma-separate the IP address from the catch args
    // Valid example: "SeekPrecision, SERVERIP=10.10.10.10"
    // Invalid example: "SeekPrecision SERVERIP=10.10.10.10"
    for (int i = 0;i < argc; i++)
    {
        QString argument(argv[i]);
        if (argument.contains("SERVERIP="))
        {
            int start = argument.indexOf("SERVERIP=") + 9; //8 = length of "SERVERIP="
            QString IP = argument.right(argument.length() - start);
            g_server_ipaddr = IP.toStdString();
        }
    }

    Catch::Session session; // There must be exactly one instance
    // writing to session.configData() here sets defaults
    // this is the preferred way to set them

    // Modify the arguments if needed here. Typically, this app will be run from QT Creator and
    // it's better to use the QT Creator run settings to specify commandline argument (such as a specific test case to run)
    int returnCode = session.applyCommandLine( argc, argv );
    if( returnCode != 0 ) // Indicates a command line error
    {
        qCritical() << "Catch2 initialization from commandline arguments failed";
        return returnCode;
    }

    // numFailed is clamped to 255 as some unices only use the lower 8 bits.
    // This clamping has already been applied, so just return it here
    // You can also do any post run clean-up here
    int numFailed = session.run();
    //qInfo() << "Catch2 finished running. Failed tests: " << numFailed;


    //return app.exec(); // This keeps the UI running intil explicitly closed. Not needed, but might be useful
    return numFailed;
}
