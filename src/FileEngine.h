#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <filesystem>

struct FileInfo {
    std::string name;
    std::string path;
};

class FileEngine {
public:
    static FileEngine& Get() {
        static FileEngine instance;
        return instance;
    }

    void RefreshIndex(std::function<void(size_t)> callback);
    std::vector<FileInfo> Search(const std::string& query, bool smartMatch);
    void OpenFile(const std::string& path);

private:
    std::vector<FileInfo> files;
    std::mutex mtx;
    
    void IndexingTask(std::function<void(size_t)> callback);
    int FuzzyScore(const std::string& str, const std::string& pattern);
};
