#![allow(dead_code)]

extern crate libc;
extern crate opener;

use std::slice;
use std::ffi::CStr;
//use rfd::FileDialog;
use std::mem;

fn from_c(ptr: *const libc::c_char) -> String {
    let c_str: &CStr = unsafe{ CStr::from_ptr(ptr) };
    return c_str.to_str().unwrap().to_owned();
}

#[no_mangle]
pub extern "C" fn uopen(ptr: *const libc::c_char) {
    let _ = opener::open(from_c(ptr));
}

#[no_mangle]
pub extern "C" fn ureveal(ptr: *const libc::c_char) {
    let _ = opener::reveal(from_c(ptr));
}
