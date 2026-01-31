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
#include <QIcon>
#include <QPixmap>
#include "mainwindow.h"
#include "fileengine.h"

// Minimalist Item Widget with Icon
class FileItemWidget : public QWidget {
public:
    FileItemWidget(const QString &name, const QString &path, QWidget *parent = nullptr) : QWidget(parent) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(15);

        // Icon
        QLabel *iconLabel = new QLabel;
        iconLabel->setFixedSize(32, 32);
        
        QString lowerName = name.toLower();
        QString iconPath = ":/resources/icons/file.svg";
        
        if (lowerName.endsWith(".jpg") || lowerName.endsWith(".png") || lowerName.endsWith(".gif") || lowerName.endsWith(".webp") || lowerName.endsWith(".svg")) {
            iconPath = ":/resources/icons/image.svg";
        } else if (lowerName.endsWith(".mp4") || lowerName.endsWith(".avi") || lowerName.endsWith(".mkv") || lowerName.endsWith(".mov")) {
            iconPath = ":/resources/icons/video.svg";
        } else if (lowerName.endsWith(".cpp") || lowerName.endsWith(".h") || lowerName.endsWith(".py") || lowerName.endsWith(".js") || lowerName.endsWith(".html") || lowerName.endsWith(".css") || lowerName.endsWith(".json") || lowerName.endsWith(".md")) {
            iconPath = ":/resources/icons/code.svg";
        }
        
        QPixmap pixmap(iconPath);
        iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        layout->addWidget(iconLabel);

        // Text Info
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        textLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("font-weight: 600; font-size: 15px; color: #f1f5f9;");
        
        QLabel *pathLabel = new QLabel(path);
        pathLabel->setStyleSheet("font-size: 12px; color: #64748b;"); // Muted slate color

        textLayout->addWidget(nameLabel);
        textLayout->addWidget(pathLabel);
        
        layout->addLayout(textLayout);
        layout->addStretch(); // Push everything to left
        
        setLayout(layout);
        setAttribute(Qt::WA_TransparentForMouseEvents); // Let the list widget handle clicks
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Modufile"); // Clean title
    resize(1000, 750);

    // Ultra Dark & Clean Theme
    setStyleSheet("QMainWindow { background-color: #0b0f19; }"
                  "QWidget { font-family: 'Inter', sans-serif; }"
                  "QScrollBar:vertical { border: none; background: #0b0f19; width: 8px; margin: 0px; }"
                  "QScrollBar::handle:vertical { background: #334155; min-height: 20px; border-radius: 4px; }"
                  "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // --- Minimalist Header ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(15);

    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("검색...");
    m_searchInput->setStyleSheet("QLineEdit { padding: 16px; font-size: 18px; border: none; border-radius: 12px; "
                                 "background-color: #1a202c; color: #e2e8f0; selection-background-color: #8b5cf6; }"
                                 "QLineEdit:focus { background-color: #2d3748; }");
    
    // Icon-only refresh button idea or simple text
    m_refreshBtn = new QPushButton("갱신");
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setFixedWidth(80);
    m_refreshBtn->setStyleSheet("QPushButton { padding: 16px; background-color: #1a202c; color: #94a3b8; "
                                 "border-radius: 12px; font-weight: 600; border: none; font-size: 14px; }"
                                 "QPushButton:hover { background-color: #2d3748; color: white; }"
                                 "QPushButton:pressed { background-color: #4a5568; }");

    m_smartMatchCheck = new QCheckBox("Smart");
    m_smartMatchCheck->setChecked(true);
    m_smartMatchCheck->setStyleSheet("QCheckBox { color: #64748b; font-weight: 600; spacing: 8px; } "
                                     "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 1px solid #475569; } "
                                     "QCheckBox::indicator:checked { background-color: #8b5cf6; border: none; }");

    headerLayout->addWidget(m_searchInput, 1);
    headerLayout->addWidget(m_smartMatchCheck);
    headerLayout->addWidget(m_refreshBtn);
    mainLayout->addLayout(headerLayout);

    // --- List Area ---
    m_resultsList = new QListWidget();
    m_resultsList->setFrameShape(QFrame::NoFrame); // No borders
    m_resultsList->setStyleSheet("QListWidget { background-color: transparent; outline: none; }"
                                 "QListWidget::item { background-color: transparent; border-bottom: 1px solid #1e293b; padding: 0px; }"
                                 "QListWidget::item:selected { background-color: #1e293b; border-radius: 12px; border-bottom: none; }");
    m_resultsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_resultsList->setUniformItemSizes(true); // Optimization
    mainLayout->addWidget(m_resultsList, 1);

    // --- Minimal Footer ---
    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #475569; font-size: 12px; font-weight: 500;");
    m_statusLabel->setAlignment(Qt::AlignRight);
    mainLayout->addWidget(m_statusLabel);

    // Connections
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_smartMatchCheck, &QCheckBox::toggled, this, &MainWindow::onSmartMatchToggled);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_resultsList, &QListWidget::itemActivated, this, &MainWindow::onItemActivated);

    connect(&FileEngine::instance(), &FileEngine::indexingStarted, this, &MainWindow::onIndexingStarted);
    connect(&FileEngine::instance(), &FileEngine::indexingProgress, this, &MainWindow::onIndexingProgress);
    connect(&FileEngine::instance(), &FileEngine::indexingFinished, this, &MainWindow::onIndexingFinished);
    
    // Async Search Connection
    connect(&FileEngine::instance(), &FileEngine::searchFinished, this, &MainWindow::onSearchFinished);
    
    // Auto-refresh on start
    QTimer::singleShot(500, this, &MainWindow::onRefreshClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onSearchTextChanged(const QString &) {
    performSearch();
}

void MainWindow::onSmartMatchToggled(bool) {
    performSearch();
}

void MainWindow::performSearch() {
    m_statusLabel->setText("searching...");
    FileEngine::instance().searchAsync(m_searchInput->text(), m_smartMatchCheck->isChecked());
}

void MainWindow::onSearchFinished(QVector<FileInfo> results) {
    m_resultsList->clear();
    m_currentResults = results;
    
    int limit = qMin(results.size(), 200); 

    for (int i = 0; i < limit; ++i) {
        const auto& f = results[i];
        QListWidgetItem *item = new QListWidgetItem(m_resultsList);
        item->setSizeHint(QSize(0, 72)); 
        m_resultsList->addItem(item);
        
        FileItemWidget *widget = new FileItemWidget(f.name, f.path);
        m_resultsList->setItemWidget(item, widget);
        item->setData(Qt::UserRole, f.path);
    }
    
    if (m_currentResults.isEmpty() && !m_searchInput->text().isEmpty()) {
        m_statusLabel->setText("no results");
    } else {
        m_statusLabel->setText(QString("%1").arg(m_currentResults.size()));
    }
}

void MainWindow::onRefreshClicked() {
    FileEngine::instance().refreshIndex();
}

void MainWindow::onIndexingStarted() {
    m_refreshBtn->setEnabled(false);
    m_statusLabel->setText("indexing...");
}

void MainWindow::onIndexingProgress(int count) {
    m_statusLabel->setText(QString("indexing %1").arg(count));
}

void MainWindow::onIndexingFinished(int count) {
    m_refreshBtn->setEnabled(true);
    m_statusLabel->setText(QString("ready (%1 files)").arg(count));
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
