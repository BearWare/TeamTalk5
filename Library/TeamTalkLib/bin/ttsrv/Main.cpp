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

#include "ServerGuard.h"
#include "AppInfo.h"

#include <TeamTalkDefs.h>
#include <teamtalk/Log.h>
#include <mystd/MyStd.h>

#include <ace/streams.h>
#include <ace/NT_Service.h>
#include <ace/Init_ACE.h>
#include <ace/Select_Reactor.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <iostream>
#include <map>
#include <sstream>

#if !defined(WIN32)
#include <unistd.h>
#endif

using namespace std;
using namespace teamtalk;

#if defined(UNICODE)
typedef wostringstream tostringstream;
#else
typedef ostringstream tostringstream;
#endif

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


void RunWizard(ServerXML& xmlSettings);

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
    ACE_INT64 log_maxsize = xmlSettings.GetServerLogMaxSize();

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

    if(!ConfigureServer(servernode, prop, channels))
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
    cout << "Copyright (c) 2002-2019, BearWare.dk" << endl;
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

bool inputTrue(const std::string& input)
{
    std::string tmp = str2lower(input);
    return tmp == "y" || tmp == "yes";
}

std::string printGetString(const std::string& input)
{
    cout << "(\"" << input << "\") ";
    std::string tmp;
    std::getline(cin, tmp);
    return tmp.empty()? input : tmp.c_str();
}

bool printGetBool(bool value)
{
    cout << "[Y/N] ";
    std::string input = printGetString(value?"Y":"N");
    return inputTrue(input);
}

int printGetInt(int value)
{
    ostringstream os;
    os << value;
    std::string sval = printGetString(os.str().c_str());
    istringstream is(sval.c_str());
    is >> value;
    return value;
}

_INT64 printGetInt64(_INT64 value)
{
    ostringstream os;
    os << value;
    std::string sval = printGetString(os.str().c_str());
    istringstream is(sval.c_str());
    is >> value;
    return value;
}

void RunWizard(ServerXML& xmlSettings)
{
    cout << TEAMTALK_NAME << " " << TEAMTALK_VERSION_FRIENDLY << " configurator" << endl;
    cout << endl;

    cout << "Do you want to configure your " TEAMTALK_NAME "? ";
    if(!printGetBool(true))
        return;

    cout << endl;
    cout << "Configuring file: " << xmlSettings.GetFileName() << endl;

    ACE_TString servername, motd, filesroot;
    std::vector<std::string> bindips;
    ACE_TString certfile, keyfile;
    int maxusers, max_logins_per_ip = 0;
    bool autosave = true;
    _INT64 diskquota = 0, maxdiskusage = 0, log_maxsize = 0;
    int tcpport = DEFAULT_TCPPORT, udpport = DEFAULT_UDPPORT, max_login_attempts = 0, logindelay = 0;

    servername = Utf8ToUnicode(xmlSettings.GetServerName().c_str());
    motd = Utf8ToUnicode(xmlSettings.GetMessageOfTheDay().c_str());
    filesroot = Utf8ToUnicode(xmlSettings.GetFilesRoot().c_str());
    bindips = xmlSettings.GetBindIPs();;
    tcpport = xmlSettings.GetHostTcpPort()==UNDEFINED?DEFAULT_TCPPORT:xmlSettings.GetHostTcpPort();
    udpport = xmlSettings.GetHostUdpPort()==UNDEFINED?DEFAULT_UDPPORT:xmlSettings.GetHostUdpPort();

    maxusers = xmlSettings.GetMaxUsers() == UNDEFINED?MAX_USERS:xmlSettings.GetMaxUsers();
    log_maxsize = xmlSettings.GetServerLogMaxSize();
    autosave = xmlSettings.GetAutoSave();
    diskquota = xmlSettings.GetDefaultDiskQuota();
    maxdiskusage = xmlSettings.GetMaxDiskUsage();
    max_login_attempts = xmlSettings.GetMaxLoginAttempts();
    max_logins_per_ip = xmlSettings.GetMaxLoginsPerIP();
    logindelay = xmlSettings.GetLoginDelay();

#if defined(ENABLE_TEAMTALKPRO)
    certfile = Utf8ToUnicode(xmlSettings.GetCertificateFile().c_str());
    keyfile = Utf8ToUnicode(xmlSettings.GetPrivateKeyFile().c_str());
#endif

    cout << endl;
    cout << "Ready to configure " << TEAMTALK_NAME << " settings." << endl;

#if !defined(WIN32)
    ACE_TCHAR *s;
    int utf8_mode = 0;

    if (((s = getenv("LC_ALL"))   && *s) ||
        ((s = getenv("LC_CTYPE")) && *s) ||
        ((s = getenv("LANG"))     && *s)) {
            if (strstr(s, "UTF-8"))
                utf8_mode = 1;
    }
    if(!utf8_mode)
        cout << "Warning: UTF-8 not enabled. Please stick to English characters!" << endl;
#endif
    //cout << "Value in parantesis will be used if no input is specified." << endl;
    cout << endl;
    cout << TEAMTALK_NAME << " name: ";
    servername = LocalToUnicode(printGetString(UnicodeToLocal(servername).c_str()).c_str());
    cout << "Message of the Day: ";
    motd = LocalToUnicode(printGetString(UnicodeToLocal(motd).c_str()).c_str());
    cout << "Maximum users allowed on server: ";
    maxusers = printGetInt(maxusers);
    if(maxusers > MAX_USERS)
    {
        cout << "Maximum allowed users is " << MAX_USERS << " and the value has been reduced accordingly." << endl;
        maxusers = MAX_USERS;
    }
    cout << "Server should automatically save changes: ";
    autosave = printGetBool(autosave);

    cout << "Enable file sharing: ";
    if(printGetBool(filesroot.length()))
    {
        ACE_TCHAR buff[1024] = {};
        ACE_OS::getcwd(buff, 1024);
#ifdef WIN32
        cout << "Directory for file storage, e.g. C:\\MyServerFiles: ";
#else
        cout << "Directory for file storage, e.g. /home/bill/srv1/files: ";
#endif
        filesroot = LocalToUnicode(printGetString(UnicodeToLocal(filesroot).c_str()).c_str());
        if(!filesroot.empty() && ACE_OS::chdir(filesroot.c_str()) == 0)
        {
            cout << "Disk quota (in KBytes) per channel, 0 = disabled: ";
            diskquota = printGetInt64(diskquota/1024)*1024;
            cout << "Maximum disk usage (in KBytes) for storing files: ";
            maxdiskusage = printGetInt64(maxdiskusage/1024)*1024;
        }
        else
        {
            cout << "Directory not found for file storage. File sharing disabled." << endl;
            filesroot.clear();
            diskquota = 0;
        }
        ACE_OS::chdir(buff);
    }
    else
    {
        filesroot.clear();
        diskquota = 0;
    }

    cout << "Log server activity: ";
    log_maxsize = printGetBool(log_maxsize != 0)? (log_maxsize!=0)? log_maxsize : -1 : 0;

    cout << "Server should bind to the following TCP port: ";
    tcpport = printGetInt(tcpport);
    cout << "Server should bind to the following UDP port: ";
    udpport = printGetInt(udpport);
    cout << "Bind to specific IP-addresses? (required for IPv6) ";
    if (printGetBool(bindips.size()))
    {
        while (true)
        {
            if (bindips.size())
            {
                cout << "Currently binding to IP-addresses:" << endl;
                for (auto ip : bindips)
                {
                    cout << "\t- " << (ip.empty()? "0.0.0.0" : ip) << endl;
                }
                
                cout << "Specify additional IP-addresses? ";
                if (!printGetBool(bindips.empty()))
                    break;
            }
            
            cout << "Specify the IP-address to bind to (IPv6 type \"::\" for all interfaces): " << endl;
            std::string ip = printGetString("0.0.0.0");
            bindips.push_back(ip);
        }
    }
    else
    {
        bindips.clear();
    }

#if defined(ENABLE_TEAMTALKPRO)
    cout << "Should server run in encrypted mode? ";
    if(printGetBool(certfile.length() && keyfile.length()))
    {
        cout << "Server certificate file (PEM file) for encryption: ";
        certfile = LocalToUnicode(printGetString(UnicodeToLocal(certfile).c_str()).c_str());
        if(ACE_OS::filesize(certfile.c_str())<=0)
            cerr << "File " << certfile << " not found! Continuing configuration..." << endl;
        cout << "Server private key file (PEM file) for encryption: ";
        keyfile = LocalToUnicode(printGetString(UnicodeToLocal(keyfile).c_str()).c_str());
        if(ACE_OS::filesize(keyfile.c_str())<=0)
            cerr << "File " << keyfile << " not found! Continuing configuration..." << endl;
    }
    else
    {
        certfile.clear();
        keyfile.clear();
    }

    bool encrypted = certfile.length() && keyfile.length();
#endif

    cout << endl << "User authentication." << endl;

    cout << endl;
    cout << "User account administration." << endl;
    int input = 0;
    enum UserAccountOptions {LIST_USERACCOUNTS = 1, CREATE_USERACCOUNT,
                             CREATE_USERACCOUNT_FACEBOOK,
#if defined(ENABLE_TEAMTALKPRO)
                             CREATE_USERACCOUNT_BEARWARE,
#endif
                             DELETE_USERACCOUNT, QUIT_USERACCOUNTS};
    ACE_CString url = WEBLOGIN_URL;
    std::string xml;

    while(input != QUIT_USERACCOUNTS)
    {
        UserAccount user;
        std::string tmp;
        int count = 0;
        while(xmlSettings.GetNextUser(count, user))count++;
        cout << endl;
        cout << "Currently there's " << count << " user accounts." << endl;
        cout << LIST_USERACCOUNTS << ") List user accounts." << endl;
        cout << CREATE_USERACCOUNT << ") Create new user account." << endl;
        cout << CREATE_USERACCOUNT_FACEBOOK << ") Create Facebook login account." << endl;
#if defined(ENABLE_TEAMTALKPRO)
        cout << CREATE_USERACCOUNT_BEARWARE << ") Create BearWare.dk web-login account." << endl;
#endif
        cout << DELETE_USERACCOUNT << ") Delete user account." << endl;
        cout << QUIT_USERACCOUNTS << ") Quit and proceed server configuration." << endl;
        cout << "Select option: ";
        switch( (input = printGetInt(QUIT_USERACCOUNTS)) )
        {
        case LIST_USERACCOUNTS :
            cout << endl;
            cout << "Active user accounts:" << endl << endl;
            count = 0;
            while(xmlSettings.GetNextUser(count, user))
            {
                cout << "User #" << count + 1 << endl;
                cout << "Username: " << user.username << endl;
                cout << "Password: " << user.passwd << endl;
                switch(user.usertype)
                {
                case USERTYPE_DEFAULT :
                    cout << "User type: Default user" << endl;
                    break;
                case USERTYPE_ADMIN :
                    cout << "User type: Administrator" << endl;
                    break;
                default :
                    cout << "User type: Unknown" << endl;
                }
                cout << endl;
                count++;
            }
            break;
        case CREATE_USERACCOUNT :
            cout << "Creating new user account." << endl;
            cout << "Type username: ";
            user.username = LocalToUnicode(printGetString("").c_str());
            cout << "Type password: ";
            user.passwd = LocalToUnicode(printGetString("").c_str());
            goto useraccountcfg;
#if defined(ENABLE_HTTP_AUTH)
        case CREATE_USERACCOUNT_FACEBOOK :
            cout << "Creating Facebook login account." << endl;
            user.username = ACE_TEXT( WEBLOGIN_FACEBOOK );
            user.passwd = ACE_TEXT("");
            cout << "Testing Facebook login service..." << endl;

            url += "client=" TEAMTALK_LIB_NAME;
            url += "&version=" TEAMTALK_VERSION;
            url += "&ping=true";
            switch(HttpRequest(url, xml))
            {
            case -1 :
                cout << "Failed to query " << WEBLOGIN_URL;
                break;
            case 0 :
                cout << "Invalid response from Facebook login service" << endl;
                break;
            case 1 :
                cout << "Got valid response from Facebook login service. Continuing..." << endl;
                break;
            }
            goto useraccountcfg;
#if defined(ENABLE_TEAMTALKPRO)
        case CREATE_USERACCOUNT_BEARWARE :

            if (!encrypted)
            {
                cout << "BearWare.dk web-login can only be used in encrypted mode." << endl;
                break;
            }

            cout << "Creating BearWare.dk web-login account." << endl;
            user.username = ACE_TEXT( WEBLOGIN_BEARWARE );
            user.passwd = ACE_TEXT("");
            cout << "Testing BearWare.dk web-login service..." << endl;

            url += "client=" TEAMTALK_LIB_NAME;
            url += "&version=" TEAMTALK_VERSION;
            url += "&ping=true";
            switch(HttpRequest(url, xml))
            {
            case -1 :
                cout << "Failed to query " << WEBLOGIN_URL;
                break;
            case 0 :
                cout << "Invalid response from BearWare.dk login service" << endl;
                break;
            case 1 :
                cout << "Got valid response from BearWare.dk login service. Continuing..." << endl;
                break;
            }
            goto useraccountcfg;
#endif /* ENABLE_TEAMTALKPRO */
#endif /* ENABLE_HTTP_AUTH */
        useraccountcfg:
            cout << "Available user types:" << endl;
            cout << "\t1. Default user." << endl;
            cout << "\t2. Administrator." << endl;
            cout << "Select user type:";
            switch(printGetInt(1))
            {
            case 2 :
                user.usertype = USERTYPE_ADMIN;
                break;
            case 1 :
            default :
            {
                user.usertype = USERTYPE_DEFAULT;
                int userrights = USERRIGHT_NONE;
                cout << "Should multiple users be allowed to log in with this user account? ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_MULTI_LOGIN?true:false)?
                    userrights | USERRIGHT_MULTI_LOGIN : userrights & ~USERRIGHT_MULTI_LOGIN;

                cout << "User can change nickname: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_LOCKED_NICKNAME?false:true)?
                    (userrights & ~USERRIGHT_LOCKED_NICKNAME) : (userrights | USERRIGHT_LOCKED_NICKNAME);

                cout << "User can see all other users on server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_VIEW_ALL_USERS?true:false)?
                    (userrights | USERRIGHT_VIEW_ALL_USERS) : (userrights & ~USERRIGHT_VIEW_ALL_USERS);

                cout << "User can create temporary channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_CREATE_TEMPORARY_CHANNEL?true:false)?
                    (userrights | USERRIGHT_CREATE_TEMPORARY_CHANNEL) : (userrights & ~USERRIGHT_CREATE_TEMPORARY_CHANNEL);

                cout << "User can create/modify all channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_MODIFY_CHANNELS?true:false)?
                    (userrights | USERRIGHT_MODIFY_CHANNELS) : (userrights & ~USERRIGHT_MODIFY_CHANNELS);

                cout << "User can broadcast text message to all users: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TEXTMESSAGE_BROADCAST?true:false)?
                    (userrights | USERRIGHT_TEXTMESSAGE_BROADCAST) : (userrights & ~USERRIGHT_TEXTMESSAGE_BROADCAST);

                cout << "User can kick users off the server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_KICK_USERS?true:false)?
                    (userrights | USERRIGHT_KICK_USERS) : (userrights & ~USERRIGHT_KICK_USERS);

                cout << "User can ban users from the server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_BAN_USERS?true:false)?
                    (userrights | USERRIGHT_BAN_USERS) : (userrights & ~USERRIGHT_BAN_USERS);

                cout << "User can move users between channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_MOVE_USERS?true:false)?
                    (userrights | USERRIGHT_MOVE_USERS) : (userrights & ~USERRIGHT_MOVE_USERS);

                cout << "User can make other users channel operator: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_OPERATOR_ENABLE?true:false)?
                    (userrights | USERRIGHT_OPERATOR_ENABLE) : (userrights & ~USERRIGHT_OPERATOR_ENABLE);

                cout << "User can upload files to channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_UPLOAD_FILES?true:false)?
                    (userrights | USERRIGHT_UPLOAD_FILES) : (userrights & ~USERRIGHT_UPLOAD_FILES);

                cout << "User can download files from channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_DOWNLOAD_FILES?true:false)?
                    (userrights | USERRIGHT_DOWNLOAD_FILES) : (userrights & ~USERRIGHT_DOWNLOAD_FILES);

                cout << "User can record conversations in channels that don't allow it: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_RECORD_VOICE?true:false)?
                    (userrights | USERRIGHT_RECORD_VOICE) : (userrights & ~USERRIGHT_RECORD_VOICE);

                cout << "User can update server properties: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_UPDATE_SERVERPROPERTIES?true:false)?
                    (userrights | USERRIGHT_UPDATE_SERVERPROPERTIES) : (userrights & ~USERRIGHT_UPDATE_SERVERPROPERTIES);

                cout << "User can transmit voice (microphone input) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_VOICE?true:false)?
                    (userrights | USERRIGHT_TRANSMIT_VOICE) : (userrights & ~USERRIGHT_TRANSMIT_VOICE);

                cout << "User can transmit video (webcam) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_VIDEOCAPTURE?true:false)?
                    (userrights | USERRIGHT_TRANSMIT_VIDEOCAPTURE) : (userrights & ~USERRIGHT_TRANSMIT_VIDEOCAPTURE);

                cout << "User can transmit desktop sharing packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_DESKTOP?true:false)?
                    (userrights | USERRIGHT_TRANSMIT_DESKTOP) : (userrights & ~USERRIGHT_TRANSMIT_DESKTOP);

                cout << "User can transmit remote desktop access packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_DESKTOPINPUT?true:false)?
                    (userrights | USERRIGHT_TRANSMIT_DESKTOPINPUT) : (userrights & ~USERRIGHT_TRANSMIT_DESKTOPINPUT);

                cout << "User can transmit audio file (wav, mp3 files) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO?true:false)?
                    (userrights | USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) : (userrights & ~USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO);

                cout << "User can transmit video file (avi, wmv files) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO?true:false)?
                    (userrights | USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) : (userrights & ~USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);

                user.userrights = userrights;

                cout << "Limit user's audio codec to a specific bit rate (in kbps), 0 = no limit: ";
                user.audiobpslimit = printGetInt(0)*1000;

                cout << "Limit number of commands user can issue (to prevent flooding)?";
                if(printGetBool(user.abuse.n_cmds && user.abuse.cmd_msec))
                {
                    cout << "Number of commands to allow within specified time frame: ";
                    user.abuse.n_cmds = printGetInt(user.abuse.n_cmds);
                    cout << "User can enter " << user.abuse.n_cmds << " commands within this number of msec: ";
                    user.abuse.cmd_msec = printGetInt(user.abuse.cmd_msec);
                }
            }
            break;
            }

            if(user.username.empty())
            {
                cout << "User account has no username. Create anonymous account? ";
                if(!printGetBool(false))
                    break;
            }

            xmlSettings.RemoveUser(UnicodeToUtf8(user.username).c_str());
            xmlSettings.AddNewUser(user);
            break;
        case DELETE_USERACCOUNT :
        {
            cout << "Type the username of the account to delete: ";
            tmp = printGetString("");
            ACE_TString utf8 = LocalToUnicode(tmp.c_str());
            if(xmlSettings.RemoveUser(UnicodeToUtf8(utf8).c_str()))
                cout << "1 user deleted." << endl;
            else
                cout << "User not found." << endl;
            break;
        }
        case QUIT_USERACCOUNTS :
            if(count == 0)
            {
                cout << "There's no active user account. No user will be able to log in!" << endl;
                cout << "Are you sure you want to exit user account";
                if(!printGetBool(false))
                    input = 0;
            }
            break;
        }
    }

    cout << endl;

    cout << "Maximum number of invalid login attempts before banning user, 0 = disabled: ";
    max_login_attempts = printGetInt(max_login_attempts);

    cout << "Maximum number of logins per IP-address, 0 = disabled: ";
    max_logins_per_ip = printGetInt(max_logins_per_ip);

    cout << "Delay in milliseconds before an IP-address can make another login, 0 = disabled: ";
    logindelay = printGetInt(logindelay);
    
    cout << endl << endl;
    cout << "Your " << TEAMTALK_NAME << " is now configured with the following settings:" << endl;
    cout << endl;
    cout << TEAMTALK_NAME << " name: " << servername << endl;
    cout << "Message of the Day: " << motd << endl;
    cout << "Maximum users allowed: " << maxusers << endl;
    cout << "Server will" << (autosave? "" : " not") << " automatically save changes." << endl;

    if(filesroot.length())
    {
        cout << "File sharing enabled." << endl;
        cout << "Root directory for root channel: " << filesroot << endl;
        cout << "Disk quota in bytes per channel: " << diskquota/1024 << " KBytes" << endl;
        cout << "Maximum disk usage for storing files: " << maxdiskusage/1024 << " KBytes" <<endl;
    }
    else
        cout << "File sharing disabled." << endl;

    if(log_maxsize != 0)
        cout << "Server logging enabled." << endl;
    else
        cout << "Server logging disabled." << endl;

    cout << "Server will bind to TCP port " << tcpport << endl;
    cout << "Server will bind to UDP port " << udpport << endl;
    cout << "Server will bind to IP-address: " << endl;
    for (auto ip : bindips)
        cout << "\t- " << ip << endl;

    cout << endl;
    if(max_login_attempts)
        cout << "Max incorrect login attempts before banning user: " << max_login_attempts << endl;
    else
        cout << "Max incorrect login attempts before banning user: " << "disabled" << endl;
    if(max_logins_per_ip)
        cout << "Max logins per IP-address: " << max_logins_per_ip << endl;
    else
        cout << "Max logins per IP-address: " << "disabled" << endl;

    cout << "Users wait for " << logindelay << " msec before attempting login again." << endl;
    
#if defined(ENABLE_TEAMTALKPRO)
    cout << "Server certificate file for encryption: " << certfile << endl;
    cout << "Server private key file for encryption: " << keyfile << endl;
#endif

    int count = 0;
    UserAccount user;
    while(xmlSettings.GetNextUser(count, user))count++;
    cout << "Active user accounts: " << count << endl;

    cout << endl;

    cout << "Save these settings? ";
    if(printGetBool(true))
    {
        xmlSettings.SetServerName(UnicodeToUtf8(servername).c_str());
        xmlSettings.SetMessageOfTheDay(UnicodeToUtf8(motd).c_str());
        xmlSettings.SetMaxUsers(maxusers);
        xmlSettings.SetAutoSave(autosave);
        xmlSettings.SetFilesRoot(UnicodeToUtf8(filesroot).c_str());
        xmlSettings.SetMaxDiskUsage(maxdiskusage);
        xmlSettings.SetDefaultDiskQuota(diskquota);
        xmlSettings.SetBindIPs(bindips);
        xmlSettings.SetHostTcpPort(tcpport);
        xmlSettings.SetHostUdpPort(udpport);
#if defined(ENABLE_TEAMTALKPRO)
        xmlSettings.SetCertificateFile(UnicodeToUtf8(certfile).c_str());
        xmlSettings.SetPrivateKeyFile(UnicodeToUtf8(keyfile).c_str());
#endif
        xmlSettings.SetServerLogMaxSize(log_maxsize);
        xmlSettings.SetMaxLoginAttempts(max_login_attempts);
        xmlSettings.SetMaxLoginsPerIP(max_logins_per_ip);
        xmlSettings.SetLoginDelay(logindelay);
        xmlSettings.SaveFile();

        cout << "Changes saved." << endl;
        cout << endl;
        cout << "If your " << TEAMTALK_NAME << " is currently running the settings will not take" << endl;
        cout << "effect until you restart the server." << endl;
        cout << endl;
    }
    else
        cout << "Changes discarded." << endl;
}
