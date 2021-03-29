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

#include "ServerConfig.h"
#include "ServerGuard.h"
#include "ServerUtil.h"
#include "AppInfo.h"

#include <TeamTalkDefs.h>
#include <teamtalk/Log.h>

#include <ace/NT_Service.h>
#include <ace/Init_ACE.h>
#include <ace/Select_Reactor.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <iostream>
#include <map>
#include <sstream>
#include <regex>

#if !defined(WIN32)
#include <unistd.h>
#endif

using namespace std;
using namespace teamtalk;

class Service;

int RunServer(
#if defined(BUILD_NT_SERVICE)
              Service* service
#endif
              );

void PrintCommandArgs();

int ParseArguments(int argc, ACE_TCHAR* argv[]
#if defined(BUILD_NT_SERVICE)
              , Service* service
#endif
);

#if defined(BUILD_NT_SERVICE)
BOOL WINAPI ControlHandler(DWORD dwControlType)
{
    switch(dwControlType)
    {
    case CTRL_LOGOFF_EVENT :
        return TRUE;
    }
    return FALSE;
}
#endif

class StopReactorEventHandler : public ACE_Event_Handler
{
public:
    StopReactorEventHandler(ACE_Reactor* r)
        : ACE_Event_Handler(r)
    {
    }

    int handle_signal(int signum, siginfo_t*,ucontext_t*)
    {
        reactor()->end_reactor_event_loop();
        return 0;
    }
};

#if defined(BUILD_NT_SERVICE)

ACE_TString GetWinError(DWORD err)
{
    ACE_TString result;
    LPTSTR s = NULL;
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        err,
        0,
        (LPTSTR)&s,
        0,
        NULL);
    if(s)
    {
        result = s;
        ::LocalFree(s);
    }
    return result;
}

class Service : public ACE_NT_Service
{
public:
    void handle_control (DWORD control_code)
    {
        if (control_code == SERVICE_CONTROL_SHUTDOWN
            || control_code == SERVICE_CONTROL_STOP)
        {
            report_status (SERVICE_STOP_PENDING);

            reactor()->end_reactor_event_loop();
            //reactor ()->notify (this,
            //                    ACE_Event_Handler::EXCEPT_MASK);
        }
        else
            inherited::handle_control (control_code);
    }
    int svc (void)
    {
        // As an NT service, we come in here in a different thread than the
        // one which created the reactor.  So in order to do anything, we
        // need to own the reactor. If we are not a service, report_status
        // will return -1.
        if(RunServer(this) < 0)
            return -1;

        return 0;
    }
    //hack
    void report_status_foo(DWORD d1, DWORD d2 = 0)
    {
        this->report_status(d1,d2);
    }
private:
    typedef ACE_NT_Service inherited;
};

ACE_NT_SERVICE_DEFINE (TeamTalk,
                       Service,
                       ACE_TEXT(TEAMTALK_DESCRIPTION));

#endif

ACE_TString bindip, settingsfile, logfile = ACE_TEXT(TEAMTALK_LOGFILE), pidfile;
int tcpport = 0;
int udpport = 0;
bool verbose = false;
bool daemon_pid = false;
//setting files
ServerXML xmlSettings(TEAMTALK_XML_ROOTNAME);

bool bDaemon = false;
bool bNonDaemon = false;
int rxloss = 0, txloss = 0;

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE::init();
    int exitcode = EXIT_FAILURE;

#if defined(BUILD_NT_SERVICE)
    Service service;
    service.reactor(ACE_Reactor::instance());
    service.name(ACE_TEXT(TEAMTALK_NAME), ACE_TEXT(TEAMTALK_NAME));
    int parse_result = ParseArguments(argc, argv, &service);
    if(parse_result > 0)
    {
        ACE_NT_SERVICE_RUN (TeamTalk,
                            &service,
                            rett);
        exitcode = rett<0?EXIT_FAILURE:EXIT_SUCCESS;

    }
#else
    if(argc <= 1)
    {
        PrintCommandArgs();
        exitcode = EXIT_SUCCESS;
    }
    else
    {
        int parse_result = ParseArguments(argc, argv);
        if(parse_result>0)
        {
            exitcode = RunServer()<0?EXIT_FAILURE:EXIT_SUCCESS;
        }
        else
        {
            exitcode = EXIT_FAILURE;
        }
    }
#endif
    ACE::fini();
    return exitcode;
}

void RunEventLoop(ACE_Reactor* tcpReactor, ACE_Reactor* udpReactor, 
                  const ACE_TString& workdir, std::ofstream& logstream, 
                  ACE_INT64 log_maxsize)
{
    int ret = ACE_Thread_Manager::instance ()->spawn(event_loop, udpReactor);
    if(ret < 0)
        TT_LOG(ACE_TEXT("Failed to spawn UDP reactor."));

    SyncReactor(*udpReactor);

    int log_check = 0;
    ACE_Time_Value tm(10,0);
    while(tcpReactor->handle_events(tm) >= 0) {
        if(++log_check % 10 == 0 && log_maxsize>0 &&
           logstream.tellp() >= log_maxsize)
            RotateLogfile(workdir, logfile.c_str(), logstream);
        tm.set(10, 0);
    }

    udpReactor->end_reactor_event_loop();
}

int RunServer(
#if defined(BUILD_NT_SERVICE)
              Service* service
#endif
              )
{
    //avoid SIGPIPE
    ACE_Sig_Action no_sigpipe ((ACE_SignalHandler) SIG_IGN);
    ACE_Sig_Action original_action;
    no_sigpipe.register_action (SIGPIPE, &original_action);

    int ret = ACE::set_handle_limit(-1);//client handler (must be BIG)
    ACE_Select_Reactor selectReactor;
    ACE_Reactor tcpReactor(&selectReactor);
    ACE_Reactor::instance(&tcpReactor);
    ACE_Reactor::instance()->owner (ACE_OS::thr_self ());

    ACE_Reactor udpReactor;

#if defined(BUILD_NT_SERVICE)
    service->reactor(ACE_Reactor::instance());
#endif
    StopReactorEventHandler signalHandler(ACE_Reactor::instance());
    ACE_Reactor::instance()->register_handler(SIGTERM, &signalHandler);
    ACE_Reactor::instance()->register_handler(SIGINT, &signalHandler);
#if defined(WIN32)
    ACE_Reactor::instance()->register_handler(SIGBREAK, &signalHandler);
#endif

    //init logger
#if defined(BUILD_NT_SERVICE)
    ACE_LOG_MSG->open(ACE_TEXT(TEAMTALK_NAME), ACE_Log_Msg::SYSLOG | ACE_Log_Msg::OSTREAM);
#else
    ACE_LOG_MSG->open(ACE_TEXT(TEAMTALK_NAME));
#endif

    ACE_TCHAR workdir[512] = {};
    ACE_OS::getcwd(workdir, 512);

    //log file
    std::ofstream logstream;
    int64_t log_maxsize = xmlSettings.GetServerLogMaxSize();

    //enable logging
    if(log_maxsize != 0)
    {
        logstream.open(logfile.c_str(), ios::app);
        ACE_OSTREAM_TYPE * output = &logstream;
        ACE_LOG_MSG->msg_ostream(output, 0);
        ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
        u_long flag = LM_ERROR | LM_INFO | LM_DEBUG;
        ACE_LOG_MSG->priority_mask(flag, ACE_Log_Msg::PROCESS);
    }

    //create listener
    ServerGuard srvguard(xmlSettings);
    ServerNode servernode(ACE_TEXT( TEAMTALK_VERSION ), &tcpReactor, &tcpReactor, &udpReactor, &srvguard);

    ServerSettings prop = servernode.GetServerProperties();

    statchannels_t channels;
    if(!ReadServerProperties(xmlSettings, prop, channels))
    {
        ACE_TCHAR error_msg[1024];
        ACE_OS::snprintf(error_msg, 1024,
                         ACE_TEXT("Failed to read server properties from settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg);
        return -1;
    }

    prop.rxloss = rxloss;
    prop.txloss = txloss;

    //check for override options
    if(tcpport > 0)
    {
        ACE_INET_Addr addr((u_short)tcpport, prop.tcpaddrs.size() ? prop.tcpaddrs[0].get_host_addr() : NULL);
        prop.tcpaddrs.clear();
        prop.tcpaddrs.push_back(addr);
    }
    if(udpport > 0)
    {
        ACE_INET_Addr addr((u_short)udpport, prop.udpaddrs.size() ? prop.udpaddrs[0].get_host_addr(): NULL);
        prop.udpaddrs.clear();
        prop.udpaddrs.push_back(addr);
    }
    if(bindip.length())
    {
        ACE_INET_Addr tcpaddr(prop.tcpaddrs.size() ? prop.tcpaddrs[0].get_port_number() : tcpport, bindip.c_str());
        prop.tcpaddrs.clear();
        prop.tcpaddrs.push_back(tcpaddr);
        ACE_INET_Addr udpaddr(prop.udpaddrs.size() ? prop.udpaddrs[0].get_port_number() : udpport, bindip.c_str());
        prop.udpaddrs.clear();
        prop.udpaddrs.push_back(udpaddr);
    }

    if (!ConfigureServer(servernode, prop, channels))
    {
        ACE_TCHAR error_msg[1024];
        ACE_OS::snprintf(error_msg, 1024,
                         ACE_TEXT("Failed to configure the server from settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg);
        return -1;
    }
    channels.clear();

    bool encrypted = false;
#if defined(ENABLE_TEAMTALKPRO)
    encrypted = xmlSettings.GetCertificateFile().size() && xmlSettings.GetPrivateKeyFile().size();
    if (encrypted && !SetupEncryption(servernode, xmlSettings))
        return -1;
#endif

    ACE_TString systemid = SERVER_WELCOME;
#if defined(ENABLE_TEAMTALKPRO)
    systemid = Utf8ToUnicode(xmlSettings.GetSystemID(UnicodeToUtf8(systemid).c_str()).c_str());
    if (!servernode.StartServer(encrypted, systemid))
#else
    if (!servernode.StartServer(false, systemid))
#endif
    {
        ACE_TCHAR error_msg[1024];
        if(bindip.length() == 0)
            ACE_OS::snprintf(error_msg, 1024,
            ACE_TEXT("Unable to launch server using TCP port %d UDP port %d.\n")
            ACE_TEXT("Make sure the ports are not currently in use."),
            (int)prop.tcpaddrs[0].get_port_number(), (int)prop.udpaddrs[0].get_port_number());
        else
            ACE_OS::snprintf(error_msg, 1024,
            ACE_TEXT("Unable to launch server using IP-address %s TCP port %d UDP port %d.\n")
            ACE_TEXT("Make sure the ports are not currently in use."),
            prop.tcpaddrs[0].get_host_addr(), (int)prop.tcpaddrs[0].get_port_number(),
            (int)prop.udpaddrs[0].get_port_number());

        TT_SYSLOG(error_msg);
        return -1;
    }
    else
    {
        TT_LOG(ACE_TEXT("Started ") ACE_TEXT( TEAMTALK_NAME ) ACE_TEXT(" v.") ACE_TEXT( TEAMTALK_VERSION ) ACE_TEXT("."));
        if(!verbose)
            ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);

        //don't write server events to syslog
        ACE_LOG_MSG->clr_flags(ACE_Log_Msg::SYSLOG);
    }

#if defined(BUILD_NT_SERVICE)
    SetConsoleCtrlHandler(ControlHandler, TRUE);
    service->report_status_foo(SERVICE_RUNNING);
    RunEventLoop(ACE_Reactor::instance(), &udpReactor, workdir, logstream, log_maxsize);
#else
    if(bDaemon)
    {
#if defined(WIN32)
        cout << "Windows does not support daemon mode. Use -nd parameter instead." << endl;
        return -1;
#else
        pid_t pid, sid;

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0)
        {
            TT_SYSLOG(ACE_TEXT("Failed to start ") ACE_TEXT( TEAMTALK_NAME ));
            exit(EXIT_FAILURE);
        }

        /* If we got a good PID, then
        we can exit the parent process. */
        if (pid > 0)
        {
            if(daemon_pid)
                cout << TEAMTALK_NAME << " daemon got PID " << pid << endl;

            if(pidfile.length())
            {
                std::ofstream fb(pidfile.c_str(), ios_base::out | ios_base::trunc);
                if(fb.fail())
                {
                    cerr << "Failed to open: " << pidfile << endl;
                }
                fb << pid;
                fb.close();
            }
            exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);

        /* Open any logs here */

        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0)
        {
            TT_SYSLOG("No SID assigned to child process.");
            /* Log any failure */
            exit(EXIT_FAILURE);
        }

        RunEventLoop(ACE_Reactor::instance(), &udpReactor, workdir, logstream, log_maxsize);

#endif /* WIN32 */
    }
    else if(bNonDaemon)
    {
        //TCP commands thread
        RunEventLoop(ACE_Reactor::instance(), &udpReactor, workdir, logstream, log_maxsize);
    }
#endif /* BUILD_NT_SERVICE */

    ACE_LOG_MSG->set_flags(ACE_Log_Msg::STDERR);

    servernode.StopServer();
    TT_LOG(ACE_TEXT("Stopped ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("."));

    ACE_Thread_Manager::instance ()->wait ();

    udpReactor.close();
    tcpReactor.close();

    ACE::fini();

    return 0;
}

int ParseArguments(int argc, ACE_TCHAR* argv[]
#if defined(BUILD_NT_SERVICE)
              , Service* service
#endif
)
{
    std::map<ACE_TString,ACE_TString> args;

    for(int i=0;i<argc;i++)
    {
        ACE_TString str(argv[i]);
        pair<ACE_TString,ACE_TString> newPair;
        newPair.first = str;
        if ((str == ACE_TEXT("-wd") ||
            str == ACE_TEXT("-tcpport") ||
            str == ACE_TEXT("-udpport") ||
            str == ACE_TEXT("-ip") ||
            str == ACE_TEXT("-c") ||
            str == ACE_TEXT("-l") ||
            str == ACE_TEXT("-pid-file") ||
            str == ACE_TEXT("-rxloss") ||
            str == ACE_TEXT("-txloss")))
        {
            if(i+1 >= argc)
            {
                cerr << "Missing option for parameter " << str << endl;
                return 0;
            }

            newPair.second = argv[i+1];
            i++;
        }
        else
        {
            newPair.second = ACE_TEXT("");
        }
        args.insert(newPair);
    }

    std::map<ACE_TString,ACE_TString>::iterator ite;

    if(args.find(ACE_TEXT("--version")) != args.end())
    {
        cout << TEAMTALK_NAME << " version " << TEAMTALK_VERSION_FRIENDLY << endl;
        return 0;
    }
    if(args.find(ACE_TEXT("--help")) != args.end() ||
        args.find(ACE_TEXT("-help")) != args.end() ||
        args.find(ACE_TEXT("/help")) != args.end() ||
        args.find(ACE_TEXT("-h")) != args.end() ||
        args.find(ACE_TEXT("/h")) != args.end() ||
        args.find(ACE_TEXT("-?")) != args.end() ||
        args.find(ACE_TEXT("/?")) != args.end()
        )
    {
        PrintCommandArgs();
        return 0;
    }
    if(args.find(ACE_TEXT("-d")) != args.end())
    {
        bDaemon = true;
    }
    if(args.find(ACE_TEXT("-nd")) != args.end())
    {
        bNonDaemon = true;
    }
    if(args.find(ACE_TEXT("-daemon-pid")) != args.end())
    {
        daemon_pid = true;
    }
    if((ite = args.find(ACE_TEXT("-pid-file"))) != args.end())
    {
        pidfile = (*ite).second.c_str();
    }
    if( (ite = args.find(ACE_TEXT("-tcpport"))) != args.end())
    {
        tcpport = int(string2i((*ite).second.c_str()));
    }
    if( (ite = args.find(ACE_TEXT("-udpport"))) != args.end())
    {
        udpport = int(string2i((*ite).second.c_str()));
    }
    if( (ite = args.find(ACE_TEXT("-ip"))) != args.end())
    {
        bindip = (*ite).second;
    }
    if( (ite = args.find(ACE_TEXT("-verbose"))) != args.end())
    {
        verbose = true;
    }
    if( (ite = args.find(ACE_TEXT("-rxloss"))) != args.end())
    {
        rxloss = ACE_OS::atoi((*ite).second.c_str());
    }
    if( (ite = args.find(ACE_TEXT("-txloss"))) != args.end())
    {
        txloss = ACE_OS::atoi((*ite).second.c_str());
    }
    if( (ite = args.find(ACE_TEXT("-wd"))) != args.end())
    {
        ACE_TString workdir = (*ite).second;
        int ret = ACE_OS::chdir(workdir.c_str());
        if(ret != 0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("%s failed to change to directory \"%s\"."),
                             ACE_TEXT(TEAMTALK_NAME), workdir.c_str());
            TT_SYSLOG(error_msg);
            return -1;
        }
    }
    if( (ite = args.find(ACE_TEXT("-c"))) != args.end())
    {
        settingsfile = ite->second;
    }
    if( (ite = args.find(ACE_TEXT("-l"))) != args.end())
    {
        logfile = ite->second;
    }
#if defined(BUILD_NT_SERVICE)
    else
    {
        //load config file from same directory as
        //executable file when running as NT service.
        ACE_TCHAR bufPath[MAX_PATH] = {};
        if(GetModuleFileName(NULL, bufPath, MAX_PATH)>0)
        {
            TCHAR* pChr = ACE_OS::strrchr(bufPath, '\\');
            TTASSERT(pChr);
            if(pChr)
            {
                pChr++;
                *pChr = 0;
                ACE_OS::chdir(bufPath);
            }
            settingsfile = bufPath;
            settingsfile += ACE_TEXT( TEAMTALK_SETTINGSFILE );
        }
    }
    //pick out options related to NT services

    if( (ite = args.find(ACE_TEXT("-i"))) != args.end())
    {
        if(service->insert(SERVICE_AUTO_START)<0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to install ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                         GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
        }
        else
        {
            TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" installed successfully."));
        }
        return 0;
    }
    if( (ite = args.find(ACE_TEXT("-u"))) != args.end())
    {
        if(service->remove()<0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to remove ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                         GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
        }
        else
        {
            TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" uninstalled successfully."));
        }
        return 0;
    }
    if( (ite = args.find(ACE_TEXT("-s"))) != args.end())
    {
        if(service->start_svc()<0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to start ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                         GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
        }
        else
        {
            TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" started successfully."));
        }
        return 0;
    }
    if( (ite = args.find(ACE_TEXT("-e"))) != args.end())
    {
        if(service->stop_svc()<0)
        {
            ACE_TCHAR error_msg[1024];
            ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to stop ") ACE_TEXT(TEAMTALK_NAME) ACE_TEXT("\nError: %s"),
                         GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg);
        }
        else
        {
            TT_LOG(ACE_TEXT(TEAMTALK_NAME) ACE_TEXT(" stopped successfully."));
        }
        return 0;
    }
#endif

    if(!LoadConfig(xmlSettings, settingsfile))
    {
        ACE_TCHAR error_msg[1024];
        ACE_OS::snprintf(error_msg, 1024, ACE_TEXT("Failed to load settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg);
        return -1;
    }

    // remove all facebook logins
    if (!VersionSameOrLater(Utf8ToUnicode(xmlSettings.GetFileVersion().c_str()), ACE_TEXT("5.2")))
    {
        bool removefb = false, fbfound = false;
        int index = 0;
        UserAccount ua;
        while (xmlSettings.GetNextUser(index, ua))
        {
            bool fbpostfix;
#if defined(UNICODE)
            fbpostfix = std::regex_search(ua.username.c_str(), std::wregex(ACE_TEXT("@facebook.com")));
#else
            fbpostfix = std::regex_search(ua.username.c_str(), std::regex("@facebook.com"));
#endif
            if (ua.username == ACE_TEXT("facebook") || fbpostfix)
            {
                fbfound = true;
                if (!removefb)
                {
                    cout << "Facebook login is no longer supported. Remove all Facebook logins.";
                    removefb = printGetBool(true);
                    if (!removefb)
                        break;
                }
#if defined(UNICODE)
                std::string fbname = UnicodeToUtf8(ua.username.c_str()).c_str();
#else
                std::string fbname = ua.username.c_str();
#endif
                cout << "Removed: " << Utf8ToLocal(fbname.c_str()) << endl;
                xmlSettings.RemoveUser(fbname.c_str());
            }
            else index++;
            
            ua = UserAccount();
        }

        // facebook accounts removed, save new version
        if (fbfound)
        {
            xmlSettings.SetFileVersion(TEAMTALK_XML_VERSION);
            xmlSettings.SaveFile();
        }
    }

    if( (ite = args.find(ACE_TEXT("-wizard"))) != args.end())
    {
        RunWizard(xmlSettings);
    }

#if !defined(BUILD_NT_SERVICE)
    if(!bNonDaemon && !bDaemon)
    {
        TT_LOG(ACE_TEXT("Missing either -d or -nd parameter in order to start."));
        return 0;
    }
#endif

    return 1; //GO
}

void PrintCommandArgs()
{
    cout << endl;
    cout << TEAMTALK_NAME << " version " << TEAMTALK_VERSION_FRIENDLY << endl;
    cout << "Compiled on " __DATE__ " " __TIME__ "." << endl;
    cout << endl;
    cout << "Copyright (c) 2002-2021, BearWare.dk" << endl;
    cout << endl;
    cout << "Usage: " << TEAMTALK_EXE << " [OPTIONS]" << endl << endl;
#if defined(BUILD_NT_SERVICE)
    cout << "Windows NT service install options:" << endl << endl;
    cout << "  -i               Install " << TEAMTALK_NAME << " as NT service." << endl;
    cout << "  -u               Uninstall " << TEAMTALK_NAME << " from NT services." << endl;
    cout << "  -s               Start NT service (must already be installed)." << endl;
    cout << "  -e               Stop NT service (must already be running)." << endl;
    cout << endl;
    cout << "Windows NT service startup arguments:" << endl << endl;
#else
    cout << "Valid options:" << endl;
#endif
#if !defined(BUILD_NT_SERVICE)
#if !defined(WIN32)
    cout << "  -d               Start " << TEAMTALK_NAME << " as daemon." << endl;
    cout << "  -daemon-pid      Print PID of daemon started with -d option." << endl;
    cout << "  -pid-file [FILE] Write PID of daemon started with -d option to file." << endl;
#endif
    cout << "  -nd              Start " << TEAMTALK_NAME << " as non-daemon." << endl;
#endif
    cout << "  -wizard          Run the setup-wizard to configure the server." << endl;
    cout << "  -c [FILE]        Instead of loading " << TEAMTALK_SETTINGSFILE << " from current directory" << endl;
    cout << "                   use this specified file." << endl;
    cout << "  -l [FILE]        If logging is enabled save to the specified filename instead" << endl;
    cout << "                   of writing to " << TEAMTALK_LOGFILE << " in current directory." << endl;
    cout << "  -wd [DIR]        Set working directory (where " << TEAMTALK_SETTINGSFILE << endl;
    cout << "                   is located and where the log file " << TEAMTALK_LOGFILE << endl;
    cout << "                   will be stored)." << endl;
    cout << "                   Current directory is the default location of " << endl;
    cout << "                   " << TEAMTALK_SETTINGSFILE << " and log file." << endl;
    cout << "  -tcpport [PORT]  Override the <tcpport> setting in " << TEAMTALK_SETTINGSFILE << "." << endl;
    cout << "  -udpport [PORT]  Override the <udpport> setting in " << TEAMTALK_SETTINGSFILE << "." << endl;
    cout << "  -ip [IPADDR]     Override <bind-ip> setting in " << TEAMTALK_SETTINGSFILE << "." << endl;
    cout << "  -verbose         Output log information to console." << endl;
    cout << "  --version        Displays version info." << endl;
    cout << "  --help           Displays this message." << endl;

#if !defined(BUILD_NT_SERVICE)
    cout << endl;
    cout << "Examples: " << TEAMTALK_EXE << " -nd" << endl;
    cout << "          " << TEAMTALK_EXE << " -wd /home/bill/srv1 -ip 192.168.0.2 -nd" << endl;
    cout << "          " << TEAMTALK_EXE << " -c /home/bill/srv1/" << TEAMTALK_SETTINGSFILE << " -l " << TEAMTALK_LOGFILE << " -nd" << endl;
#endif
    cout << endl;
    cout << "Kind regards go to the people behind the ACE Framework, Speex, OPUS" << endl;
    cout << "and WebM projects!" << endl;
    cout << endl;
    cout << "Report bugs to contact@bearware.dk" << endl;
    cout << endl;
}
