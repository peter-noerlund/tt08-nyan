#include <QApplication>

#include "window.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    tt06::Window window;
    window.show();

    return app.exec();
}