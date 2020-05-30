#include "client.h"

#include <QByteArray>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QHostAddress>


Client::Client(QObject *parent) :
    QObject(parent)
{
    initialization();
    _socket.connectToHost(QHostAddress(host), port);
    connect(&_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void Client::initialization() // получение настроек
{
    //Создание папки bin
    if(!QDir("bin").exists())
    {
        QDir().mkdir("bin");
    }
    //Создание папки log
    if(!QDir("log").exists())
    {
        QDir().mkdir("log");
    }
    //Создание папки etc
    if(!QDir("etc").exists())
    {
        QDir().mkdir("etc");
    }
    //Проверка наличия .ini файла
    if(!QFile(initPath).exists())
    {
        createInit();
        initAsDefault();
        return;
    }

    QFile file(initPath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        bool ok;
        QString port_ = in.readLine();
        QString path_ = in.readLine();
        QString host_ = in.readLine();

        port = port_.toUInt(&ok,10);
        path = path_;
        host = host_;

        if(!ok || path_ == "" || host_ == "")
        {
            qDebug() << ".init file corrupted... initializate as default";
            createInit();
            initAsDefault();
        }
        else
        {
            qDebug() << "initialization succes";
            qDebug() << "port = "<<port;
            qDebug() << "path = "<<path;
            qDebug() << "host = "<<host_;
        }
     }
     else
     {
        qDebug() << "cant open .init file";
        initAsDefault();
     }
     file.close();
}

void Client::createInit() // получение настроек
{
    QFile file(initPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out<<"6666"<<endl;
        out<<"Images"<<endl;
        out<<"127.0.0.1"<<endl;
        if(!QDir("Images").exists())
            QDir().mkdir("Images");
    }
    else
    {
        qDebug() << "Create .ini failed";
        return;
    }
    file.close();
    qDebug() << "Create .ini success";
}

void Client::initAsDefault() // получение настроек
{
    qDebug() << "initialization with default configuration";
    port = 6666;
    qDebug() << "port = "<<port;
    path = "Images";
    qDebug() << "path = "<<path;
    host = "127.0.0.1";
    qDebug() << "host = "<<host;
    if(!QDir("Images").exists())
        QDir().mkdir("Images");
}

void Client::onReadyRead() // обработчик
{
    //Прослойка, служит для разделения раздела с описаниями файла (имя размер итд)
    QString prob_1("@FhK#-12");
    //Прослойка, служит для разделения раздела с описаниями файла и раздела с данными файла
    QString prob_2("@FhK#-12ddawd-?1^");

    qDebug() << "onread start";
    QTcpSocket* socket = static_cast<QTcpSocket *>(QObject::sender());

    QByteArray bmpArray;
    while(socket->bytesAvailable() || socket->waitForReadyRead())
    {
        bmpArray.append(socket->readAll());
    }
    qDebug() << "total size "<<bmpArray.size();

    while(bmpArray.contains(QByteArray().append(prob_2)))
    {
        QByteArray infoArray;//Раздел с описаниями файла
        QByteArray dataArray;//Раздел с данными файла
        QByteArray fileNameArray;//Название файла
        QByteArray fileSizeArray;//Размер файла
        //Парсинг начинается с конца
        //По сути это вариация метода split т.е разделяю bmpArray на блоки с разделами
        //разделители это prob_1 и prob_2
        dataArray = bmpArray.mid(bmpArray.lastIndexOf(prob_2));
        bmpArray.remove(bmpArray.lastIndexOf(prob_2),dataArray.size());
        dataArray.remove(dataArray.lastIndexOf(prob_2),prob_2.size());
        if(bmpArray.contains(QByteArray().append(prob_2)))
        {
            infoArray = bmpArray.mid(bmpArray.lastIndexOf(prob_2));
            bmpArray.remove(bmpArray.lastIndexOf(prob_2),infoArray.size());
            infoArray.remove(infoArray.lastIndexOf(prob_2),prob_2.size());
        }
        else
        {
            infoArray = bmpArray;
            bmpArray.clear();
        }

        fileSizeArray = infoArray.mid(infoArray.lastIndexOf(prob_1));
        infoArray.remove(infoArray.lastIndexOf(prob_1),fileSizeArray.size());
        fileSizeArray.remove(fileSizeArray.lastIndexOf(prob_1),prob_1.size());
        fileNameArray = infoArray;

        QString fileName(fileNameArray);
        QString fileSize(fileSizeArray);

        qDebug() << "img - "<<fileName<<" size("<<fileSize.toInt()<<") totalsize("<<dataArray.size()<<")";

        QFile file(path+"/"+fileName);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(dataArray);
        }
        file.close();
    }
    qDebug() << "onread end";
}
