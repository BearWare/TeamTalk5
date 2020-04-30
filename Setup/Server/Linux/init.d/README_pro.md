# TeamTalk 5 Professional System V service on Debian 9

Here's the instruction for installing the TeamTalk 5 Pro daemon on
Debian 9.

## Configure the TeamTalk 5 Pro service

Edit ```tt5proserver``` and set up the variables for user and file
locations.

## Installing and starting the TeamTalk 5 service

Afterwards as root user, copy ```tt5proserver``` to ```/etc/init.d```.

As root user enable the TeamTalk service:

```systemctl enable tt5proserver```

As root user start the TeamTalk service:

```systemctl start tt5proserver```

## Stopping and uninstalling the TeamTalk 5 service

To stop the TeamTalk service:

```systemctl stop tt5proserver```

To uninstall the TeamTalk service:

```systemctl disable tt5proserver```
