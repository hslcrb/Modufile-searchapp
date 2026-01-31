#include "FileEngine.h"
#include <algorithm>
#include <vector>
#include <wx/utils.h>

namespace fs = std::filesystem;

void FileEngine::RefreshIndex(std::function<void(size_t)> callback) {
    std::thread([this, callback]() {
        this->IndexingTask(callback);
    }).detach();
}

void FileEngine::IndexingTask(std::function<void(size_t)> callback) {
    std::vector<FileInfo> new_files;
    new_files.reserve(500000);

    std::vector<std::string> skip_prefixes = {"/proc", "/sys", "/dev", "/run", "/tmp", "/snap"};
    
    #ifdef _WIN32
    std::string root = "C:\\";
    #else
    std::string root = "/";
    #endif

    try {
        for (auto& p : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
            try {
                std::string path_str = p.path().string();
                
                #ifndef _WIN32
                bool skip = false;
                for(const auto& prefix : skip_prefixes) {
                    if (path_str.rfind(prefix, 0) == 0) {
                        skip = true;
                        break;
                    }
                }
                if (skip) continue;
                #endif

                if (p.is_regular_file()) {
                     new_files.push_back({p.path().filename().string(), path_str});
                }
            } catch (...) {}
        }
    } catch (...) {}

    size_t count = 0;
    {
        std::lock_guard<std::mutex> lock(mtx);
        files = std::move(new_files);
        count = files.size();
    }
    
    // Callback is likely called from thread, user needs to handle UI update properly (CallAfter)
    callback(count);
}

int FileEngine::FuzzyScore(const std::string& str, const std::string& pattern) {
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

std::vector<FileInfo> FileEngine::Search(const std::string& query, bool smartMatch) {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<FileInfo> result;
    
    if (query.empty()) {
        for(size_t i=0; i<files.size() && i<100; ++i) result.push_back(files[i]);
        return result;
    }

    if (smartMatch) {
        struct Match {
            const FileInfo* f;
            int score;
        };
        std::vector<Match> matches;
        
        for (const auto& file : files) {
            int score = FuzzyScore(file.name, query);
            if (score > 0) {
                matches.push_back({&file, score});
                if (matches.size() > 5000) break; // Hard limit for perf
            }
        }
        
        std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b){
            return a.score > b.score;
        });
        
        for(size_t i=0; i<matches.size() && i<200; ++i) {
            result.push_back(*matches[i].f);
        }
    } else {
        std::string q_lower = query;
        std::transform(q_lower.begin(), q_lower.end(), q_lower.begin(), ::tolower);
        
        for(const auto& file : files) {
            std::string n_lower = file.name;
            std::transform(n_lower.begin(), n_lower.end(), n_lower.begin(), ::tolower);
            
            if (n_lower.find(q_lower) != std::string::npos) {
                result.push_back(file);
                if (result.size() >= 200) break;
            }
        }
    }
    return result;
}

void FileEngine::OpenFile(const std::string& path) {
    wxLaunchDefaultApplication(path);
}
