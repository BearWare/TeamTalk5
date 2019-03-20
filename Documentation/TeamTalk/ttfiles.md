# .tt Files and tt:// URLs for TeamTalk Servers {#ttfiles}

It is sometimes difficult to explain novice TeamTalk users how they
should connect to a TeamTalk server and join a channel. For this
reason it's possible to make a so-called .tt files and tt:// URLs which
automates this process.

A .tt file is a file which is associated with the TeamTalk
application, so when a users double-clicks the file it bring up
TeamTalk which will then connect to the server.

A tt:// URL is a link which can be put on a HTML page and once clicked
will launch TeamTalk which will connect to the server.

.tt files and tt:// URLs are explained the following sections:

- [Generate .tt File](@ref ttfile)
- [tt:// URL](@ref tturl)

# Generate .tt File {#ttfile}

To generate a .tt file bring up the Connect dialog by pressing
F2. Fill out the fields which are required to connect to the TeamTalk
server like in the example below:

![Connect dialog](dlg_connect.png "Connect dialog for .tt File")

When all the required information has been put in press the button
**Generate .tt File**. The Generate .tt File dialog will then appear
as shown below.

![Generate .tt File dialog](dlg_ttfile.png "Generate .tt File")

The *Generate .tt File* dialog allows users to specify additional
options which can be used to set up the client application of the user
who's launching the .tt file.

When the settings have been specified press **Save .tt File** to store
it.  Once the file has been stored on disk you can send the .tt file
to other users, so they can connect to the same server.

# tt:// URL {#tturl}

An <a href=".... tag on a HTML page can be used to launch the TeamTalk
application and make it connect to the server specified. Here is an
example:

\verbatim
<html>
    <title>My TeamTalk Server Website</title>
    <body>
    
        Click <a href="tt://tt5eu.bearware.dk?tcpport=10335&amp;udpport=10335&amp;username=guest&amp;password=guest">here</a> to join my TeamTalk Server.
        
    </body>
</html>
\endverbatim

A tt:// URL supports the following properties:

- tcpport
  - Server's TCP port

- udpport
  - Server's UDP port

- username
  - Username for user account

- password
  - Password for user account 

- channel
  - The channel to join after login 

- chanpasswd
  - Password of the channel to join

All the above properties are optional. If they're not specified the
TeamTalk client will connect using the default TCP and UDP
ports. Click here to try the above tt:// URL
