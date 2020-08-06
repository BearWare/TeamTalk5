# TeamTalk Preferences {#preferences} #

TeamTalk has many options which can be configured in the application's
Preferences. To bring up Preferences simply hit F4.

The Preferences dialog has the following tabs:

- [General](@ref generaltab)
- [Display](@ref displaytab)
- [Connection](@ref connectiontab)
- [Sound System](@ref soundsystemtab)
- [Sound Events](@ref soundeventstab)
- [Shortcuts](@ref shortcutstab)
- [Video Capture](@ref videocapturetab)

The items in each of the tabs are explained in the following sections.

# General {#generaltab}

The *General-tab* is for basic settings like nickname and how to
transmit audio.

![General-tab in Preferences](pref_general.png "General-tab")

- **Nickname**
  - The name other users will see as you when logging on to a server.
  
- **Gender**
  - This option selects whether other users should see you as a Male
    or Female.

- **BearWare.dk Web Login ID**
  - A BearWare.dk web login is required if a TeamTalk server specifies
    that you must [log on with the username \"bearware\"](@ref connectdlg).
	You can create a BearWare.dk web login by clicking
    the *Activate* button.

- **Set away status after...**
  - If non-zero this option will automatically set you as away after
    the specified number of seconds.
   
- **Push To Talk**
  - A so-called Push To Talk key combination can be set up so one has
    to hold down a set of keys in order to transmit audio data. This is
    especially useful to avoid echos from speakers.
    
- **Voice activated**
  - Instead of using a Push To Talk key combination one can simply
    have TeamTalk start transmitting whenever you're talking by
    enabling this option.

# Display {#displaytab}

The *Display*-tab contains settings about the application's
appearance.

![Display-tab in Preferences](pref_display.png "Display-tab")

The items in the Display-tab are explained here:

- **User interface language**
  - The language to use in the application.
  
- **Start minimized**
  - If enabled TeamTalk will minimized itself once it starts.
  
- **Minimize to tray icon**
  - If enabled TeamTalk will go in Windows system tray when
    minimized. The system tray is in the bottom right corner of
    Windows.
    
- **Always on top**
  - If enabled TeamTalk will always be in front of other running
    programs.
    
- **Show number of users in channel**
  - If enabled a parentesis will be shown on each channel with the
    number of users in the channel.
    
- **Popup dialog when receiving text message**
  - If enabled a dialog window will pop up if someone writes you a
    text message.
    
- **Start video in popup dialog**
  - When a user starts transmitting video (from webcam) to the video
    session should be displayed in a separate dialog instead of in the
    [Video-tab](@ref videotab).

- **Closed video dialog should return to video-tab**
  - By default closing a video popup dialog will return it to the
    [Video-tab](@ref videotab). If the video should simply no longer
    be displayed after closing a video dialog then uncheck this
    option.

- **Start desktops in popup dialog**
  - When a user shares a desktop window it should be displayed in a
    separate dialog instead of the [Desktop-tab](@ref desktoptab).
    
- **Timestamp text messages**
  - If enabled each text message will have a timestamp telling when it
    was received.
  
- **Show statusbar events in chat-window**
  - If enabled all messages which are shown in the application's
    statusbar will also be put in the channel [chat-tab
    window](@ref chattab).
    
- **Check for program updates at start up**
  - When TeamTalk starts it should check to see if there's a new
    version available. If a new version is available a text message
    will be shown in the [Chat-tab](@ref chattab).

- **Show source in corner of video window**
  - If enabled the name of the person who is sending video will be
    shown in corner of the video window.

- **Maximum text length in channel list**
  - The names of users and channels can sometimes be very long. Use
    this option to limit the length names.

# Connection {#connectiontab}

The *Connection*-tab is used for configuring how the client should
connect to a server.

![Connection-tab in Preferences](pref_connection.png "Connection-tab")

The items in the Connection-tab are explained here:

- **Connect to latest host on startup**
  - If enabled the server which TeamTalk was connected to last it was
    running will be used again when TeamTalk starts.

- **Reconnect on connection dropped**
  - If the connection to a server is lost TeamTalk should
    automatically reconnect to the server.
  
- **Join root channel upon connection**
  - If enabled TeamTalk will automatically join the root channel on
    the server after authentication.

- **Query server's maximum payload upon connection**
  - Some networks restrict the size of UDP-packets which can make
    cause audio, video and desktop sessions not to work
    properly. Enabling this option will make TeamTalk query the
    maximum size the server allows for UDP-packets. TeamTalk
    accomplish this by sending several "bogus" UDP-packets with
    different sizes to the server in order to figure out how big
    UDP-packets are allowed.
  
- **Add application to Windows Firewall exception list**
  - The Windows Firewall by default does not allow applications to
    receive data from unknown clients. This can cause TeamTalk not
    being able to communicate properly with server and other clients
    when using P2P mode. It is therefore recommended to add TeamTalk
    to the Windows Firewall exception list.
  
- **Default Subscriptions upon Connection** 

  - **User Messages**
    - By disabling this option all text message sent to you will be
      ignored unless you explicitly subscribe to User Messages from a
      user in the [Subscriptions-menu](@ref subscriptionsmenu).

  - **Channel Messages**
    - By disabling this option all channel text message will be
      ignored unless you explicitly subscribe to Channel Messages from
      a user in the [Subscriptions-menu](@ref subscriptionsmenu).

  - **Broadcast Messages**
    - By disabling this option all broadcast text message will be
      ignored unless you explicitly subscribe to Broadcast Messages
      from a user in the [Subscriptions-menu](@ref subscriptionsmenu).

  - **Media Files**
    - By disabling this option you will not hear or see media files
      being streamed to the channel you're in unless you explicitly
      subscribe to Media Files from a user in the
      [Subscriptions-menu](@ref subscriptionsmenu).

  - **Voice**
    - By disabling this option all voice will be ignored unless you
      explicitly subscribe to Voice from a user in the
      [Subscriptions-menu](@ref subscriptionsmenu).

  - **Video Capture**
    - By disabling this option all video will be ignored unless you
      explicitly subscribe to Video from a user in the
      [Subscriptions-menu](@ref subscriptionsmenu).

  - **Desktop**
    - By disabling this option all shared desktops will be ignored
      unless you explicitly subscribe to Desktop from a user in the
      [Subscriptions-menu](@ref subscriptionsmenu).
      
  - **Desktop Access**
    - Setting up default subscriptions for desktop access can make
      others users automatically get control of mouse and keyboard.
      Desktop Access is also available in the
      [Subscriptions-menu](@ref subscriptionsmenu).
    
- **TCP port**
  - The socket used for TCP should bind to the specified port. Do not
    change unless you know what you're doing.

- **UDP port**
  - The socket used for UDP should bind to the specified port. Do not
    change unless you know what you're doing.

# Sound System {#soundsystemtab}

The *Sound System*-tab is for configuring your audio recording and
playback devices.

![Sound System-tab in Preferences](pref_soundsystem.png "Sound System-tab")

The items in the Sound System-tab are explained here:

- **Windows Audio Session**
  - The preferred sound system on Windows.
  - Echo cancellation, denoising and automatic gain control is
    performed using Windows' built in functions.
  - When enabling echo cancellation, denoising or automatic gain
    control (AGC) the sound system automatically switches to mono at
    22 KHz.

- **DirectSound**
  - Provides best compatibility with sound devices.
  - Echo cancellation, denoising and automatic gain control is
    performed using Speex's audio processing.

- **Windows Standard**
  - Windows default sound system. This typically is slower than
    DirectSound and Windows Audio Session.
  - Echo cancellation, denoising and automatic gain control is
    performed using Speex's audio processing.
  
- **ALSA**
  - This sound system is only available on Linux.
  - Echo cancellation, denoising and automatic gain control is
    performed using Speex's audio processing.

- **Core Audio**
  - This sound system is only available on Mac OS X.
  - Echo cancellation, denoising and automatic gain control is
    performed using Speex's audio processing.

- **Input Device**
  - The sound device to use for recording audio. It is advised to use
    one which supports stereo, i.e. label below should say Max Input
    Channels 2.

- **Output Device**
  - The sound device to use for audio playback. It is advised to use
    one which supports stereo, i.e. label below should say Max Output
    Channels 2.

- **Test Selected**
  - Use this to test the selected sound devices.

- **Enable duplex mode (required for echo cancellation)**
  - When this option is enabled all received audio is merged into a
    single stream and played. Sound cards which cannot option multiple
    streams can use this option and if you intend to use echo
    cancellation this option is a requirement.

- **Enable echo cancellation (remove echo from speakers)**
  - If you're using speakers instead of headphones when other users
    may be hearing themselves due to echo from speakers. Enabling this
    option will remove the echo from speakers. It, however, requires a
    lot of system resources and is therefore not enabled by
    default. For best echo cancellation it's also advised to enable
    the [Audio Configuration](@ref createchandlg) option when creating
    channels.

- **Enable automatic gain control (microphone level adjusted
  automatically)**
  - The volume level of microphones vary a lot therefore it's
    recommended enabling automatic gain control (AGC) to ensure all
    users in a channel speak at the same volume level. AGC is used in
    combination with [Audio Configuration](@ref createchandlg) when
    creating channels. The volume level specified in the
    [Audio Configuration](@ref createchandlg) will be used as
    reference volume for AGC.

- **Enable denoising**
  - If you have a poor quality microphone it is advised to enable
    denoising.

- **Default**
  - This button resets all sound system items to their default values.

# Sound Events {#soundeventstab}

The *Sound Events*-tab is for playing sound when certain events take
place.

![Sound Events-tab in Preferences](pref_soundevents.png "Sound Events-tab")

The items in the Sound Events-tab are explained here:

- **New user**
  - Play this sound when a new user joins your channel. 

- **Remove user**
  - Play this sound when a user leaves your channel. 

- **Server lost** 
  - Play this sound if TeamTalk drops its connection to the server. 

- **New user message** 
  - Play this sound when a user to user text message is received.

- **New channel message** 
  - Play this sound when a channel text message is received.

- **Hotkey pressed**
  - Play this sound when the Push to Talk key combination is pressed.

- **Channel silent**
  - Play this sound when the last user has stopped talking and there's
    no one left who is talking in the channel.

- **Files updated**
  - Play this sound when a file is either added or deleted.

- **File transfer complete**
  - Play this sound when a file transfer is completed. 

- **New video session**
  - Play this sound when a new video stream can be seen in the
    [Video-tab](@ref videotab).

- **New desktop session**
  - Play this sound when a new shared desktop can be seen in the
    [Desktops-tab](@ref desktoptab).

- **User entered question-mode**
  - Play this sound if a user in your channel changes status to
    Question-mode.

- **Desktop access request**
  - Play this sound when a user requests access to your shared desktop
    session.

# Shortcuts {#shortcutstab}

The Shortcuts tab is for keyboard shortcuts (hotkey) for common tasks.

![Shortcuts-tab in Preferences](pref_shortcuts.png "Shortcuts-tab")

The items in the Shortcuts-tab are explained here:

- **Enable/disable voice activation** 
  - Global hotkey to turn on/off voice activation.

- **Increase volume**
  - Global hotkey to increase the master volume.

- **Lower volume**
  - Global hotkey to lower the master volume.

- **Enable/disable mute all** 
  - Global hotkey to mute all users.

- **Increase microphone gain** 
  - Global hotkey to increase the microphone volume.

- **Lower microphone gain** 
  -Global hotkey to low the microphone volume.

- **Enable/disable video transmission** 
  - Global hotkey to turn on/off video transmission.

# Video Capture {#videocapturetab}

The *Video Capture*-tab is for configuring web camera for video
recording.

![Video Capture-tab in Preferences](pref_videocapture.png "Video Capture-tab")

The items in the Video Capture-tab are explained here:

- **Video Capture Device**
  - The web camera to use for capturing video.

- **Video Resolution**
  - The video resolution to use and the frame rate (FPS).

- **Image Format**
  - The image format to use. Use RGB32 to get the best picture quality
    and I420 for the highest frame rate.

- **Test Selected**
  - Test the selected video capture settings. Note the video codec
    settings are not used when testing this.

- **Codec**
  - The video codec to use for compressing the video data.

- **Bitrate**
  - The bitrate the video encoder should target. The higher the
    value, the higher the bandwidth usage. Read the Quick Start
    section on how to configure the settings which are best for your
    Internet connection.

- **Default** 
  - Make TeamTalk choose the default settings.
