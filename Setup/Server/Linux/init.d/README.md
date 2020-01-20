# TeamTalk 5 System V service on Debian 9

Here's the instruction for installing the TeamTalk 5 daemon on Debian
9.

## Configure the TeamTalk 5 service

Edit ```tt5server``` and set up the variables for user and file
locations.

## Installing and starting the TeamTalk 5 service

Afterwards as root user, copy ```tt5server``` to ```/etc/init.d```.

As root user enable the TeamTalk service:

```systemctl enable tt5server```

As root user start the TeamTalk service:

```systemctl start tt5server```

## Stopping and uninstalling the TeamTalk 5 service

To stop the TeamTalk service:

```systemctl stop tt5server```

To uninstall the TeamTalk service:

```systemctl disable tt5server```
