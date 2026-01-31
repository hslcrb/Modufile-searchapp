#include <QApplication>
#include <QHeaderView>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include "mainwindow.h"
#include "fileengine.h"

// Custom Widget for List Items to ensure perfect visibility
class FileItemWidget : public QWidget {
public:
    FileItemWidget(const QString &name, const QString &path, QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 5, 10, 5);
        layout->setSpacing(2);

        QLabel *nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: #f8fafc;");
        
        QLabel *pathLabel = new QLabel(path);
        pathLabel->setStyleSheet("font-size: 12px; color: #94a3b8;");

        layout->addWidget(nameLabel);
        layout->addWidget(pathLabel);
        setLayout(layout);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("모두파일 (Modufile) - 네이티브 에디션");
    resize(1000, 750);

    // Global Dark Background
    setStyleSheet("QMainWindow { background-color: #0f172a; } "
                  "QWidget { color: #f1f5f9; }");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Search Bar Area
    QHBoxLayout *headerLayout = new QHBoxLayout();
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("찾고 싶은 파일 이름을 입력하세요...");
    m_searchInput->setStyleSheet("QLineEdit { padding: 15px; font-size: 17px; border-radius: 10px; "
                                 "background-color: #1e293b; border: 2px solid #334155; color: white; }"
                                 "QLineEdit:focus { border: 2px solid #a855f7; }");
    
    m_smartMatchCheck = new QCheckBox("알잘딱 매칭");
    m_smartMatchCheck->setChecked(true);
    m_smartMatchCheck->setStyleSheet("QCheckBox { font-weight: bold; margin-left: 10px; color: #e2e8f0; }");
    
    m_refreshBtn = new QPushButton("인덱싱 갱신");
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet("QPushButton { padding: 12px 25px; background-color: #a855f7; color: white; "
                                 "border-radius: 10px; font-weight: bold; font-size: 14px; border: none; }"
                                 "QPushButton:hover { background-color: #9333ea; }"
                                 "QPushButton:disabled { background-color: #475569; }");

    headerLayout->addWidget(m_searchInput, 1);
    headerLayout->addWidget(m_smartMatchCheck);
    headerLayout->addWidget(m_refreshBtn);
    mainLayout->addLayout(headerLayout);

    // Status Label
    m_statusLabel = new QLabel("서버 연결 중...");
    m_statusLabel->setStyleSheet("color: #cbd5e1; font-weight: 500;");
    mainLayout->addWidget(m_statusLabel);

    // Results List
    m_resultsList = new QListWidget();
    m_resultsList->setStyleSheet("QListWidget { background-color: #1e293b; border: 2px solid #334155; "
                                 "border-radius: 12px; outline: none; } "
                                 "QListWidget::item { background-color: transparent; border-bottom: 1px solid #1e293b; } "
                                 "QListWidget::item:selected { background-color: #334155; border-radius: 8px; }");
    m_resultsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mainLayout->addWidget(m_resultsList, 1);

    // Connections
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_smartMatchCheck, &QCheckBox::toggled, this, &MainWindow::onSmartMatchToggled);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_resultsList, &QListWidget::itemActivated, this, &MainWindow::onItemActivated);

    connect(&FileEngine::instance(), &FileEngine::indexingStarted, this, &MainWindow::onIndexingStarted);
    connect(&FileEngine::instance(), &FileEngine::indexingProgress, this, &MainWindow::onIndexingProgress);
    connect(&FileEngine::instance(), &FileEngine::indexingFinished, this, &MainWindow::onIndexingFinished);
    
    // Auto-refresh on start
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
    m_resultsList->clear();
    m_currentResults = FileEngine::instance().search(m_searchInput->text(), m_smartMatchCheck->isChecked());
    
    for (const auto &f : m_currentResults) {
        QListWidgetItem *item = new QListWidgetItem(m_resultsList);
        item->setSizeHint(QSize(0, 60)); 
        m_resultsList->addItem(item);
        
        FileItemWidget *widget = new FileItemWidget(f.name, f.path);
        m_resultsList->setItemWidget(item, widget);
        item->setData(Qt::UserRole, f.path);
    }
    
    if (m_currentResults.isEmpty()) {
        if (!m_searchInput->text().isEmpty())
            m_statusLabel->setText("검색 결과가 없습니다.");
    } else {
        m_statusLabel->setText(QString("검색 완료: %1개의 항목 발견").arg(m_currentResults.size()));
    }
}

void MainWindow::onRefreshClicked() {
    FileEngine::instance().refreshIndex();
}

void MainWindow::onIndexingStarted() {
    m_refreshBtn->setEnabled(false);
    m_statusLabel->setText("인덱싱 시작 중...");
}

void MainWindow::onIndexingProgress(int count) {
    m_statusLabel->setText(QString("실시간 인덱싱 중... (%L1개 파일 스캔 완료)").arg(count));
}

void MainWindow::onIndexingFinished(int count) {
    m_refreshBtn->setEnabled(true);
    m_statusLabel->setText(QString("인덱싱 완료! 총 %L1개의 파일을 찾았습니다.").arg(count));
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
