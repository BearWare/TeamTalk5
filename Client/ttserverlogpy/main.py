from TeamTalk5 import *
import time
import sys

if sys.getdefaultencoding() != "utf-8":
    sys.exit("Only utf-8 is supported by this client")

timestamp = lambda: int(round(time.time() * 1000))
DEF_WAIT = 2000

def waitForEvent(ttclient, event, timeout = DEF_WAIT):
    msg = ttclient.getMessage(timeout)
    end = timestamp() + timeout
    while msg.nClientEvent != event:
        if timestamp() >= end:
            return False, TTMessage()
        msg = ttclient.getMessage(timeout)
        
    return True, msg

def waitForCmdSuccess(ttclient, cmdid, timeout):
    result = True
    while result:
        result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, timeout)
        if result and msg.nSource == cmdid:
            return result, msg

    return False, TTMessage()

print(ttstr(getVersion()))

ttclient = TeamTalk()

if ttclient.connect(ttstr("127.0.0.1"), 10333, 10333, 0, 0, False):
    print("Connecting")
else:
    sys.exit('Failed to issue connect')

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS)
if result:
    print("Connected successfully")
else:
    sys.exit("Failed to connect")

cmdid = ttclient.doLogin(ttstr("Hello Python"), ttstr("guest"), ttstr("guest"), ttstr("python"))

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN)
if result:
    print("Logged in successfully")
else:
    sys.exit("Failed to log in")

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE)
if result:
    print("Server name is: " + ttstr(msg.serverproperties.szServerName))
else:
    sys.exit("Failed to log in")

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW)
if result:
    print("New channel: " + ttstr(msg.channel.szName))
    print("Audio codec: " + str(msg.channel.audiocodec.nCodec))

result, msg = waitForCmdSuccess(ttclient, cmdid, DEF_WAIT)

print(ttstr(msg.channel.szName))

if result:
    print("Login completed")

cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), ttstr(""))

result, msg = waitForCmdSuccess(ttclient, cmdid, DEF_WAIT)
if result:
    print("Joined root channel")
else:
    sys.exit("Failed to join channel")
    
ttclient.closeTeamTalk()

