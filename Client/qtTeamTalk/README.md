# TeamTalk 5 client application based on [Qt Framework](http://www.qt.io)

To try a compiled version of the this application go [here](http://bearware.dk/?page_id=327).

## Build dependencies on Debian 9

The following dependencies must be installed in order to
build the TeamTalk Qt Client application:

* qt5-default
* qtbase5-dev
* libqt5x11extras5-dev
* qtmultimedia5-dev
* qtwebengine5-dev
* g++

Afterwards run the following command:

```# qmake teamtalk5.pro```

Followed by:

```# make```

## Build dependencies on Ubuntu 16 xenial

The following dependencies must be installed in order to
build the TeamTalk Qt Client application:

* qt5-default
* libqt5x11extras5-dev
* qtmultimedia5-dev
* libqt5webkit5-dev
* g++

Afterwards run the following command:

```# qmake CONFIG+=nowebengine teamtalk5.pro```

Followed by:

```# make```

## Build dependencies on Ubuntu 18 cosmic

The following dependencies must be installed in order to
build the TeamTalk Qt Client application:

* qt5-default
* libqt5x11extras5-dev
* qtmultimedia5-dev
* qtwebengine5-dev
* g++

Afterwards run the following command:

```# qmake teamtalk5.pro```

Followed by:

```# make```
