#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include "fileengine.h"
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "========================================";
    qDebug() << "   Modufile CLI Diagnostics Tool";
    qDebug() << "   (Checking core engine functionality)";
    qDebug() << "========================================";

    FileEngine &engine = FileEngine::instance();

    QObject::connect(&engine, &FileEngine::indexingStarted, [](){
        qDebug() << "[Status] Indexing Started...";
    });

    QObject::connect(&engine, &FileEngine::indexingProgress, [](int count){
        std::cout << "\r[Status] Indexed files: " << count << std::flush;
    });

    QObject::connect(&engine, &FileEngine::indexingFinished, [&](int count){
        std::cout << "\n";
        qDebug() << "[Status] Indexing Finished!";
        qDebug() << "[Result] Total indexed files:" << count;
        
        if (count == 0) {
            qDebug() << "[Error] No files found. Check permissions or exclusion logic.";
            QCoreApplication::exit(1);
            return;
        }

        // Perform a test search
        QString query = "modufile"; 
        qDebug() << "\n[Test] Searching for 'modufile'...";
        
        auto results = engine.search(query, true); // Smart match
        qDebug() << "[Test] Found" << results.size() << "matches.";

        for(int i = 0; i < std::min(5, (int)results.size()); ++i) {
            qDebug() << "  -" << results[i].name << "(" << results[i].path << ")";
        }

        QCoreApplication::quit();
    });

    // Start indexing
    engine.refreshIndex();

    return app.exec();
}
