#include <QApplication>
#include <QDebug>
#include "UI/windows/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Initialize Qt resources
    Q_INIT_RESOURCE(resources);
    qDebug() << "Resources initialized";
    
    // Test if resource exists
    QIcon testIcon(":/icons/add.svg");
    if (!testIcon.isNull()) {
        qDebug() << "Icon loaded successfully!";
    } else {
        qDebug() << "ERROR: Icon failed to load!";
    }
    
    // Set application info
    app.setApplicationName("Garage Management");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("PBL2");
    
    // Set style
    app.setStyle("Fusion");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
