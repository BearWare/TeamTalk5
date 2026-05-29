/*
 * Copyright (c) 2005-2018, BearWare.dk
 *
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#include "AppInfo.h"
#include "ServerUtil.h"
#include "SpamBotConfig.h"
#include "SpamBotSession.h"
#include "SpamBotXML.h"
#include "TeamTalkDefs.h"
#include "myace/MyACE.h"
#include "teamtalk/Log.h"

#include <ace/Init_ACE.h>
#include <ace/Log_Msg.h>
#include <ace/OS_NS_unistd.h>
#include <ace/SString.h>

#if defined(BUILD_NT_SERVICE)
#include <ace/NT_Service.h>
#endif

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(WIN32)
#include <unistd.h>
#endif

using namespace std;
using namespace spambot;

static ACE_TString settingsfile;
static ACE_TString logfile = ACE_TEXT(TEAMTALK_LOGFILE);
static ACE_TString pidfile;
static ACE_TString weblogin;
static ACE_TString tokenlogin;
static bool verbose    = false;
static bool daemon_pid = false;
static bool daemon_mode = false;
static bool nondaemon  = false;

static SpamBotXML xmlSettings;
static std::ofstream logstream;
static SpamBotSession* g_session = nullptr;

class Service;

static int RunBot(
#if defined(BUILD_NT_SERVICE)
                  Service* service
#endif
                  );

static void PrintCommandArgs();

static int ParseArguments(int argc, ACE_TCHAR* argv[]
#if defined(BUILD_NT_SERVICE)
                          , Service* service
#endif
);

#if defined(BUILD_NT_SERVICE)

ACE_TString GetWinError(DWORD err)
{
    ACE_TString result;
    LPTSTR s = nullptr;
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, err, 0, (LPTSTR)&s, 0, NULL);
    if (s)
    {
        result = s;
        ::LocalFree(s);
    }
    return result;
}

class Service : public ACE_NT_Service
{
public:
    void handle_control(DWORD control_code) override
    {
        if (control_code == SERVICE_CONTROL_SHUTDOWN || control_code == SERVICE_CONTROL_STOP)
        {
            report_status(SERVICE_STOP_PENDING);
            if (g_session)
                g_session->Stop();
        }
        else
        {
            inherited::handle_control(control_code);
        }
    }

    int svc() override
    {
        return RunBot(this) < 0 ? -1 : 0;
    }

    void report_status_foo(DWORD d1, DWORD d2 = 0)
    {
        this->report_status(d1, d2);
    }

private:
    typedef ACE_NT_Service inherited;
};

ACE_NT_SERVICE_DEFINE(SpamBot, Service, ACE_TEXT(TEAMTALK_DESCRIPTION));

#endif /* BUILD_NT_SERVICE */

static void HandleSignal(int /*signum*/)
{
    if (g_session)
        g_session->Stop();
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE::init();
    int exitcode = EXIT_FAILURE;

#if defined(BUILD_NT_SERVICE)
    Service service;
    service.name(ACE_TEXT(TEAMTALK_NAME), ACE_TEXT(TEAMTALK_NAME));
    int const parse_result = ParseArguments(argc, argv, &service);
    if (parse_result > 0)
    {
        ACE_NT_SERVICE_RUN(SpamBot, &service, rett);
        exitcode = rett ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else if (parse_result == 0)
        exitcode = EXIT_SUCCESS;
#else
    if (argc <= 1)
    {
        PrintCommandArgs();
        exitcode = EXIT_SUCCESS;
    }
    else
    {
        int const parse_result = ParseArguments(argc, argv);
        if (parse_result > 0)
            exitcode = RunBot() < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
        else if (parse_result == 0)
            exitcode = EXIT_SUCCESS;
        else
            exitcode = EXIT_FAILURE;
    }
#endif
    ACE::fini();
    return exitcode;
}

static int RunBot(
#if defined(BUILD_NT_SERVICE)
                  Service* service
#endif
                  )
{
#if defined(BUILD_NT_SERVICE)
    ACE_LOG_MSG->open(ACE_TEXT(TEAMTALK_NAME), ACE_Log_Msg::SYSLOG | ACE_Log_Msg::OSTREAM);
#else
    ACE_LOG_MSG->open(ACE_TEXT(TEAMTALK_NAME));
#endif

    if (xmlSettings.GetLogEnabled())
    {
        logstream.open(logfile.c_str(), ios::app);
        ACE_LOG_MSG->msg_ostream(&logstream, false);
        ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
        u_long const flag = LM_ERROR | LM_INFO | LM_DEBUG;
        ACE_LOG_MSG->priority_mask(flag, ACE_Log_Msg::PROCESS);
    }

    std::signal(SIGINT,  HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    TT_LOG(ACE_TEXT("Started ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" v.") ACE_TEXT(TEAMTALK_VERSION) ACE_TEXT("."));
    if (!verbose)
        ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);

    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::SYSLOG);

    SpamBotSession session(xmlSettings);
    g_session = &session;
#if defined(BUILD_NT_SERVICE)
    service->report_status_foo(SERVICE_RUNNING);
#endif

#if !defined(BUILD_NT_SERVICE) && !defined(WIN32)
    if (daemon_mode)
    {
        pid_t const pid = fork();
        if (pid < 0)
        {
            TT_SYSLOG(ACE_TEXT("Failed to start ") ACE_TEXT(TEAMTALK_NAME));
            return -1;
        }
        if (pid > 0)
        {
            if (daemon_pid)
                cout << TEAMTALK_NAME << " daemon got PID " << pid << endl;
            if (!pidfile.empty())
            {
                std::ofstream fb(pidfile.c_str(), ios_base::out | ios_base::trunc);
                if (fb.fail())
                    cerr << "Failed to open: " << pidfile << endl;
                fb << pid;
                fb.close();
            }
            exit(EXIT_SUCCESS);
        }
        umask(0);
        if (setsid() < 0)
        {
            TT_SYSLOG(ACE_TEXT("No SID assigned to child process."));
            return -1;
        }
    }
#endif

    int const rc = session.Run();
    g_session = nullptr;

    ACE_LOG_MSG->set_flags(ACE_Log_Msg::STDERR);
    TT_LOG(ACE_TEXT("Stopped ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("."));
    return rc;
}

static int ParseArguments(int argc, ACE_TCHAR* argv[]
#if defined(BUILD_NT_SERVICE)
                          , Service* service
#endif
)
{
    std::map<ACE_TString, ACE_TString> args;

    for (int i = 0; i < argc; ++i)
    {
        ACE_TString const str(argv[i]);
        std::pair<ACE_TString, ACE_TString> p;
        p.first = str;
        if (str == ACE_TEXT("-c") || str == ACE_TEXT("-l") || str == ACE_TEXT("-wd") ||
            str == ACE_TEXT("-pid-file") ||
            str == ACE_TEXT("-weblogin") || str == ACE_TEXT("-tokenlogin"))
        {
            if (i + 1 >= argc)
            {
                cerr << "Missing option for parameter " << str << endl;
                return 0;
            }
            p.second = argv[i + 1];
            ++i;
        }
        else
        {
            p.second = ACE_TEXT("");
        }
        args.insert(p);
    }

    std::map<ACE_TString, ACE_TString>::iterator ite;

    if (args.contains(ACE_TEXT("--version")))
    {
        cout << TEAMTALK_NAME << " version " << TEAMTALK_VERSION_FRIENDLY << endl;
        return 0;
    }
    if (args.contains(ACE_TEXT("--help")) || args.contains(ACE_TEXT("-help")) ||
        args.contains(ACE_TEXT("-h")) || args.contains(ACE_TEXT("/h")) ||
        args.contains(ACE_TEXT("-?")) || args.contains(ACE_TEXT("/?")))
    {
        PrintCommandArgs();
        return 0;
    }

    if (args.contains(ACE_TEXT("-d")))  daemon_mode = true;
    if (args.contains(ACE_TEXT("-nd"))) nondaemon  = true;
    if (args.contains(ACE_TEXT("-verbose"))) verbose = true;
    if (args.contains(ACE_TEXT("-daemon-pid"))) daemon_pid = true;

    if ((ite = args.find(ACE_TEXT("-pid-file"))) != args.end())
        pidfile = ite->second;

    if ((ite = args.find(ACE_TEXT("-wd"))) != args.end())
    {
        ACE_TString const workdir = ite->second;
        if (ACE_OS::chdir(workdir.c_str()) != 0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("%s failed to change to directory \"%s\"."),
                             ACE_TEXT(TEAMTALK_NAME), workdir.c_str());
            TT_SYSLOG(error_msg);
            return -1;
        }
    }
    if ((ite = args.find(ACE_TEXT("-c"))) != args.end())
        settingsfile = ite->second;
    if ((ite = args.find(ACE_TEXT("-l"))) != args.end())
        logfile = ite->second;
    if ((ite = args.find(ACE_TEXT("-weblogin"))) != args.end())
        weblogin = ite->second;
    if ((ite = args.find(ACE_TEXT("-tokenlogin"))) != args.end())
        tokenlogin = ite->second;

#if defined(BUILD_NT_SERVICE)
    if (settingsfile.is_empty())
    {
        ACE_TCHAR bufPath[MAX_PATH] = {};
        if (GetModuleFileName(nullptr, bufPath, MAX_PATH) > 0)
        {
            TCHAR* pChr = ACE_OS::strrchr(bufPath, '\\');
            if (pChr)
            {
                pChr++;
                *pChr = 0;
                ACE_OS::chdir(bufPath);
            }
            settingsfile = bufPath;
            settingsfile += ACE_TEXT(TEAMTALK_SETTINGSFILE);
        }
    }

    if ((ite = args.find(ACE_TEXT("-i"))) != args.end())
    {
        if (service->insert(SERVICE_AUTO_START) < 0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to install ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                             GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
            return -1;
        }
        TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" installed successfully."));
        return 0;
    }
    if ((ite = args.find(ACE_TEXT("-u"))) != args.end())
    {
        if (service->remove() < 0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to remove ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                             GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
            return -1;
        }
        TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" uninstalled successfully."));
        return 0;
    }
    if ((ite = args.find(ACE_TEXT("-s"))) != args.end())
    {
        if (service->start_svc() < 0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to start ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                             GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
            return -1;
        }
        TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" started successfully."));
        return 0;
    }
    if ((ite = args.find(ACE_TEXT("-e"))) != args.end())
    {
        if (service->stop_svc() < 0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to stop ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                             GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
            return -1;
        }
        TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" stopped successfully."));
        return 0;
    }
#endif /* BUILD_NT_SERVICE */

    if (settingsfile.is_empty())
        settingsfile = ACE_TEXT(TEAMTALK_SETTINGSFILE);

    if (!LoadConfig(xmlSettings, settingsfile))
    {
        ACE_TCHAR error_msg[1024];
        ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to load settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg);
        return -1;
    }

#if defined(ENABLE_TEAMTALKPRO)
    if (!tokenlogin.empty())
    {
        const ACE_TString rgx = ACE_TEXT("^(.*):(.*)$");
#if defined(UNICODE)
        std::wsmatch sm;
        std::wstring s = tokenlogin.c_str();
#else
        std::smatch sm;
        std::string s = tokenlogin.c_str();
#endif
        if (std::regex_search(s, sm, BuildRegex(rgx.c_str())) && sm.size() == 3)
        {
            ACE_TString const loginid  = sm[1].str().c_str();
            ACE_TString const newtoken = sm[2].str().c_str();
            if (AuthBearWareAccount(loginid, newtoken) != WEBLOGIN_SUCCESS)
            {
                TT_SYSLOG(ACE_TEXT("Failed to authenticate BearWare.dk WebLogin."));
                return 0;
            }
            xmlSettings.SetBearWareWebLogin(UnicodeToUtf8(loginid).c_str(),
                                            UnicodeToUtf8(newtoken).c_str());
            xmlSettings.SaveFile();
            TT_SYSLOG(ACE_TEXT("BearWare.dk WebLogin succedded. Token stored."));
        }
        else
        {
            TT_SYSLOG(ACE_TEXT("Invalid format for TOKEN. Should be \"bear_dk@bearware.dk:aefd43fea\""));
            return 0;
        }
    }
    if (!weblogin.empty())
    {
        const ACE_TString rgx = ACE_TEXT("^(.*):(.*)$");
#if defined(UNICODE)
        std::wsmatch sm;
        std::wstring s = weblogin.c_str();
#else
        std::smatch sm;
        std::string s = weblogin.c_str();
#endif
        if (std::regex_search(s, sm, BuildRegex(rgx.c_str())) && sm.size() == 3)
        {
            ACE_TString const loginid = sm[1].str().c_str();
            ACE_TString const passwd  = sm[2].str().c_str();
            ACE_TString bearwareid, token;
            if (LoginBearWareAccount(loginid, passwd, token, bearwareid) != WEBLOGIN_SUCCESS)
            {
                TT_SYSLOG(ACE_TEXT("Failed to authenticate BearWare.dk WebLogin."));
                return 0;
            }
            xmlSettings.SetBearWareWebLogin(UnicodeToUtf8(bearwareid).c_str(),
                                            UnicodeToUtf8(token).c_str());
            xmlSettings.SaveFile();
            TT_SYSLOG(ACE_TEXT("BearWare.dk WebLogin succedded. Token stored."));
        }
        else
        {
            TT_SYSLOG(ACE_TEXT("Invalid format for AUTH. Should be \"bear_dk:mysecretpassword\""));
            return 0;
        }
    }
#endif /* ENABLE_TEAMTALKPRO */

    bool skipstart = false;

    if (args.contains(ACE_TEXT("-wizard")))
    {
        RunWizard(xmlSettings);
        skipstart = true;
    }

#if !defined(BUILD_NT_SERVICE)
    if (!nondaemon && !daemon_mode)
    {
        if (!skipstart)
            TT_LOG(ACE_TEXT("Missing either -d or -nd parameter in order to start."));
        return 0;
    }
#endif

#if defined(ENABLE_TEAMTALKPRO)
    while (!LoginBearWare(xmlSettings))
    {
        TT_LOG(ACE_TEXT("Failed to log on using BearWare.dk WebLogin."));
    }
#endif

    return 1;
}

static void PrintCommandArgs()
{
    cout << endl;
    cout << TEAMTALK_NAME << " version " << TEAMTALK_VERSION_FRIENDLY << endl;
    cout << "Compiled on " __DATE__ " " __TIME__ "." << endl;
    cout << endl;
    cout << "Copyright (c) 2002-2026, BearWare.dk" << endl;
    cout << endl;
    cout << "Usage: " << TEAMTALK_EXE << " [OPTIONS]" << endl << endl;
#if defined(BUILD_NT_SERVICE)
    cout << "Windows NT service options:" << endl << endl;
    cout << "  -i               Install " << TEAMTALK_NAME << " as NT service." << endl;
    cout << "  -u               Uninstall " << TEAMTALK_NAME << " from NT services." << endl;
    cout << "  -s               Start the service." << endl;
    cout << "  -e               Stop the service." << endl;
    cout << endl;
#endif
    cout << "Options:" << endl;
    cout << "  -wizard          Run interactive setup wizard." << endl;
    cout << "  -c <FILE>        Use specific XML configuration file." << endl;
    cout << "  -l <FILE>        Use specific log file." << endl;
    cout << "  -wd <DIR>        Change to working directory before starting." << endl;
    cout << "  -verbose         Also print log messages to standard error." << endl;
    cout << "  -weblogin <USER@bearware.dk:PASSWORD>" << endl;
    cout << "                   Authenticate BearWare.dk WebLogin and store token." << endl;
    cout << "  -tokenlogin <USER@bearware.dk:TOKEN>" << endl;
    cout << "                   Store BearWare.dk WebLogin token for later use." << endl;
#if !defined(BUILD_NT_SERVICE)
    cout << "  -d               Run " << TEAMTALK_NAME << " as a daemon (Linux only)." << endl;
    cout << "  -nd              Run " << TEAMTALK_NAME << " as a non-daemon." << endl;
    cout << "  -pid-file <FILE> Write daemon PID to this file." << endl;
    cout << "  -daemon-pid      Print daemon PID after forking." << endl;
#endif
}
