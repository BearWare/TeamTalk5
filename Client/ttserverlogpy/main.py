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

print(getVersion())

ttclient = TeamTalk()

if ttclient.connect(b"127.0.0.1", 10443, 10443, 0, 0, True):
    print("Connecting")
else:
    sys.exit('Failed to issue connect')

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS)
if result:
    print("Connected successfully")
else:
    sys.exit("Failed to connect")

cmdid = ttclient.doLogin(b"Hello Python", b"guest", b"guest", b"python")

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN)
if result:
    print("Logged in successfully")
else:
    sys.exit("Failed to log in")

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE)
if result:
    print("Server name is: " + msg.serverproperties.szServerName.decode("utf-8"))
else:
    sys.exit("Failed to log in")

result, msg = waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW)
if result:
    print("New channel: " + msg.channel.szName.decode("utf-8"))
    print("Audio codec: " + str(msg.channel.audiocodec.nCodec))

result, msg = waitForCmdSuccess(ttclient, cmdid, DEF_WAIT)

if result:
    print("Login completed")
    
ttclient.closeTeamTalk()

