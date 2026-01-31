#include "fileengine.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <algorithm>

namespace fs = std::filesystem;

FileEngine::FileEngine(QObject *parent) : QObject(parent) {}

FileEngine::~FileEngine() {}

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
    QVector<FileInfo> newFiles;
    newFiles.reserve(1000000);

    QStringList skipDirs;
#ifdef Q_OS_WIN
    QString root = "C:/";
    skipDirs << "C:/Windows" << "C:/$Recycle.Bin" << "C:/System Volume Information";
#else
    QString root = "/";
    skipDirs << "/proc" << "/sys" << "/dev" << "/run" << "/tmp" << "/var/lib" << "/var/cache";
#endif

    try {
        for (auto const& entry : fs::recursive_directory_iterator(root.toStdString(), fs::directory_options::skip_permission_denied)) {
            try {
                auto path = QString::fromStdString(entry.path().string());
                
                bool skip = false;
                for (const auto& s : skipDirs) {
                    if (path.startsWith(s)) {
                        skip = true;
                        break;
                    }
                }
                if (skip) {
                    if (entry.is_directory()) {
                        // We can't easily tell the iterator to skip from here without more complex logic
                        // but std::filesystem is fast enough for now.
                    }
                    continue;
                }

                if (entry.is_regular_file()) {
                    newFiles.append({QString::fromStdString(entry.path().filename().string()), path});
                    if (newFiles.size() % 10000 == 0) {
                        emit indexingProgress(newFiles.size());
                    }
                }
            } catch (...) {
                continue;
            }
        }
    } catch (...) {}

    {
        QWriteLocker locker(&m_lock);
        m_files = std::move(newFiles);
    }
    m_isIndexing = false;
    emit indexingFinished(m_files.size());
}

int FileEngine::fuzzyScore(const QString &str, const QString &pattern) {
    int score = 0;
    int run = 0;
    int strIdx = 0;
    int patIdx = 0;

    QString s = str.toLower();
    QString p = pattern.toLower();

    while (strIdx < s.length() && patIdx < p.length()) {
        if (s[strIdx] == p[patIdx]) {
            score += 10 + (run * 5);
            run++;
            patIdx++;
        } else {
            run = 0;
            score -= 1;
        }
        strIdx++;
    }

    if (patIdx < p.length()) return 0;
    return qMax(1, score);
}

QVector<FileInfo> FileEngine::search(const QString &query, bool smartMatch) {
    QReadLocker locker(&m_lock);
    QVector<FileInfo> results;

    if (query.isEmpty()) {
        for (int i = 0; i < qMin(100, (int)m_files.size()); ++i) {
            results.append(m_files[i]);
        }
        return results;
    }

    if (smartMatch) {
        struct Match {
            const FileInfo* info;
            int score;
        };
        QVector<Match> matches;
        matches.reserve(m_files.size() / 10);

        for (const auto& f : m_files) {
            int score = fuzzyScore(f.name, query);
            if (score > 0) {
                matches.append({&f, score});
                if (matches.size() > 10000) break;
            }
        }

        std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
            return a.score > b.score;
        });

        for (int i = 0; i < qMin(200, matches.size()); ++i) {
            results.append(*matches[i].info);
        }
    } else {
        QString q = query.toLower();
        for (const auto& f : m_files) {
            if (f.name.toLower().contains(q)) {
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
