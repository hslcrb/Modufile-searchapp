#include "fileengine.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <algorithm>

namespace fs = std::filesystem;

FileEngine::FileEngine(QObject *parent) : QObject(parent) {
    connect(&m_searchWatcher, &QFutureWatcher<QVector<FileInfo>>::finished, this, [this]() {
        emit searchFinished(m_searchWatcher.result());
    });
}

FileEngine::~FileEngine() {
    if (m_searchWatcher.isRunning()) {
        m_searchWatcher.waitForFinished();
    }
}

FileEngine& FileEngine::instance() {
    static FileEngine inst;
    return inst;
}

void FileEngine::refreshIndex() {
    if (m_isIndexing) return;
    m_isIndexing = true;
    emit indexingStarted();
    std::thread([this]() { this->indexingTask(); }).detach();
}

void FileEngine::indexingTask() {
    // Clear existing index at start
    {
        QWriteLocker locker(&m_lock);
        m_files.clear();
        m_files.reserve(1000000); 
    }

    QVector<FileInfo> chunk;
    chunk.reserve(5000);

    QStringList skipDirs;
#ifdef Q_OS_WIN
    QString root = "C:/";
    skipDirs << "C:/Windows" << "C:/$Recycle.Bin" << "C:/System Volume Information" << "C:/ProgramData" << "C:/PerfLogs";
#else
    QString root = "/";
    skipDirs << "/proc" << "/sys" << "/dev" << "/run" << "/tmp" << "/var/lib" << "/var/cache" << "/snap" 
             << "/timeshift" << "/mnt" << "/media" << "/lost+found" << "/.trash" << "/boot" << "/srv";
#endif

    qDebug() << "인덱싱 시작: " << root;

    try {
        auto it = fs::recursive_directory_iterator(root.toStdString(), fs::directory_options::skip_permission_denied);
        auto end = fs::recursive_directory_iterator();

        while (it != end) {
            try {
                auto path = QString::fromStdString(it->path().string());
                
                bool skip = false;
                for (const auto& s : skipDirs) {
                    if (path.startsWith(s)) {
                        skip = true;
                        break;
                    }
                }
                
                if (skip) {
                    it.disable_recursion_pending();
                    ++it;
                    continue;
                }

                if (it->is_regular_file()) {
                    QString name = QString::fromStdString(it->path().filename().string());
                    chunk.append({name, path, name.toLower()});
                    
                    // Commit chunk every 5000 files
                    if (chunk.size() >= 5000) {
                        {
                            QWriteLocker locker(&m_lock);
                            m_files.append(chunk);
                        }
                        emit indexingProgress(m_files.size());
                        chunk.clear();
                    }
                }
                ++it;
            } catch (const std::exception& e) {
                try { ++it; } catch (...) { break; }
            } catch (...) {
                try { ++it; } catch (...) { break; }
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "치명적 인덱싱 오류: " << e.what();
    }

    // Append remaining files
    if (!chunk.isEmpty()) {
        QWriteLocker locker(&m_lock);
        m_files.append(chunk);
    }

    m_isIndexing = false;
    qDebug() << "인덱싱 완료. 총 파일 수: " << m_files.size();
    emit indexingFinished(m_files.size());
}

int FileEngine::fuzzyScore(const QString &strLower, const QString &patternLower) {
    if (patternLower.isEmpty()) return 0;
    
    // Highly optimized fuzzy match score using pre-lowercased strings
    int score = 0;
    int run = 0;
    
    const int sLen = strLower.length();
    const int pLen = patternLower.length();
    
    if (pLen > sLen) return 0;

    int strIdx = 0;
    int patIdx = 0;

    // Bonus for starting match
    if (strLower.startsWith(patternLower)) {
        score += 50; 
    }

    // Matching
    while (strIdx < sLen && patIdx < pLen) {
        if (strLower[strIdx] == patternLower[patIdx]) {
            int charScore = 10;
            
            // Consecutive match bonus (compounding)
            if (run > 0) charScore += (run * 10);
            
            score += charScore;
            run++;
            patIdx++;
        } else {
            run = 0;
            score -= 1; // Small penalty for gaps
        }
        strIdx++;
    }

    if (patIdx < pLen) return 0; // Incomplete match
    
    // Penalize long strings for short queries slightly to prefer exact matches
    score -= (sLen - pLen); 

    return qMax(1, score);
}

void FileEngine::searchAsync(const QString &query, bool smartMatch) {
    if (m_searchWatcher.isRunning()) {
        m_searchWatcher.cancel();
    }

    QFuture<QVector<FileInfo>> future = QtConcurrent::run([this, query, smartMatch]() {
        return this->search(query, smartMatch);
    });
    
    m_searchWatcher.setFuture(future);
}

QVector<FileInfo> FileEngine::search(const QString &query, bool smartMatch) {
    QVector<FileInfo> currentFiles;
    {
        QReadLocker locker(&m_lock);
        currentFiles = m_files; 
    }
    
    QVector<FileInfo> results;
    if (query.isEmpty()) {
        int limit = qMin(100, (int)currentFiles.size());
        for (int i = 0; i < limit; ++i) {
            results.append(currentFiles[i]);
        }
        return results;
    }

    QString qLower = query.toLower();

    if (smartMatch) {
        struct Match {
            const FileInfo* info;
            int score;
        };

        std::vector<Match> matches;
        matches.reserve(qMin((int)currentFiles.size(), 20000));

        // Use nameLower directly for massive speedup
        for (const auto& f : currentFiles) {
            if (m_searchWatcher.isCanceled()) return {}; 

            int score = fuzzyScore(f.nameLower, qLower);
            if (score > 0) {
                matches.push_back({&f, score});
                if (matches.size() > 50000) break;
            }
        }

        std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
            if (a.score != b.score) return a.score > b.score;
            return a.info->nameLower.length() < b.info->nameLower.length();
        });

        int limit = qMin(200, (int)matches.size());
        for (int i = 0; i < limit; ++i) {
            results.append(*matches[i].info);
        }
    } else {
        // Simple contains check
        for (const auto& f : currentFiles) {
            if (m_searchWatcher.isCanceled()) return {};

            if (f.nameLower.contains(qLower)) {
                results.append(f);
                if (results.size() >= 200) break;
            }
        }
    }

    return results;
}

void FileEngine::openFile(const QString &path) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
