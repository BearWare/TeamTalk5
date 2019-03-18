# TeamTalk Server Setup {#teamtalkserver}

This section explains how to set up a TeamTalk server. This is a quite
complicated procedure so users who are not so skilled with network
setup are advised to use the public TeamTalk servers which are
available in the [Connect to Server dialog](@ref connectdlg) (press
F2).

Here's an overview of this section:

- [TeamTalk Server for Windows](@ref winserver)
  - [TeamTalk Console Server](@ref consoleserver)
  - [Installing the TeamTalk Server as an NT service](@ref ntservice)
  - [Installing multiple TeamTalk NT Services](@ref multintservice)
- [TeamTalk Server for Linux](@ref tt5srvlinux)
- [TeamTalk Server for Mac OS X](@ref tt5srvmac)
- [TeamTalk Server with Facebook Login](@ref fbserver)

# TeamTalk Server for Windows {#winserver}

To be able to set up a TeamTalk server you first you need to ensure
that the TeamTalk server was installed when you ran the TeamTalk
installation.

![Install TeamTalk Server](install_server.png "Install TeamTalk Server component")

If the server is installed there will be a folder called "TeamTalk NT
Service" in the "TeamTalk 5" program group of Windows' start-menu.

## TeamTalk Console Server {#consoleserver}

To start the TeamTalk Console Server click "TeamTalk 5 Console Server"
in the TeamTalk 5 program group. This will bring up a console window
where you're asked if you want to configure the TeamTalk server.

Unlike the [TeamTalk NT service](@ref ntservice) the Windows console
server doesn't require administrator access because all settings for
the server will be saved in your home-directory:\verbatim
c:\Users\USERNAME\AppData\Roaming\BearWare.dk\tt5srv.xml
\endverbatim

## Installing the TeamTalk Server as an NT service {#ntservice}

To install the TeamTalk server click "Install TeamTalk NT Service" in
the TeamTalk 5 program group. On Windows this will bring up User
Access Control (UAC) since the TeamTalk server requires Administrator
access. The TeamTalk server will now run it's configuration wizard
(its "-wizard" option). If you want to change the server's default
settings or set up user accounts you need to run the configuration
wizard and answer 'y' to the first question.

Here is an example of how to set up a server with an administrator
user account. The user account has the username 'admin' and password
'admin'.

![Configure TeamTalk Server](serverconfig.png "Configure TeamTalk Server")

Once the server is up and running you can connect to the server using
the administrator user account that you just created. Use the
[user account dialog](@ref useraccountsdlg) to create more user accounts
for the server.


## Installing multiple TeamTalk NT Services {#multintservice}

To install multiple TeamTalk 5 NT Services the **sc.exe** command can
be used. Here is an example:\verbatim
sc.exe create "TeamTalk Server 2" binPath= "C:\MyServer\tt5svc.exe -wd c:\MyServer" start= auto
\endverbatim

The service in the above example will be called "TeamTalk Server 2" in
the NT service list. Notice the spaces after binPath and start. For
some strange reason these are required.

To start the above service run:\verbatim
sc.exe start "TeamTalk Server 2"
\endverbatim

To stop it type:\verbatim
sc.exe stop "TeamTalk Server 2"
\endverbatim

To uninstall it type:\verbatim
sc.exe delete "TeamTalk Server 2"
\endverbatim

# TeamTalk Server for Linux {#tt5srvlinux}

On Linux the TeamTalk server binary is called **tt5srv** and is 
located in the **server**-subfolder after untar'ing the archive.

The TeamTalk server for Linux can run both in daemon mode and as a
regular console application. A daemon script is also included which
can be put in /etc/init.d.

For instructions on how to configure and run the TeamTalk server
(tt5srv) simply type:\verbatim
./tt5srv
\endverbatim

To start the TeamTalk server setup-wizard type:\verbatim
./tt5srv -wizard
\endverbatim

When running the setup-wizard then make sure the executable is run at
a writable location so it's possible for the setup-wizard to save its
changes to disk.


# TeamTalk Server for Mac OS X {#tt5srvmac}

On Mac OS X the server binary is also called **tt5srv** and is located
in **Applications/TeamTalk5.app/Contents/Server** subfolder after
installing the TeamTalk .dmg file.

The TeamTalk server for Mac OS X can run both in daemon mode and as a regular
console application.

For instructions on how to configure and run the TeamTalk server
(tt5srv) simply type:\verbatim
/Applications/TeamTalk5.app/Contents/Server/tt5srv
\endverbatim

To start the TeamTalk server setup-wizard type:\verbatim
/Applications/TeamTalk5.app/Contents/Server/tt5srv -wizard
\endverbatim

When running the setup-wizard then make sure the executable is run at
a writable location so it's possible for the setup-wizard to save its
changes to disk.

# TeamTalk Server with Facebook Login {#fbserver}

If a TeamTalk server is set up with a user account that has the
username "facebook" then this will force the TeamTalk server to
authenticate the user's credentials with Facebook's login service. For
the TeamTalk server to do this it must have internet access and be
able to do DNS lookups.

Read section [Facebook Login](@ref fbuseraccount) for more information
on how to set up a Facebook account.

