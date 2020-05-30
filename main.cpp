#include <QCoreApplication>
#include "client.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Client obj;
    return a.exec();
}
