from TeamTalk5 import *

print(getVersion())

ttclient = TeamTalk()

if ttclient.connect(b"tt4eu.bearware.dk", 10443, 10443, 0, 0, True):
    print("Connecting")
else:
    sys.exit('Failed to issue connect')

msg = ttclient.getMessage(5000)

if msg.nClientEvent == ClientEvent.CLIENTEVENT_CON_SUCCESS:
    print("Connected successfully")
else:
    sys.exit("Failed to connect")

ttclient.closeTeamTalk()
