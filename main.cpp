#include "RepRapMiniHost.h"

#include <QtGui>
#include <QApplication>
#include "RepRapHost.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RepRapMiniHost w;
    w.show();
    return a.exec();
}
