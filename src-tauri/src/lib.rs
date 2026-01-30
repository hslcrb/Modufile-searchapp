use serde::{Serialize, Deserialize};
use std::sync::{Arc, RwLock};
use tauri::State;
use std::path::PathBuf;
use std::ffi::CStr;
use std::os::raw::{c_char, c_void};

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct FileInfo {
    name: String,
    path: String,
}

pub struct AppState {
    files: Arc<RwLock<Vec<FileInfo>>>,
}

// C function declaration
extern "C" {
    fn start_walk(callback: extern "C" fn(*const c_char, *const c_char, *mut c_void), context: *mut c_void);
}

// Global/Context callback for C
extern "C" fn file_callback(name: *const c_char, path: *const c_char, context: *mut c_void) {
    unsafe {
        if name.is_null() || path.is_null() { return; }
        
        let name_str = CStr::from_ptr(name).to_string_lossy().into_owned();
        let path_str = CStr::from_ptr(path).to_string_lossy().into_owned();
        
        // This is safe because we know context is a &mut Vec<FileInfo>
        let files = &mut *(context as *mut Vec<FileInfo>);
        files.push(FileInfo { name: name_str, path: path_str });
    }
}

#[tauri::command]
fn search(query: String, smart_match: bool, state: State<'_, AppState>) -> Vec<FileInfo> {
    let files = state.files.read().unwrap();
    if query.is_empty() {
        return files.iter().take(100).cloned().collect();
    }
    
    let query_lower = query.to_lowercase();
    
    if smart_match {
        use fuzzy_matcher::skim::SkimMatcherV2;
        use fuzzy_matcher::FuzzyMatcher;
        let matcher = SkimMatcherV2::default();
        
        let mut results: Vec<(i64, FileInfo)> = files.iter()
            .filter_map(|f| {
                matcher.fuzzy_match(&f.name, &query).map(|score| (score, f.clone()))
            })
            .collect();
            
        results.sort_by(|a, b| b.0.cmp(&a.0));
        
        results.into_iter()
            .take(200)
            .map(|(_, f)| f)
            .collect()
    } else {
        files.iter()
            .filter(|f| f.name.to_lowercase().contains(&query_lower))
            .take(200)
            .cloned()
            .collect()
    }
}

#[tauri::command]
async fn refresh_index(state: State<'_, AppState>) -> Result<usize, String> {
    let files_arc = state.files.clone();
    
    tokio::task::spawn_blocking(move || {
        let mut new_files = Vec::new();
        let context_ptr = &mut new_files as *mut Vec<FileInfo> as *mut c_void;
        
        unsafe {
            start_walk(file_callback, context_ptr);
        }
        
        let count = new_files.len();
        let mut files = files_arc.write().unwrap();
        *files = new_files;
        count
    }).await.map_err(|e| e.to_string())
}

#[tauri::command]
fn open_file(path: String) -> Result<(), String> {
    opener::reveal(PathBuf::from(path)).map_err(|e: std::io::Error| e.to_string())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let state = AppState {
        files: Arc::new(RwLock::new(Vec::new())),
    };

    tauri::Builder::default()
        .manage(state)
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![search, refresh_index, open_file])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
