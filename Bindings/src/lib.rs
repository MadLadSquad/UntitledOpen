#![allow(dead_code)]

extern crate libc;
extern crate opener;
extern crate rfd;

use std::slice;
use std::ffi::CStr;
//use rfd::FileDialog;
use std::mem;

pub fn convert_c_char_array(c_array: *const *const libc::c_char, size: libc::c_ulong) -> Vec<String> {
    // Create a slice of *const c_char from the C array
    let c_str_slice: &[*const libc::c_char] = unsafe { slice::from_raw_parts(c_array, size as usize) };

    // Convert each C string pointer to a Rust String
    let mut result = Vec::new();
    for &c_str_ptr in c_str_slice {
        let c_str = unsafe { CStr::from_ptr(c_str_ptr) };
        if let Ok(rust_str) = c_str.to_str() {
            result.push(String::from(rust_str));
        }
    }

    return result;
}

fn to_c(ptr: String) -> *const libc::c_char {
    let c_str = std::ffi::CString::new(ptr).unwrap();
    return c_str.into_raw();
}

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

#[no_mangle]
pub extern "C" fn umsg_dialogue(title: *const libc::c_char, description: *const libc::c_char, msg_type: libc::c_char) -> libc::c_char {
    let msg_type_real: rfd::MessageButtons;

    match msg_type {
        0 => msg_type_real = rfd::MessageButtons::Ok,
        1 => msg_type_real = rfd::MessageButtons::YesNo,
        _ => msg_type_real = rfd::MessageButtons::OkCancel,
    }

    let msg = rfd::MessageDialog::new()
                .set_title(from_c(title).as_str())
                .set_description(from_c(description).as_str())
                .set_buttons(msg_type_real)
                .show();

    return unsafe { mem::transmute(msg) };
}

#[no_mangle]
pub extern "C" fn usave_file(path: *const libc::c_char, name: *const libc::c_char) -> *const libc::c_char {
    let res = rfd::FileDialog::new()
                .set_file_name(from_c(name).as_str())
                .set_directory(from_c(path).as_str())
                .save_file();
    match res {
        None => return to_c("".to_string()),
        Some(val) => return to_c(val.into_os_string().into_string().unwrap()),
    }
    //return to_c(res.into_os_string().into_string().unwrap());
}

#[repr(C)]
pub struct UFilters {
    name: *const libc::c_char,
    filters: *const *const libc::c_char,
    filter_size: libc::c_ulong,
}

#[repr(C)]
pub struct UDirectoriesArray {
    data: *const *const libc::c_char,
    size: libc::c_ulong,
}

fn upick_file_generic(filters: *const UFilters, size: libc::c_ulong) -> rfd::FileDialog {
    let fls = unsafe { std::slice::from_raw_parts(filters as *const UFilters, size as usize) };
    let mut dialogue = rfd::FileDialog::new();

    for filter in fls {
        dialogue = dialogue.add_filter(from_c(filter.name).as_str(), &convert_c_char_array(filter.filters, filter.filter_size).iter().map(|s| s.as_str()).collect::<Vec<&str>>()[..]);
    }

    //for (filter in fls) {
    //    dialogue = dialogue.add_filter(from_c(filter.name), std::slice::from_raw_parts(filter.filters as *String, filter.filter_size as usize));
    //}
    return dialogue;
}

#[no_mangle]
pub extern "C" fn upick_file(path: *const libc::c_char, filters: *const UFilters, size: libc::c_ulong) -> *const libc::c_char {
    let result = upick_file_generic(filters, size).set_directory(from_c(path)).pick_file();
    match result {
        None => return to_c("".to_string()),
        Some(val) => return to_c(val.into_os_string().into_string().unwrap()),
    }
    //return to_c(result.into_os_string().into_string().unwrap());
}

#[no_mangle]
pub extern "C" fn upick_directory(path: *const libc::c_char, filters: *const UFilters, size: libc::c_ulong) -> *const libc::c_char {
    let result = upick_file_generic(filters, size).set_directory(from_c(path)).pick_folder();
    match result {
        None => return to_c("".to_string()),
        Some(val) => return to_c(val.into_os_string().into_string().unwrap()),
    }
    //return to_c(result.into_os_string().into_string().unwrap());
}

//#[no_mangle]
//pub extern "C" fn upick_files(path: *const libc::c_char, filters: *UFilters, size: libc::c_ulong) -> *UDirectoriesArray {
//    let result = upick_file_generic(filters, size).set_directory(from_c(path)).pick_files();
//    let arr: Vec<UDirectoriesArray>;
//    for (a in result) {
//        arr.append({ a. })
//    }
//
//}

#[no_mangle]
pub extern "C" fn ufree_string(ptr: *const libc::c_char) {
    unsafe { let _ = std::ffi::CString::from_raw(ptr as *mut _); };
}
