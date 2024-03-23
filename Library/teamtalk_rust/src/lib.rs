
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::CString;

pub fn add(left: usize, right: usize) -> usize {
    left + right
}

fn ttstr(str: &str) -> CString {
    CString::new(str).unwrap_or(CString::new("").unwrap())
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
            let a = TT_InitTeamTalkPoll();
            assert!(TT_Connect(a, ttstr("192.168.0.110").as_ptr(), 10333, 10333, 0, 0, FALSE) == TRUE);
            let c = Channel::default();
            let mut m = TTMessage::default();
            assert!(TT_GetMessage(a, &mut m, ptr::null()) == TRUE);
            assert_eq!(m.nClientEvent, ClientEvent::CLIENTEVENT_CON_SUCCESS);
            let cmdid = TT_DoLogin(a, ttstr("foobar").as_ptr(), ttstr("guest").as_ptr(), ttstr("guest").as_ptr());
            assert!(cmdid > 0);
            assert!(TT_GetMessage(a, &mut m, ptr::null()) == TRUE);
            assert_eq!(m.nClientEvent, ClientEvent::CLIENTEVENT_CMD_MYSELF_LOGGEDIN);
            TT_CloseTeamTalk(a);
        }
    }

}
