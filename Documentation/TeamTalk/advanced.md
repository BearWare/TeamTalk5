# Advanced Topics {#advanced}

The TeamTalk client supports a couple of command-line arguments and
additionals configuration options which are explain in the following
sections:

- [Command-line arguments](@ref cmdline)
- [Load settings file (TeamTalk5.ini)](@ref settingsfile)
- [Multiple client instances with different settings](@ref multiclients)
- [Extra INI-file settings](@ref inifile)

# Command line arguments {#cmdline}

The TeamTalk 5 client supports the following command-line arguments:

- TeamTalk URL
  - Usage:\verbatim TeamTalk5.exe tt://192.168.0.10?tcpport=10333&udpport=10333 \endverbatim

- TeamTalk .tt file
  - Usage:\verbatim TeamTalk5.exe C:\Documents\myserver.tt \endverbatim

- Add to Windows Firewall 
  - Usage:\verbatim TeamTalk5.exe -fwadd \endverbatim

- Remove from Windows Firewall 
  - Usage:\verbatim TeamTalk5.exe -fwremove \endverbatim

- Override auto-connect setting 
  - Usage:\verbatim TeamTalk5.exe -noconnect \endverbatim

- Create new or load existing ini-file
  - Usage:\verbatim TeamTalk5.exe -cfg c:\Documents\TeamTalk5.ini \endverbatim

# Load settings file (TeamTalk5.ini) {#settingsfile}

By default the TeamTalk client first looks in the directory of the
executable for its settings file (TeamTalk5.ini). If the settings file
isn't located there it will instead load its settings from the
user-profile's directory. On Windows 7 the user-profile directory is
typically located in:\verbatim
C:\Users\username\AppData\Roaming\endverbatim
and on Windows XP it's in\verbatim
C:\Documents and Settings\username\Application Data\endverbatim.
On Linux and Mac the settings are located in:\verbatim
$HOME/.config/BearWare.dk\endverbatim

To manually create/load TeamTalk5.ini use the\verbatim -cfg \endverbatim
command line argument, e.g.\verbatim TeamTalk5.exe -cfg MyIniFile.ini\endverbatim

# Multiple client instances with different settings {#multiclients}

Since the TeamTalk executable first tries to load settings from its
own directory it's possible to run multiple TeamTalk clients with
different settings. Simply create a new directory, copy TeamTalk5.exe
and TeamTalk5.dll to that directory and create a file called
TeamTalk5.ini (TeamTalk5Classic.xml in the Classic client). Launch
TeamTalk5.exe and it will use the local settings file instead of the
settings file in the user-profile.

# Extra INI-file settings {#inifile}

Most settings in the TeamTalk ini-file are configurable using the
[Preferences dialog](@ref preferencesdlg) but there's a few hidden
entries which are described here.

## Mute left or right audio channel

In the sound system section of the INI-file it's possible to mute
either the left or right channel when the client is in a stereo
channel. Simply add the following lines to the TeamTalk5.ini file:
\verbatim
[soundsystem]
sndoutput-mute-left=true
sndoutput-mute-right=true
\endverbatim

## Make an auto connect INI-file

To make an INI-file which automatically connects to a host once the
application is started simply add the following to the TeamTalk5.ini
file:
\verbatim
[latesthosts]
0_hostaddr=tt5eu.bearware.dk
0_tcpport=10333
0_udpport=10333
0_username=guest
0_password=guest
0_channel=/
0_chanpassword=

[connection]
autoconnect=true
\endverbatim
