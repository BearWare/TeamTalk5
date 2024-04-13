use core::panic;
use std::{io::{self, Write}, path::PathBuf, time::Duration, vec};
use teamtalk::{self, wait_for_event, Subscriptions, TTMessage, INT32};
use teamtalk::ttstr as ttstr;
use teamtalk::from_ttstr as from_ttstr;
use teamtalk::TRUE as TRUE;
use teamtalk::FALSE as FALSE;

struct Session {
    ipaddr: String,
    tcpport: i32,
    udpport: i32,
    encrypted: bool,
    username: String,
    password: String,
    audiodir: PathBuf
}

const DEF_WAIT: Duration = Duration::new(10, 0);

fn print_ne(pstr: &str) {
    print!("{pstr}");
    io::stdout().flush().expect("Failed to flush");
}

fn get_value(info: &str, default: &str) -> io::Result<String> {
    print_ne(format!("{} ({}): ", info, default).as_str());
    let mut text = String::new();
    match io::stdin().read_line(&mut text) {
        Ok(_) => {
            let text_s = text.trim();
            Ok( if text_s.is_empty() { default.to_string() } else { text_s.to_string() } )
        },
        Err(e) => Err(e),
    }
}

fn get_session() -> Session {

    println!("TeamTalk 5 server login information.");

    let ipaddr = get_value("Specify IP-address of server to connect to", "192.168.0.110").expect("Invalid IP-address");

    let tcpport_s = get_value("Specify TCP port", "10333").expect("Invalid TCP port");
    let udpport_s = get_value("Specify UDP port", "10333").expect("Invalid UDP port");

    let mut encrypted_s = get_value("Is server encrypted?", "N").expect("Invalid encryption input");
    encrypted_s = encrypted_s.to_lowercase();

    let username = get_value("Specify username", "admin").expect("Invalid username");
    let password = get_value("Specify password", "admin").expect("Invalid password");

    let path_s = get_value("Specify directory where to store audio", "./").expect("Invalid path");
    let path = PathBuf::from(path_s);
    if !path.exists() {
        panic!("Directory {} does not exist", path.to_str().unwrap());
    }

    Session {
        ipaddr,
        tcpport: tcpport_s.trim().parse().expect("Invalid TCP port"),
        udpport: udpport_s.trim().parse().expect("Invalid UDP port"),
        encrypted: encrypted_s == "y" || encrypted_s == "yes",
        username, password,
        audiodir: path
    }
}

unsafe fn init_sound(ttinst: *mut teamtalk::TTInstance) -> bool {
    let mut inputid: INT32 = 0;
    let mut outputid: INT32 = 0;
    if teamtalk::TT_GetDefaultSoundDevices(&mut inputid, &mut outputid) == FALSE {
        return false;
    }

    let mut snddevs = vec![teamtalk::SoundDevice::default(); 25];
    let mut count = snddevs.len() as INT32;
    if teamtalk::TT_GetSoundDevices(snddevs.as_mut_ptr(), &mut count) != TRUE {
        panic!("Failed to get sound devices");
    }

    let mut indev = snddevs.iter().filter(|d| d.nDeviceID == inputid);
    match indev.next() {
        Some(d) => {
            println!("Using sound input device {}", from_ttstr(d.szDeviceName.as_ptr()));
        }
        None => {
            println!("Cannot find sound input device");
        }
    }

    let mut outdev = snddevs.iter().filter(|d| d.nDeviceID == outputid);
    match outdev.next() {
        Some(d) => {
            println!("Using sound output device {}", from_ttstr(d.szDeviceName.as_ptr()));
        }
        None => {
            println!("Cannot find sound output device");
        }
    }

    teamtalk::TT_InitSoundDuplexDevices(ttinst, inputid, outputid) == TRUE
}

unsafe fn connect(ttinst: *mut teamtalk::TTInstance, session: &Session) -> bool {
    if teamtalk::TT_Connect(ttinst, ttstr(&session.ipaddr).as_ptr(),
                            session.tcpport, session.udpport, 0, 0,
                            if session.encrypted { TRUE } else { FALSE }) != TRUE {
        return false;
    }

    if wait_for_event(ttinst, teamtalk::ClientEvent::CLIENTEVENT_CON_SUCCESS, DEF_WAIT).is_err() {
        return false;
    }

    assert_eq!(teamtalk::TT_GetFlags(ttinst) & teamtalk::ClientFlag::CLIENT_CONNECTED as u32, teamtalk::ClientFlag::CLIENT_CONNECTED as u32);

    true
}

unsafe fn login(ttinst: *mut teamtalk::TTInstance, session: &Session) -> bool {
    let cmdid = teamtalk::TT_DoLogin(ttinst, ttstr("").as_ptr(), ttstr(&session.username).as_ptr(), ttstr(&session.password).as_ptr());
    if cmdid < 0 {
        return false;
    }

    assert!(wait_for_event(ttinst, teamtalk::ClientEvent::CLIENTEVENT_CMD_PROCESSING, DEF_WAIT).is_ok());

    match wait_for_event(ttinst, teamtalk::ClientEvent::CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT) {
        Ok(_) => {
        }
        Err(_) => {
            return false;
        }
    }
    assert_eq!(teamtalk::TT_GetFlags(ttinst) & teamtalk::ClientFlag::CLIENT_AUTHORIZED as u32, teamtalk::ClientFlag::CLIENT_AUTHORIZED as u32);    

    true
}

fn main() {
    println!("TeamTalk 5 server logger");

    let session = get_session();
    unsafe {
        let ttinst = teamtalk::TT_InitTeamTalkPoll();

        if ! init_sound(ttinst) {
            panic!("Failed to initialize sound devices");
        }

        if ! connect(ttinst, &session) {
            panic!("Failed to connect to {}", session.ipaddr);
        }
        println!("Connected to {}:{}", session.ipaddr, session.tcpport);

        if ! login(ttinst, &session) {
            panic!("Login failed");
        }
        println!("Logged in... Got user ID #{}", teamtalk::TT_GetMyUserID(ttinst));

        let mut msg = teamtalk::TTMessage::default();
        while teamtalk::TT_GetMessage(ttinst, &mut msg, std::ptr::null()) == TRUE {
            process_ttmessage(ttinst, &session, msg);
        }
    }
}

unsafe fn process_ttmessage(ttinst: *mut teamtalk::TTInstance, session: &Session, msg: TTMessage) {
    match msg.nClientEvent {
        teamtalk::ClientEvent::CLIENTEVENT_CMD_SERVER_UPDATE => {
            println!("Got new server properties");
            println!("MOTD: {}", from_ttstr(msg.__bindgen_anon_1.serverproperties.szMOTD.as_ptr()));
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_CHANNEL_NEW => {
            let chanpath = get_channel_path(ttinst, msg.__bindgen_anon_1.channel.nChannelID);
            println!("Added channel {chanpath}");
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_CHANNEL_UPDATE => {
            let chanpath = get_channel_path(ttinst, msg.__bindgen_anon_1.channel.nChannelID);
            println!("Updated channel {chanpath}");
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_CHANNEL_REMOVE => {
            println!("Removed channel #{}", msg.__bindgen_anon_1.channel.nChannelID);
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_USER_LOGGEDIN => {
            println!("User #{} {} logged in", msg.__bindgen_anon_1.user.nUserID,
                     from_ttstr(msg.__bindgen_anon_1.user.szNickname.as_ptr()));
            let cmdid = teamtalk::TT_DoSubscribe(ttinst, msg.__bindgen_anon_1.user.nUserID,
                                                 teamtalk::Subscription::SUBSCRIBE_INTERCEPT_USER_MSG as Subscriptions |
                                                 teamtalk::Subscription::SUBSCRIBE_INTERCEPT_CHANNEL_MSG as Subscriptions |
                                                 teamtalk::Subscription::SUBSCRIBE_INTERCEPT_VOICE as Subscriptions);
            if cmdid > 0 {
                println!("Subscribing to text and audio events from #{}", msg.__bindgen_anon_1.user.nUserID);
            } else {
                println!("Failed to issue command to subscribe");
            }
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_USER_LOGGEDOUT => {
            println!("User #{} {} logged out", msg.__bindgen_anon_1.user.nUserID,
                     from_ttstr(msg.__bindgen_anon_1.user.szNickname.as_ptr()));
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_USER_JOINED => {
            let chanpath = get_channel_path(ttinst, msg.__bindgen_anon_1.user.nChannelID);
            println!("User #{} {} joined {}", msg.__bindgen_anon_1.user.nUserID,
                     from_ttstr(msg.__bindgen_anon_1.user.szNickname.as_ptr()),
                     chanpath);
            setup_audio_storage(ttinst, msg.__bindgen_anon_1.user.nUserID, session);
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_USER_LEFT => {
            let chanpath = get_channel_path(ttinst, msg.__bindgen_anon_1.user.nChannelID);
            println!("User #{} {} left {}", msg.__bindgen_anon_1.user.nUserID,
                     from_ttstr(msg.__bindgen_anon_1.user.szNickname.as_ptr()),
                     chanpath);
        }
        teamtalk::ClientEvent::CLIENTEVENT_CMD_USER_TEXTMSG => {
            printtextmsg(ttinst, msg.__bindgen_anon_1.textmessage);
        }
        teamtalk::ClientEvent::CLIENTEVENT_USER_RECORD_MEDIAFILE => {
            if msg.__bindgen_anon_1.mediafileinfo.nStatus == teamtalk::MediaFileStatus::MFS_FINISHED {
                println!("Finished recoding audio from #{} to file {}", msg.nSource, from_ttstr(msg.__bindgen_anon_1.mediafileinfo.szFileName.as_ptr()));
            }
        }
        _ => {
            println!("Skipped event {:?}", msg.nClientEvent)
        }
    }
}

unsafe fn get_channel_path(ttinst: *mut teamtalk::TTInstance, chanid: INT32) -> String {
    let mut chanpath: [teamtalk::TTCHAR; teamtalk::TT_STRLEN as usize] = [0; teamtalk::TT_STRLEN as usize];
    if teamtalk::TT_GetChannelPath(ttinst, chanid, chanpath.as_mut_ptr()) == TRUE {
        from_ttstr(chanpath.as_ptr())
    } else {
        "".to_string()
    }
}

unsafe fn printtextmsg(ttinst: *mut teamtalk::TTInstance, textmsg: teamtalk::TextMessage) {
    let msgtext = from_ttstr(textmsg.szMessage.as_ptr());
    match textmsg.nMsgType {
        teamtalk::TextMsgType::MSGTYPE_USER => {
            println!("Text message from user #{} to user #{} content: {}",
                     textmsg.nFromUserID, textmsg.nToUserID, msgtext);
        }
        teamtalk::TextMsgType::MSGTYPE_CHANNEL => {
            println!("Text message from user #{} to channel {} content: {}",
                     textmsg.nFromUserID, get_channel_path(ttinst, textmsg.nChannelID), msgtext);
        }
        teamtalk::TextMsgType::MSGTYPE_BROADCAST => {
            println!("Text message from user #{} to entire server content: {}",
                     textmsg.nFromUserID, msgtext);

        }
        teamtalk::TextMsgType::MSGTYPE_CUSTOM => {
        }
        teamtalk::TextMsgType::MSGTYPE_NONE => {
        }
    }
}

unsafe fn setup_audio_storage(ttinst: *mut teamtalk::TTInstance, userid: INT32, session: &Session) {
    let audiodir = session.audiodir.to_str().unwrap_or("");
    teamtalk::TT_SetUserMediaStorageDir(ttinst, userid,
                                        ttstr(&audiodir).as_ptr(),
                                        std::ptr::null(),
                                        teamtalk::AudioFileFormat::AFF_WAVE_FORMAT);

}
