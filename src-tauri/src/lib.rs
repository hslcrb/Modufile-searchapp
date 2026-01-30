use serde::{Serialize, Deserialize};
use tauri::State;
use std::path::PathBuf;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void, c_int};
use std::sync::Mutex;

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct FileInfo {
    name: String,
    path: String,
}

// C function declarations
extern "C" {
    fn refresh_index_c() -> c_int;
    fn search_c(query: *const c_char, smart_match: c_int, callback: extern "C" fn(*const c_char, *const c_char, *mut c_void), context: *mut c_void);
}

// Result Callback
extern "C" fn search_callback(name: *const c_char, path: *const c_char, context: *mut c_void) {
    unsafe {
        if name.is_null() || path.is_null() { return; }
        
        let name_str = CStr::from_ptr(name).to_string_lossy().into_owned();
        let path_str = CStr::from_ptr(path).to_string_lossy().into_owned();
        
        let results = &mut *(context as *mut Vec<FileInfo>);
        results.push(FileInfo { name: name_str, path: path_str });
    }
}

// Just a dummy state to satisfy Tauri signatures if needed, though we rely on C static global DB
pub struct AppState(Mutex<()>);

#[tauri::command]
fn search(query: String, smart_match: bool, _state: State<'_, AppState>) -> Vec<FileInfo> {
    let mut results = Vec::new();
    let query_c = CString::new(query).unwrap_or_default();
    let smart_match_int = if smart_match { 1 } else { 0 };
    
    let context_ptr = &mut results as *mut Vec<FileInfo> as *mut c_void;
    
    unsafe {
        search_c(query_c.as_ptr(), smart_match_int, search_callback, context_ptr);
    }
    
    results
}

#[tauri::command]
async fn refresh_index(_state: State<'_, AppState>) -> Result<usize, String> {
    tokio::task::spawn_blocking(move || {
        unsafe {
            refresh_index_c() as usize
        }
    }).await.map_err(|e| e.to_string())
}

#[tauri::command]
fn open_file(path: String) -> Result<(), String> {
    opener::reveal(PathBuf::from(path)).map_err(|e: std::io::Error| e.to_string())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .manage(AppState(Mutex::new(())))
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![search, refresh_index, open_file])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
