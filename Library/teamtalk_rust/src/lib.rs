
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::time::{Duration, Instant};
use std::{ffi::CString, ptr};
use std::result::Result;

pub fn add(left: usize, right: usize) -> usize {
    left + right
}

fn ttstr(str: &str) -> CString {
    CString::new(str).unwrap_or(CString::new("").unwrap())
}

fn wait_for_event(ttinst: *mut TTInstance, event: ClientEvent, def_wait: Duration) -> Result<TTMessage, ClientEvent> {
    let mut m = TTMessage::default();
    let start = Instant::now();
    unsafe {
        let mut remain_msec: INT32  = def_wait.as_millis().try_into().expect("Duration is too high");
        while TT_GetMessage(ttinst, &mut m, &remain_msec) == TRUE || def_wait <= Instant::now() - start {
            if m.nClientEvent == event {
                return Ok(m)
            }
            remain_msec = (def_wait - (Instant::now() - start)).as_millis().try_into().expect("Duration is too high");
        }
    }
    Err(event)
}

fn wait_for_cmdsuccess(ttinst: *mut TTInstance, cmdid: INT32, def_wait: Duration) -> Result<TTMessage, ClientEvent> {
    loop {
        match wait_for_event(ttinst, ClientEvent::CLIENTEVENT_CMD_SUCCESS, def_wait) {
            Ok(m) => if m.nSource == cmdid {
                return Ok(m)
            }
            Err(e) => return Err(e)
        }
    }
}

#[cfg(test)]
mod tests {
    use std::{ffi::CStr, ptr};

    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }

    #[test]
    fn printversion() {
        let a = unsafe { CStr::from_ptr(TT_GetVersion()) };
        println!("{:?}", a);
    }

    #[test]
    fn initclose() {
        unsafe {
            let a = TT_InitTeamTalkPoll();
            TT_CloseTeamTalk(a);
        }
    }

    #[test]
    fn login() {
        unsafe {
            let DEF_WAIT = Duration::new(5, 0);
            let a = TT_InitTeamTalkPoll();
            assert!(TT_Connect(a, ttstr("192.168.0.110").as_ptr(), 10333, 10333, 0, 0, FALSE) == TRUE);
            let c = Channel::default();
            let mut m = TTMessage::default();
            assert!(wait_for_event(a, ClientEvent::CLIENTEVENT_CON_SUCCESS, DEF_WAIT).is_ok());
            let cmdid = TT_DoLogin(a, ttstr("foobar").as_ptr(), ttstr("guest").as_ptr(), ttstr("guest").as_ptr());
            assert!(cmdid > 0);
            assert!(wait_for_cmdsuccess(a, cmdid, DEF_WAIT).is_ok());
            TT_CloseTeamTalk(a);
        }
    }

}
