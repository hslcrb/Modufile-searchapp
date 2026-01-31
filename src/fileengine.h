#ifndef FILEENGINE_H
#define FILEENGINE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QReadWriteLock>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <filesystem>
#include <atomic>
#include <mutex>

struct FileInfo {
    QString name;
    QString path;
};

class FileEngine : public QObject {
    Q_OBJECT
public:
    explicit FileEngine(QObject *parent = nullptr);
    ~FileEngine();

    static FileEngine& instance();

    void refreshIndex();
    void searchAsync(const QString &query, bool smartMatch);
    // Legacy sync method kept for CLI or other needs, but internally calls the same logic if possible or keeps old logic?
    // Let's keep existing search() but optimize it, and add searchAsync purely as a wrapper.
    QVector<FileInfo> search(const QString &query, bool smartMatch); 
    void openFile(const QString &path);

signals:
    void indexingStarted();
    void indexingFinished(int count);
    void indexingProgress(int count);
    void searchFinished(QVector<FileInfo> results);

private:
    void indexingTask();
    
    // Static helper for concurrent operations
    static int fuzzyScore(const QString &str, const QString &pattern);

    QVector<FileInfo> m_files;
    QReadWriteLock m_lock;
    std::atomic<bool> m_isIndexing{false};
    
    QFutureWatcher<QVector<FileInfo>> m_searchWatcher;
};

#endif // FILEENGINE_H
