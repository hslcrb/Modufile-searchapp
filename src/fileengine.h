#ifndef FILEENGINE_H
#define FILEENGINE_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QObject>
#include <QReadWriteLock>
#include <filesystem>
#include <thread>
#include <atomic>

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
    QVector<FileInfo> search(const QString &query, bool smartMatch);
    void openFile(const QString &path);

signals:
    void indexingStarted();
    void indexingFinished(int count);
    void indexingProgress(int count);

private:
    void indexingTask();
    int fuzzyScore(const QString &str, const QString &pattern);

    QVector<FileInfo> m_files;
    QReadWriteLock m_lock;
    std::atomic<bool> m_isIndexing{false};
};

#endif // FILEENGINE_H
