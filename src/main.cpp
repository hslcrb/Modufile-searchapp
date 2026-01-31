#include "mainwindow.h"
#include <QApplication>
#include <QHeaderView>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("모두파일 (Modufile) - 네이티브 C++ 에디션");
    resize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("파일 이름 검색...");
    m_searchInput->setStyleSheet("padding: 8px; font-size: 14px; border-radius: 4px; border: 1px solid #ccc;");
    
    m_smartMatchCheck = new QCheckBox("알잘딱 (스마트 매칭)");
    m_smartMatchCheck->setChecked(true);
    
    m_refreshBtn = new QPushButton("인덱싱 갱신");
    m_refreshBtn->setStyleSheet("padding: 8px 15px; background-color: #a855f7; color: white; border-radius: 4px; font-weight: bold;");

    headerLayout->addWidget(m_searchInput, 1);
    headerLayout->addWidget(m_smartMatchCheck);
    headerLayout->addWidget(m_refreshBtn);
    mainLayout->addLayout(headerLayout);

    // Status
    m_statusLabel = new QLabel("대기 중...");
    m_statusLabel->setStyleSheet("color: #666; font-style: italic;");
    mainLayout->addWidget(m_statusLabel);

    // List
    m_resultsList = new QListWidget();
    m_resultsList->setStyleSheet("QListWidget { border: 1px solid #eee; border-radius: 4px; background: white; }"
                                 "QListWidget::item { padding: 10px; border-bottom: 1px solid #f9f9f9; }"
                                 "QListWidget::item:selected { background: #f3e8ff; color: #7e22ce; }");
    mainLayout->addWidget(m_resultsList, 1);

    // Signal connections
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_smartMatchCheck, &QCheckBox::toggled, this, &MainWindow::onSmartMatchToggled);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_resultsList, &QListWidget::itemActivated, this, &MainWindow::onItemActivated);

    connect(&FileEngine::instance(), &FileEngine::indexingStarted, this, &MainWindow::onIndexingStarted);
    connect(&FileEngine::instance(), &FileEngine::indexingFinished, this, &MainWindow::onIndexingFinished);
    
    // Initial indexing
    QTimer::singleShot(200, this, &MainWindow::onRefreshClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onSearchTextChanged(const QString &) {
    performSearch();
}

void MainWindow::onSmartMatchToggled(bool) {
    performSearch();
}

void MainWindow::performSearch() {
    m_currentResults = FileEngine::instance().search(m_searchInput->text(), m_smartMatchCheck->isChecked());
    updateList(m_currentResults);
    m_statusLabel->setText(QString("검색 결과: %1개").arg(m_currentResults.size()));
}

void MainWindow::updateList(const QVector<FileInfo> &results) {
    m_resultsList->clear();
    for (const auto &f : results) {
        QListWidgetItem *item = new QListWidgetItem(m_resultsList);
        item->setText(f.name + "\n" + f.path);
        item->setData(Qt::UserRole, f.path);
    }
}

void MainWindow::onRefreshClicked() {
    FileEngine::instance().refreshIndex();
}

void MainWindow::onIndexingStarted() {
    m_refreshBtn->setEnabled(false);
    m_statusLabel->setText("시스템 파일을 스캐닝하고 있습니다... 잠시만 기다려 주세요.");
}

void MainWindow::onIndexingFinished(int count) {
    m_refreshBtn->setEnabled(true);
    m_statusLabel->setText(QString("인덱싱 완료. 총 %1개의 파일을 찾았습니다.").arg(count));
    performSearch();
}

void MainWindow::onItemActivated(QListWidgetItem *item) {
    QString path = item->data(Qt::UserRole).toString();
    FileEngine::instance().openFile(path);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
