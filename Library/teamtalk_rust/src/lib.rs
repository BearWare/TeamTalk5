
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

pub fn add(left: usize, right: usize) -> usize {
    left + right
}

#[cfg(test)]
mod tests {
    use std::ffi::CStr;

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

}
