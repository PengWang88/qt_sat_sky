#include <QApplication>
#include "widgets/satsky.h"

#if NOVA_DEBUG
    #include <windows.h>
    #define USE_UTF8_CONSOLE SetConsoleOutputCP(CP_UTF8);
#else
    #define USE_UTF8_CONSOLE 
#endif

int main(int argc, char *argv[])
{
    USE_UTF8_CONSOLE

    QApplication app(argc, argv);
    app.setApplicationName("SatSky");

    // 注册Qt资源
    Q_INIT_RESOURCE(icons);

    SatSky window;
    window.show();

    return app.exec();
}