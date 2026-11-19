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
#include "ServerConfig.h"
#include "ServerGuard.h"
#include "ServerUtil.h"
#include "ServerXML.h"
#include "UPnP.h"
#include "myace/MyACE.h"
#include "teamtalk/Commands.h"
#include "teamtalk/Log.h"
#include "teamtalk/server/Server.h"
#include "teamtalk/server/ServerNode.h"

#include <ace/High_Res_Timer.h>
#include <ace/Init_ACE.h>
#include <ace/NT_Service.h>
#include <ace/Select_Reactor.h>
#include <ace/Timer_Heap.h>

#include <array>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <utility>

#if !defined(WIN32)
#include <unistd.h>
#endif

using namespace std;
using namespace teamtalk;

class Service;

static int RunServer(
#if defined(BUILD_NT_SERVICE)
              Service* service
#endif
              );

static void PrintCommandArgs();

static int ParseArguments(int argc, tchar* argv[]
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

tstring GetWinError(DWORD err)
{
    tstring result;
    LPTSTR s = NULL;
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        err,
        0,
        reinterpret_cast<LPTSTR>(&s),
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
                       Utf8ToUnicode(TEAMTALK_DESCRIPTION.data()).c_str());

#endif

// overwritable options
static tstring bindip;
static tstring settingsfile;
static tstring logfile = Utf8ToUnicode(TEAMTALK_LOGFILE.data());
static tstring pidfile;
static tstring weblogin;
static tstring tokenlogin;
static int tcpport = 0;
static int udpport = 0;
static bool verbose = false;
static bool daemon_pid = false;
static bool daemon_mode = false;
static bool nondaemon = false;
static int rxloss = 0, txloss = 0;
static bool cleanfiles = false;

//setting files
static ServerXML xmlSettings(TEAMTALK_XML_ROOTNAME.data());
//log file
static std::ofstream logstream;
tstring TEAMTALK_NAME_UNICODE = Utf8ToUnicode(TEAMTALK_NAME.data());

class SignalEventHandler : public ACE_Event_Handler
{
public:
    SignalEventHandler(ACE_Reactor* r)
        : ACE_Event_Handler(r)
    {
    }

    int handle_signal(int signum, siginfo_t* /*unused*/,ucontext_t* /*unused*/) override
    {
        switch (signum)
        {
        case SIGINT :
        case SIGTERM :
            reactor()->end_reactor_event_loop();
            break;
        case SIGHUP :
            if (!LoadConfig(xmlSettings, settingsfile))
            {
                std::array<tchar, 1024> error_msg{};
                tsnprintf(error_msg.data(), error_msg.size(), TTEXT("Failed to reload settings file %s."), settingsfile.c_str());
                TT_SYSLOG(error_msg.data());
            }
            else
            {
                logstream.close();

                if (xmlSettings.GetServerLogMaxSize() != 0)
                {
                    logstream.open(logfile.c_str(), ios::app);
                }

                std::array<tchar, 1024> msg{};
                tsnprintf(msg.data(), msg.size(), TTEXT("Reloaded settings file %s."), settingsfile.c_str());
                TT_LOG(msg.data());
            }
        }
        return 0;
    }
};

int ACE_TMAIN(int argc, tchar* argv[])
{
    ACE::init();
    int exitcode = EXIT_FAILURE;

#if defined(BUILD_NT_SERVICE)
    Service service;
    service.reactor(ACE_Reactor::instance());
    service.name(TEAMTALK_NAME_UNICODE.c_str(), TEAMTALK_NAME_UNICODE.c_str());
    int parse_result = ParseArguments(argc, argv, &service);
    if (parse_result > 0)
    {
        ACE_NT_SERVICE_RUN(TeamTalk,
            &service,
            rett);
        // 'rett' gets result of StartServiceCtrlDispatcherW()
        exitcode = rett ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else if (parse_result == 0)
        exitcode = EXIT_SUCCESS;
#else
    if(argc <= 1)
    {
        PrintCommandArgs();
        exitcode = EXIT_SUCCESS;
    }
    else
    {
        int const parse_result = ParseArguments(argc, argv);
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

static void RunEventLoop(ACE_Reactor* tcpReactor, ACE_Reactor* udpReactor,
                  const tstring& workdir)
{
    int const ret = ACE_Thread_Manager::instance ()->spawn(EventLoop, udpReactor);
    if(ret < 0)
        TT_LOG(TTEXT("Failed to spawn UDP reactor."));

    SyncReactor(*udpReactor);

    int log_check = 0;
    int upnp_check = 0;
    ACE_Time_Value tm(10,0);
    while (tcpReactor->handle_events(tm) >= 0)
    {
        if (++log_check % 10 == 0 && xmlSettings.GetServerLogMaxSize() > 0 &&
            logstream.tellp() >= xmlSettings.GetServerLogMaxSize())
        {
            RotateLogfile(workdir, logfile.c_str(), logstream);
        }

        if (xmlSettings.GetUPnP() && ++upnp_check >= 60)
        {
            upnp_check = 0;
            if (!UPnP_RenewPortMapping(tcpport, udpport))
            {
                UPnP_RemovePortMapping(tcpport, udpport);
                std::string externalIP;
                UPnP_AddPortMapping(tcpport, udpport, externalIP);
            }
        }

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
    ACE_Sig_Action no_sigpipe (static_cast<ACE_SignalHandler>(SIG_IGN));
    ACE_Sig_Action original_action;
    no_sigpipe.register_action (SIGPIPE, &original_action);

    int const ret = ACE::set_handle_limit(-1);//client handler (must be BIG)
    ACE_Timer_Heap timerheap;
    timerheap.set_time_policy(&ACE_High_Res_Timer::gettimeofday_hr);
    ACE_Select_Reactor selectReactor(nullptr, &timerheap);
    ACE_Reactor tcpReactor(&selectReactor);
    ACE_Reactor::instance(&tcpReactor);
    ACE_Reactor::instance()->owner (ACE_OS::thr_self ());

    ACE_Reactor udpReactor;

#if defined(BUILD_NT_SERVICE)
    service->reactor(ACE_Reactor::instance());
#endif
    SignalEventHandler signalHandler(ACE_Reactor::instance());
    ACE_Reactor::instance()->register_handler(SIGTERM, &signalHandler);
    ACE_Reactor::instance()->register_handler(SIGINT, &signalHandler);
    ACE_Reactor::instance()->register_handler(SIGHUP, &signalHandler);
#if defined(WIN32)
    ACE_Reactor::instance()->register_handler(SIGBREAK, &signalHandler);
#endif

    //init logger
#if defined(BUILD_NT_SERVICE)
    ACE_LOG_MSG->open(TEAMTALK_NAME_UNICODE.c_str(), ACE_Log_Msg::SYSLOG | ACE_Log_Msg::OSTREAM);
#else
    ACE_LOG_MSG->open(TEAMTALK_NAME_UNICODE.c_str(), ACE_Log_Msg::SYSLOG | ACE_Log_Msg::OSTREAM);
#endif

    std::error_code ec;
    tstring workdir = std::filesystem::current_path(ec).native().c_str();

    //enable logging
    if (xmlSettings.GetServerLogMaxSize() != 0)
    {
        logstream.open(logfile.c_str(), ios::app);
        ACE_OSTREAM_TYPE * output = &logstream;
        ACE_LOG_MSG->msg_ostream(output, false);
        ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
        u_long const flag = LM_ERROR | LM_INFO | LM_DEBUG;
        ACE_LOG_MSG->priority_mask(flag, ACE_Log_Msg::PROCESS);
    }

    //create listener
    ServerGuard srvguard(xmlSettings);
    ServerNode servernode(Utf8ToUnicode( TEAMTALK_VERSION ), &tcpReactor, &tcpReactor, &udpReactor, &srvguard);

    ServerSettings prop = servernode.GetServerProperties();

    statchannels_t channels;
    if(!ReadServerProperties(xmlSettings, prop, channels))
    {
        std::array<tchar, 1024> error_msg{};
        tsnprintf(error_msg.data(), error_msg.size(),
                         TTEXT("Failed to read server properties from settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg.data());
        return -1;
    }

    prop.rxloss = rxloss;
    prop.txloss = txloss;

    //check for override options
    if (tcpport > 0)
    {
        ACE_INET_Addr const addr(static_cast<u_short>(tcpport), (!prop.tcpaddrs.empty()) ? prop.tcpaddrs[0].get_host_addr() : nullptr);
        prop.tcpaddrs.clear();
        prop.tcpaddrs.push_back(addr);
    }
    else
    {
        tcpport = prop.tcpaddrs.front().get_port_number();
    }

    if (udpport > 0)
    {
        ACE_INET_Addr const addr(static_cast<u_short>(udpport), (!prop.udpaddrs.empty()) ? prop.udpaddrs[0].get_host_addr(): nullptr);
        prop.udpaddrs.clear();
        prop.udpaddrs.push_back(addr);
    }
    else
    {
        udpport = prop.udpaddrs.front().get_port_number();
    }

    if(!bindip.empty())
    {
        ACE_INET_Addr const tcpaddr((!prop.tcpaddrs.empty()) ? prop.tcpaddrs[0].get_port_number() : tcpport, bindip.c_str());
        prop.tcpaddrs.clear();
        prop.tcpaddrs.push_back(tcpaddr);
        ACE_INET_Addr const udpaddr((!prop.udpaddrs.empty()) ? prop.udpaddrs[0].get_port_number() : udpport, bindip.c_str());
        prop.udpaddrs.clear();
        prop.udpaddrs.push_back(udpaddr);
    }

    if (!ConfigureServer(servernode, prop, channels))
    {
        std::array<tchar, 1024> error_msg{};
        tsnprintf(error_msg.data(), error_msg.size(),
                         TTEXT("Failed to configure the server from settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg.data());
        return -1;
    }
    channels.clear();

    bool encrypted = false;
#if defined(ENABLE_TEAMTALKPRO)
    encrypted = (!xmlSettings.GetCertificateFile().empty()) && (!xmlSettings.GetPrivateKeyFile().empty());
    if (encrypted && !SetupEncryption(servernode, xmlSettings))
        return -1;
#endif

    tstring systemid = SERVER_WELCOME;
#if defined(ENABLE_TEAMTALKPRO)
    systemid = Utf8ToUnicode(xmlSettings.GetSystemID(UnicodeToUtf8(systemid).c_str()).c_str());
    if (!servernode.StartServer(encrypted, systemid))
#else
    if (!servernode.StartServer(false, systemid))
#endif
    {
        std::array<tchar, 1024> error_msg{};
        if(bindip.empty())
            tsnprintf(error_msg.data(), error_msg.size(),
            TTEXT("Unable to launch server using TCP port %d UDP port %d.\n")
            TTEXT("Make sure the ports are not currently in use."),
            prop.tcpaddrs[0].get_port_number(), prop.udpaddrs[0].get_port_number());
        else
            tsnprintf(error_msg.data(), error_msg.size(),
            TTEXT("Unable to launch server using IP-address %s TCP port %d UDP port %d.\n")
            TTEXT("Make sure the ports are not currently in use."),
            prop.tcpaddrs[0].get_host_addr(), prop.tcpaddrs[0].get_port_number(),
            prop.udpaddrs[0].get_port_number());

        TT_SYSLOG(error_msg.data());
        return -1;
    }

    if (xmlSettings.GetUPnP())
    {
        std::string externalIP;
        if (UPnP_AddPortMapping(tcpport, udpport, externalIP))
        {
            std::array<tchar, 256> msg{};
            tsnprintf(msg.data(), msg.size(),
                TTEXT("UPnP: Ports TCP %d, UDP %d forwarded. External IP: %s"),
                tcpport, udpport, externalIP.c_str());
            TT_LOG(msg.data());
        }
        else
        {
            TT_SYSLOG(TTEXT("UPnP: Failed to add port mappings. Check if your router supports UPnP."));
        }
    }

    std::string startmsg = std::string("Started {} v.{}.") + std::string(TEAMTALK_NAME) + TEAMTALK_VERSION;
    TT_LOG(Utf8ToUnicode(startmsg.c_str()));
    if(!verbose)
        ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);

    //don't write server events to syslog
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::SYSLOG);

#if defined(BUILD_NT_SERVICE)
    SetConsoleCtrlHandler(ControlHandler, TRUE);
    service->report_status_foo(SERVICE_RUNNING);
    RunEventLoop(ACE_Reactor::instance(), &udpReactor, workdir);
#else
    if(daemon_mode)
    {
#if defined(WIN32)
        cout << "Windows does not support daemon mode. Use -nd parameter instead." << endl;
        return -1;
#else
        pid_t pid;
        pid_t sid;

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0)
        {
            auto failmsg = TTEXT("Failed to start ") + TEAMTALK_NAME_UNICODE;
            TT_SYSLOG(failmsg.c_str());
            exit(EXIT_FAILURE);
        }

        /* If we got a good PID, then
        we can exit the parent process. */
        if (pid > 0)
        {
            if(daemon_pid)
                cout << TEAMTALK_NAME << " daemon got PID " << pid << endl;

            if(!pidfile.empty())
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

        RunEventLoop(ACE_Reactor::instance(), &udpReactor, workdir);

#endif /* WIN32 */
    }
    else if(nondaemon)
    {
        //TCP commands thread
        RunEventLoop(ACE_Reactor::instance(), &udpReactor, workdir);
    }
#endif /* BUILD_NT_SERVICE */

    ACE_LOG_MSG->set_flags(ACE_Log_Msg::STDERR);

    if (xmlSettings.GetUPnP())
    {
        UPnP_RemovePortMapping(tcpport, udpport);
    }

    servernode.StopServer();
    tstring stopmsg = tstring(TTEXT("Stopped {}.")) + TEAMTALK_NAME_UNICODE;
    TT_LOG(stopmsg.c_str());

    ACE_Thread_Manager::instance ()->wait ();

    udpReactor.close();
    tcpReactor.close();

    ACE::fini();

    return 0;
}

int ParseArguments(int argc, tchar* argv[]
#if defined(BUILD_NT_SERVICE)
              , Service* service
#endif
)
{
    std::map<tstring,tstring> args;

    for(int i=0;i<argc;i++)
    {
        tstring const str(argv[i]);
        pair<tstring,tstring> newPair;
        newPair.first = str;
        if ((str == TTEXT("-wd") ||
            str == TTEXT("-tcpport") ||
            str == TTEXT("-udpport") ||
            str == TTEXT("-ip") ||
            str == TTEXT("-c") ||
            str == TTEXT("-l") ||
            str == TTEXT("-pid-file") ||
            str == TTEXT("-rxloss") ||
            str == TTEXT("-txloss") ||
            str == TTEXT("-weblogin") ||
            str == TTEXT("-tokenlogin")))
        {
            if(i+1 >= argc)
            {
                cerr << "Missing option for parameter " << UnicodeToLocal(str) << endl;
                return 0;
            }

            newPair.second = argv[i+1];
            i++;
        }
        else
        {
            newPair.second = TTEXT("");
        }
        args.insert(newPair);
    }

    std::map<tstring,tstring>::iterator ite;

    if(args.contains(TTEXT("--version")))
    {
        cout << TEAMTALK_NAME << " version " << TEAMTALK_VERSION_FRIENDLY << endl;
        return 0;
    }
    if(args.contains(TTEXT("--help")) ||
        args.contains(TTEXT("-help")) ||
        args.contains(TTEXT("/help")) ||
        args.contains(TTEXT("-h")) ||
        args.contains(TTEXT("/h")) ||
        args.contains(TTEXT("-?")) ||
        args.contains(TTEXT("/?"))
        )
    {
        PrintCommandArgs();
        return 0;
    }
    if(args.contains(TTEXT("-d")))
    {
        daemon_mode = true;
    }
    if(args.contains(TTEXT("-nd")))
    {
        nondaemon = true;
    }
    if(args.contains(TTEXT("-daemon-pid")))
    {
        daemon_pid = true;
    }
    if((ite = args.find(TTEXT("-pid-file"))) != args.end())
    {
        pidfile = (*ite).second.c_str();
    }
    if( (ite = args.find(TTEXT("-tcpport"))) != args.end())
    {
        tcpport = int(String2I((*ite).second.c_str()));
    }
    if( (ite = args.find(TTEXT("-udpport"))) != args.end())
    {
        udpport = int(String2I((*ite).second.c_str()));
    }
    if( (ite = args.find(TTEXT("-ip"))) != args.end())
    {
        bindip = (*ite).second;
    }
    if( (ite = args.find(TTEXT("-verbose"))) != args.end())
    {
        verbose = true;
    }
    if( (ite = args.find(TTEXT("-rxloss"))) != args.end())
    {
        rxloss = static_cast<int>(String2I((*ite).second));
    }
    if( (ite = args.find(TTEXT("-txloss"))) != args.end())
    {
        txloss = static_cast<int>(String2I((*ite).second));
    }
    if( (ite = args.find(TTEXT("-wd"))) != args.end())
    {
        tstring const workdir = (*ite).second;
        std::error_code ec;
        std::filesystem::current_path(workdir.c_str(), ec);
        if(ec)
        {
            std::array<tchar, 1024> error_msg{};
            tsnprintf(error_msg.data(), error_msg.size(), TTEXT("%s failed to change to directory \"%s\"."),
                             TEAMTALK_NAME_UNICODE.c_str(), workdir.c_str());
            TT_SYSLOG(error_msg.data());
            return -1;
        }
    }
    if( (ite = args.find(TTEXT("-c"))) != args.end())
    {
        settingsfile = ite->second;
    }
    if ((ite = args.find(TTEXT("-weblogin"))) != args.end())
    {
        weblogin = ite->second;
    }
    if ((ite = args.find(TTEXT("-tokenlogin"))) != args.end())
    {
        tokenlogin = ite->second;
    }
    if( (ite = args.find(TTEXT("-l"))) != args.end())
    {
        logfile = ite->second;
    }
#if defined(BUILD_NT_SERVICE)
    else
    {
        //load config file from same directory as
        //executable file when running as NT service.
        std::array<tchar, MAX_PATH> bufPath{};
        if(GetModuleFileName(NULL, bufPath.data(), MAX_PATH)>0)
        {
            TCHAR* pChr = ACE_OS::strrchr(bufPath.data(), '\\');
            TTASSERT(pChr);
            if(pChr)
            {
                pChr++;
                *pChr = 0;
                std::error_code ec;
                std::filesystem::current_path(bufPath.data(), ec);
            }
            settingsfile = bufPath.data();
            settingsfile += Utf8ToUnicode(TEAMTALK_SETTINGSFILE.data());
        }
    }
    //pick out options related to NT services

    if( (ite = args.find(TTEXT("-i"))) != args.end())
    {
        if (service->insert(SERVICE_AUTO_START) < 0)
        {
            std::array<tchar, 1024> error_msg{};
            tsnprintf(error_msg.data(), error_msg.size(), TTEXT("Failed to install %s\nError: %s"),
                         TEAMTALK_NAME_UNICODE.c_str(), GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg.data());
            return -1;
        }
        else
        {
            std::array<tchar, 1024> success_msg{};
            tsnprintf(success_msg.data(), success_msg.size(), TTEXT("%s installed successfully."),
                TEAMTALK_NAME_UNICODE.c_str());
            TT_LOG(success_msg.data());
        }
        return 0;
    }
    if( (ite = args.find(TTEXT("-u"))) != args.end())
    {
        if (service->remove() < 0)
        {
            std::array<tchar, 1024> error_msg{};
            tsnprintf(error_msg.data(), error_msg.size(), TTEXT("Failed to remove %s\nError: %s"),
                TEAMTALK_NAME_UNICODE.c_str(), GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg.data());
            return -1;
        }
        else
        {
            std::array<tchar, 1024> success_msg{};
            tsnprintf(success_msg.data(), success_msg.size(), TTEXT("%s uninstalled successfully."),
                TEAMTALK_NAME_UNICODE.c_str());
            TT_LOG(success_msg.data());
        }
        return 0;
    }
    if( (ite = args.find(TTEXT("-s"))) != args.end())
    {
        if (service->start_svc() < 0)
        {
            std::array<tchar, 1024> error_msg{};
            tsnprintf(error_msg.data(), error_msg.size(), TTEXT("Failed to start %s\nError: %s"),
                TEAMTALK_NAME_UNICODE.c_str(), GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg.data());
            return -1;
        }
        else
        {
            std::array<tchar, 1024> success_msg{};
            tsnprintf(success_msg.data(), success_msg.size(), TTEXT("%s started successfully."),
                TEAMTALK_NAME_UNICODE.c_str());
            TT_LOG(success_msg.data());
        }
        return 0;
    }
    if( (ite = args.find(TTEXT("-e"))) != args.end())
    {
        if (service->stop_svc() < 0)
        {
            std::array<tchar, 1024> error_msg{};
            tsnprintf(error_msg.data(), error_msg.size(), TTEXT("Failed to stop %s\nError: %s"),
                         TEAMTALK_NAME_UNICODE.c_str(), GetWinError(ACE_OS::last_error()).c_str());
            TT_SYSLOG(error_msg.data());
            return -1;
        }
        else
        {
            std::array<tchar, 1024> success_msg{};
            tsnprintf(success_msg.data(), success_msg.size(), TTEXT("%s stopped successfully."),
                TEAMTALK_NAME_UNICODE.c_str());
            TT_LOG(success_msg.data());
        }
        return 0;
    }
#endif

    if(!LoadConfig(xmlSettings, settingsfile))
    {
        std::array<tchar, 1024> error_msg{};
        tsnprintf(error_msg.data(), error_msg.size(), TTEXT("Failed to load settings file %s."), settingsfile.c_str());
        TT_SYSLOG(error_msg.data());
        return -1;
    }

    RemoveFacebookLogins(xmlSettings);

#if defined(ENABLE_TEAMTALKPRO)
    // -tokenlogin foo@bearware.dk:fedeabe0912345
    if (!tokenlogin.empty())
    {
        const tstring rgxtokenlogin = TTEXT("^(.*):(.*)$");
#if defined(UNICODE)
        std::wsmatch sm;
        std::wstring const tokenstr = tokenlogin.c_str();
#else
        std::smatch sm;
        std::string const tokenstr = tokenlogin.c_str();
#endif
        if (std::regex_search(tokenstr, sm, BuildRegex(rgxtokenlogin.c_str())) && sm.size() == 3)
        {
            tstring const loginid = sm[1].str().c_str();
            tstring const newtoken = sm[2].str().c_str();
            if (AuthBearWareAccount(loginid, newtoken) != WEBLOGIN_SUCCESS)
            {
                TT_SYSLOG(TTEXT("Failed to authenticate BearWare.dk WebLogin."));
                return 0;
            }

            xmlSettings.SetBearWareWebLogin(UnicodeToUtf8(loginid).c_str(), UnicodeToUtf8(newtoken).c_str());
            xmlSettings.SaveFile();
            TT_SYSLOG(TTEXT("BearWare.dk WebLogin succedded. Token stored."));
        }
        else
        {
            TT_SYSLOG(TTEXT("Invalid format for TOKEN. Should be \"bear_dk@bearware.dk:aefd43fea\""));
            return 0;
        }
    }

    // -weblogin foo@bearware.dk:mysecretpassword
    if (!weblogin.empty())
    {
        const tstring rgxweblogin = TTEXT("^(.*):(.*)$");
#if defined(UNICODE)
        std::wsmatch sm;
        std::wstring webloginstr = weblogin.c_str();
#else
        std::smatch sm;
        std::string const webloginstr = weblogin.c_str();
#endif
        if (std::regex_search(webloginstr, sm, BuildRegex(rgxweblogin.c_str())) && sm.size() == 3)
        {
            tstring const loginid = sm[1].str().c_str();
            tstring const passwd = sm[2].str().c_str();
            tstring bearwareid, token;
            if (LoginBearWareAccount(loginid, passwd, token, bearwareid) != WEBLOGIN_SUCCESS)
            {
                TT_SYSLOG(TTEXT("Failed to authenticate BearWare.dk WebLogin."));
                return 0;
            }

            xmlSettings.SetBearWareWebLogin(UnicodeToUtf8(bearwareid).c_str(), UnicodeToUtf8(token).c_str());
            xmlSettings.SaveFile();
            TT_SYSLOG(TTEXT("BearWare.dk WebLogin succedded. Token stored."));
        }
        else
        {
            TT_SYSLOG(TTEXT("Invalid format for AUTH. Should be \"bear_dk:mysecretpassword\""));
            return 0;
        }
    }
#endif /*TEAMTALK_PRO */

    bool skipstart_output = false;

    if( (ite = args.find(TTEXT("-wizard"))) != args.end())
    {
        RunWizard(xmlSettings);
        skipstart_output = true;
    }

    if (args.contains(TTEXT("-cleanfiles")))
    {
        RemoveUnusedFiles(xmlSettings);
        skipstart_output = true;
    }

#if !defined(BUILD_NT_SERVICE)
    if(!nondaemon && !daemon_mode)
    {
        if (!skipstart_output)
            TT_LOG(TTEXT("Missing either -d or -nd parameter in order to start."));
        return 0;
    }
#endif

#if defined(ENABLE_TEAMTALKPRO)
    while (!LoginBearWare(xmlSettings))
    {
        TT_LOG(TTEXT("Failed to log on using BearWare.dk WebLogin."));
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
    cout << "Copyright (c) 2002-2025, BearWare.dk" << endl;
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
#if defined(ENABLE_TEAMTALKPRO)
    cout << "  -weblogin [AUTH] Setup BearWare.dk WebLogin using credentials" << endl;
    cout << "                   specified in AUTH." << endl;
    cout << "                   AUTH format is \"username:mysecretpassword\"" << endl;
    cout << "                   where colon is separator." << endl;
    cout << "  -tokenlogin [TOKEN]" << endl;
    cout << "                   Setup BearWare.dk WebLogin using token" << endl;
    cout << "                   specified in TOKEN." << endl;
    cout << "                   TOKEN format is \"username:03abc6dfe\"" << endl;
    cout << "                   where colon is separator." << endl;
#endif
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
    cout << "  -cleanfiles      Remove files that are not referenced by any channel." << std::endl;
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
    cout << "Kind regards go to the people behind the ACE Framework, miniupnpc, OpenSSL" << endl;
    cout << "and TinyXML2 projects!" << endl;
    cout << endl;
    cout << "Report bugs to contact@bearware.dk" << endl;
    cout << endl;
}
