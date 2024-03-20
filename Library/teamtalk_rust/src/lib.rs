#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::time::{Duration, Instant};
use std::{ffi::CString, ffi::CStr};
use std::result::Result;

pub fn ttstr(str: &str) -> CString {
    CString::new(str).unwrap_or(CString::new("").unwrap())
}

pub fn from_ttstr(str: *const TTCHAR) -> String {
    unsafe {
        match CStr::from_ptr(str).to_str() {
            Ok(s) => { return s.to_string() }
            Err(_) => return "UTF8FAIL".to_string()
        }
    }
}

pub fn wait_for_event_pred<P>(ttinst: *mut TTInstance, event: ClientEvent, pred_fn: P, def_wait: Duration) -> Result<TTMessage, ClientEvent>
    where P: Fn(&TTMessage) -> bool,
{
    let mut m = TTMessage::default();
    let start = Instant::now();
    unsafe {
        let mut remain_msec: INT32  = def_wait.as_millis().try_into().expect("Duration is too high");
        while TT_GetMessage(ttinst, &mut m, &remain_msec) == TRUE || def_wait <= Instant::now() - start {
            if m.nClientEvent == event && pred_fn(&m) {
                return Ok(m)
            }
            remain_msec = (def_wait - (Instant::now() - start)).as_millis().try_into().expect("Duration is too high");
        }
    }
    Err(event)
}

pub fn wait_for_event(ttinst: *mut TTInstance, event: ClientEvent, def_wait: Duration) -> Result<TTMessage, ClientEvent> {
    let m_fn = |_: &TTMessage| -> bool { true };
    wait_for_event_pred(ttinst, event, m_fn, def_wait)
}

pub fn wait_for_cmd_success(ttinst: *mut TTInstance, cmdid: INT32, def_wait: Duration) -> Result<TTMessage, ClientEvent> {
    let m_fn = |m: &TTMessage| -> bool { m.nSource == cmdid };
    wait_for_event_pred(ttinst, ClientEvent::CLIENTEVENT_CMD_SUCCESS, m_fn, def_wait)
}

pub fn wait_for_cmd_complete(ttinst: *mut TTInstance, cmdid: INT32, def_wait: Duration) -> Result<TTMessage, ClientEvent> {
    let m_fn = |m: &TTMessage| -> bool { 
        unsafe {
            m.nSource == cmdid && m.__bindgen_anon_1.bActive == FALSE   
        }
    };
    wait_for_event_pred(ttinst, ClientEvent::CLIENTEVENT_CMD_PROCESSING, m_fn, def_wait)
}

#[cfg(test)]
mod tests {
    use std::{ffi::CStr};

    use super::*;

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
            let e = wait_for_event(a, ClientEvent::CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT);
            assert!(e.is_ok());
            assert!(e.unwrap().__bindgen_anon_1.useraccount.uUserType != 0);
            print!("{}", from_ttstr(e.unwrap().__bindgen_anon_1.useraccount.szUsername.as_ptr()));
            assert!(wait_for_cmd_success(a, cmdid, DEF_WAIT).is_ok());
            assert!(wait_for_cmd_complete(a, cmdid, DEF_WAIT).is_ok());
            TT_CloseTeamTalk(a);
        }
    }

}
