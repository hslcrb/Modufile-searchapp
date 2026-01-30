fn main() {
    cc::Build::new()
        .file("src/backend.c")
        .compile("modufile_backend");
        
    tauri_build::build();
}
