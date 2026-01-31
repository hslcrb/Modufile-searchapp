#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "fileengine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSearchTextChanged(const QString &text);
    void onRefreshClicked();
    void onIndexingStarted();
    void onIndexingFinished(int count);
    void onItemActivated(QListWidgetItem *item);
    void onSmartMatchToggled(bool checked);

private:
    void performSearch();
    // void updateList(const QVector<FileInfo> &results); // Removed as it is now handled inline

    QLineEdit *m_searchInput;
    QListWidget *m_resultsList;
    QPushButton *m_refreshBtn;
    QLabel *m_statusLabel;
    QCheckBox *m_smartMatchCheck;
    
    QVector<FileInfo> m_currentResults;
};

#endif // MAINWINDOW_H
