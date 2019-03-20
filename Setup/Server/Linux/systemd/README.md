# TeamTalk 5 systemd service for Debian 9

Here's the instruction for installing the TeamTalk 5 daemon on Debian
9.

## Configure the TeamTalk 5 service

Edit ```tt5server.service``` and set up the paths to binary,
xml-configuration and log file. Basically the line starting with
```ExecStart=```.

## Installing and starting the TeamTalk 5 service

Afterwards as root user, copy ```tt5server.service``` to ```/etc/systemd/system```.

As root user enable the TeamTalk service:

```systemctl enable tt5server```

As root user start the TeamTalk service:

```systemctl start tt5server```

## Stopping and uninstalling the TeamTalk 5 service

To stop the TeamTalk service:

```systemctl stop tt5server```

To uninstall the TeamTalk service:

```systemctl disable tt5server```
