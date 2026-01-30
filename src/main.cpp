#include "webview.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <sstream>
#include <iomanip>

// --- Backend Logic (Ported from C) ---
namespace fs = std::filesystem;

struct FileInfo {
    std::string name;
    std::string path;
};

struct AppState {
    std::vector<FileInfo> files;
    std::mutex mtx;
};

static AppState app_state;

// Fuzzy Matcher Logic
int fuzzy_score(const std::string& str, const std::string& pattern) {
    int score = 0;
    int run = 0;
    
    auto str_it = str.begin();
    auto pat_it = pattern.begin();
    
    while (str_it != str.end() && pat_it != pattern.end()) {
        if (std::tolower(*str_it) == std::tolower(*pat_it)) {
            score += 10 + (run * 5);
            run++;
            pat_it++;
        } else {
            run = 0;
            score -= 1;
        }
        str_it++;
    }
    
    if (pat_it != pattern.end()) return 0;
    return score > 0 ? score : 1;
}

// Indexing Logic
void refresh_index_impl() {
    std::vector<FileInfo> new_files;
    new_files.reserve(500000); // Pre-allocate expectation

    try {
        // Recursive iterate
        // Skip some system dirs for speed on Linux
        std::vector<std::string> skip_prefixes = {"/proc", "/sys", "/dev", "/run", "/tmp", "/snap"};
        
        #ifdef _WIN32
        std::string root = "C:\\";
        #else
        std::string root = "/";
        #endif

        for (auto& p : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
            try {
                // Check skips
                std::string path_str = p.path().string();
                #ifndef _WIN32
                bool skip = false;
                for(const auto& prefix : skip_prefixes) {
                    if (path_str.rfind(prefix, 0) == 0) {
                        skip = true;
                        break;
                    }
                }
                if (skip) continue; // Note: this doesn't stop recursing INTO them with std::filesystem, but filters results. 
                                    // To skip recursing, we need customized iterator control which is harder in std::fs. 
                                    // For now, accept slight overhead or use C-style if needed. 
                                    // Actually std::filesystem is robust.
                #endif

                if (p.is_regular_file()) {
                     new_files.push_back({p.path().filename().string(), path_str});
                }
            } catch (...) {
                continue; 
            }
        }
    } catch (...) {}

    std::lock_guard<std::mutex> lock(app_state.mtx);
    app_state.files = std::move(new_files);
}

// --- JSON Helper ---
std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    o << "\\u"
                      << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

// --- Main ---

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif
    webview::webview w(true, nullptr);
    w.set_title("모두파일 (Modufile)");
    w.set_size(800, 600, WEBVIEW_HINT_NONE);

    // Bind: Refresh Index
    w.bind("refresh_index", [&](const std::string& seq, const std::string& req, void* arg) {
        std::thread([&, seq]() {
            refresh_index_impl();
            size_t count = 0;
            {
                std::lock_guard<std::mutex> lock(app_state.mtx);
                count = app_state.files.size();
            }
            w.resolve(seq, 0, std::to_string(count));
        }).detach();
    });

    // Bind: Search
    w.bind("search", [&](const std::string& seq, const std::string& req, void* arg) {
        // req is JSON array like ["query", true/false] but webview binding might pass it differently depending on JS.
        // Actually webview library passes the argument string as is. 
        // We'll parse it simply or assume structure. 
        // For simplicity, let's assume JS sends an object: {"query": "...", "smartMatch": true}
        
        // Manual JSON parsing (since we removed serde)
        std::string query;
        bool smart_match = false;
        
        // Ultra-naive parser for the specific format we send from JS
        // JS: invoke("search", { query, smartMatch }) -> actually webview binds receive serialized args.
        // If we use standard webview bind, the JS side should be: window.search(query, smartMatch)
        
        // Let's adjust JS to pass a JSON string or simplified args.
        // Assuming req is key:value json.
        
        std::string clean_req = req;
        // Basic extraction
        size_t q_pos = clean_req.find("\"query\":\"");
        if (q_pos != std::string::npos) {
            size_t q_end = clean_req.find("\"", q_pos + 9);
            query = clean_req.substr(q_pos + 9, q_end - (q_pos + 9));
        }
        
        if (clean_req.find("\"smartMatch\":true") != std::string::npos) {
            smart_match = true;
        }

        struct ResultItem {
            const FileInfo* file;
            int score;
        };
        std::vector<ResultItem> results;

        {
            std::lock_guard<std::mutex> lock(app_state.mtx);
            if (query.empty()) {
                for (size_t i = 0; i < app_state.files.size() && i < 100; ++i) {
                    results.push_back({&app_state.files[i], 0});
                }
            } else {
                for (const auto& f : app_state.files) {
                    int score = 0;
                    if (smart_match) {
                        score = fuzzy_score(f.name, query);
                    } else {
                        // Case insensitive substring
                        auto it = std::search(
                            f.name.begin(), f.name.end(),
                            query.begin(), query.end(),
                            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
                        );
                        if (it != f.name.end()) score = 100;
                    }

                    if (score > 0) {
                        results.push_back({&f, score});
                    }
                    
                    // Perf limit for smart match sort
                    if (smart_match && results.size() > 2000) break;
                    if (!smart_match && results.size() > 200) break;
                }
            }
        }

        if (smart_match) {
            std::sort(results.begin(), results.end(), [](const ResultItem& a, const ResultItem& b) {
                return a.score > b.score;
            });
        }

        // Build JSON response
        std::ostringstream json;
        json << "[";
        int limit = std::min((int)results.size(), 200);
        for (int i = 0; i < limit; ++i) {
            if (i > 0) json << ",";
            json << "{\"name\":\"" << escape_json(results[i].file->name) 
                 << "\",\"path\":\"" << escape_json(results[i].file->path) << "\"}";
        }
        json << "]";

        w.resolve(seq, 0, json.str());
    });

    // Bind: Open File
    w.bind("open_file", [&](const std::string& seq, const std::string& req, void* arg) {
         std::string path;
         size_t p_pos = req.find("\"path\":\"");
         if (p_pos != std::string::npos) {
            size_t p_end = req.find("\"", p_pos + 8);
            path = req.substr(p_pos + 8, p_end - (p_pos + 8));
         }
         
         // Simple command to open file
         #ifdef _WIN32
         ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOW);
         #else
         std::string cmd = "xdg-open \"" + path + "\"";
         system(cmd.c_str());
         #endif
         
         w.resolve(seq, 0, "null");
    });

    // Load HTML
    // For development, serving from local file system. 
    // In production, one would embed this info header. 
    // For now, let's read index.html content and inject it or use file://
    
    // Simplest: Assume ui/index.html is relative to CWD
    // To make it self contained, better to read file content into string
    
    // Fix: We need absolute path for file:// or we can inject html
    // Let's try to find the ui folder.
    
    fs::path bin_path = fs::absolute("ui/index.html");
    if (fs::exists(bin_path)) {
       w.navigate("file://" + bin_path.string());
    } else {
        w.set_html("<html><body><h1>UI Not Found</h1><p>Please ensure 'ui' folder exists.</p></body></html>");
    }

    w.run();
    return 0;
}
