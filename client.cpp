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
    lg = new Log(logPath);
    lg->createLog("daemon client start");

    if(!udp)
    {
        //если сокет TCP
        _socket = new QTcpSocket();
        _socket->connectToHost(QHostAddress(host), port);
        connect(_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }
    else
    {
        //если socket UDP
        _udpSocket = new QUdpSocket(this);
        _udpSocket->bind(QHostAddress::Any);
        connect(_udpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));

        //Отправляем сообщения серверу чтоб он понял кому надо отсылать данные
        QByteArray arr;
        arr.append("message");
        _udpSocket->writeDatagram(arr, QHostAddress(host), port);
    }
}

void Client::initialization() // получение настроек
{
    //Создание папки bin
    if(!QDir("bin").exists())
    {
        QDir().mkdir("bin");
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
    //Считывание данных из файла .ini
    QFile file(initPath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        bool ok;
        bool ok_2;
        QString port_ = in.readLine(); //порт
        QString imagePath_ = in.readLine();//папка с картинками
        QString host_ = in.readLine();//хост
        QString logPath_ = in.readLine();// папка для логирования
        QString udp_ = in.readLine();// флаг использования UDP
        port_.toUInt(&ok,10);
        udp_.toInt(&ok_2, 10);
        if(!ok || !ok_2 || imagePath_ == "" || logPath_ == "" || host_ == "")
        {
            //если какая то переменная в файле не верна
            qDebug() << ".init file corrupted... initializate as default";
            createInit();
            initAsDefault();
        }
        else
        {
            port = port_.toUInt(&ok,10);
            imagePath = imagePath_;
            host = host_;
            logPath = logPath_;
            udp = udp_.toInt(&ok_2, 10);
            qDebug() << "initialization succes";
            qDebug() << "port = "<<port;
            qDebug() << "image path = "<<imagePath;
            qDebug() << "log path = "<<logPath;
            qDebug() << "use "<<(udp?"UPD":"TCP")<<" socket";
        }
     }
     else
     {
        //если файл конфига не открывается
        qDebug() << "cant open .ini file";
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
        out<<"log"<<endl;
        out<<"0"<<endl;
    }
    else
    {
        qDebug() << "Create .ini failed";
        return;
    }
    file.close();
    qDebug() << "Create .ini success";
}

void Client::initAsDefault() // инициализация по умолчанию
{
    qDebug() << "initialization with default configuration";
    port = 6666;
    qDebug() << "port = "<<port;
    imagePath = "Images";
    qDebug() << "path = "<<imagePath;
    host = "127.0.0.1";
    qDebug() << "host = "<<host;
    if(!QDir("Images").exists())
        QDir().mkdir("Images");
    logPath = "log";
    qDebug() << "log path = "<<logPath;
    udp = false;
    qDebug() << "use "<<(udp?"UPD":"TCP")<<" socket";
}

void Client::onReadyRead() // обработчик TCP
{
    qDebug() << "onread start";
    QTcpSocket* socket = static_cast<QTcpSocket *>(QObject::sender());
    lg->createLog(socket, " server connected ");
    QByteArray bmpArray;
    while(socket->bytesAvailable() || socket->waitForReadyRead())
    {
        bmpArray.append(socket->readAll());
    }
    qDebug() << "total size "<<bmpArray.size();
    lg->createLog(" succes download packet " + QString::number(bmpArray.size()));
    parseData(bmpArray);
    qDebug() << "onread end";
}

void Client::readMessage()//Обработчки UDP
{

    //Прослойка, служит обозначения последней датаграммы.
    //Используется только для UDP
    QString prob_3("@Fgaw-12awdd435dawd-?1^");
    //АДрес отправителя
    QHostAddress senderHost;
    //ПОрт отправителя
    quint16 senderPort;

    QByteArray tmpArray;
    while(_udpSocket->hasPendingDatagrams() || _udpSocket->bytesAvailable() >0)
    {
        tmpArray.resize(_udpSocket->pendingDatagramSize());
        _udpSocket->readDatagram(tmpArray.data(), tmpArray.size(), &senderHost, &senderPort);
        bmp1Array.append(tmpArray);
    }
    //Если получена последняя датаграмма
    if(!(bmp1Array.lastIndexOf(prob_3)+1))
    {
        //удаляем прослойку prob_3
        bmp1Array.remove(bmp1Array.lastIndexOf(prob_3),prob_3.size());
        //Парсим сообщения, создаём файлы итд
        parseData(bmp1Array);
        //чистим массив для повтороного использования
        bmp1Array.clear();
    }
}

void Client::parseData(QByteArray bmpArray) //Разборка полученных сообщений
{
    //Прослойка, служит для разделения раздела с описаниями файла (имя размер итд)
    QString prob_1("@FhK#-12");
    //Прослойка, служит для разделения раздела с описаниями файла и раздела с данными файла
    QString prob_2("@FhK#-12ddawd-?1^");

    while(bmpArray.contains(QByteArray().append(prob_2)))
    {
        QByteArray infoArray;//Раздел с описаниями файла
        QByteArray dataArray;//Раздел с данными файла
        QByteArray fileNameArray;//Название файла
        QByteArray fileSizeArray;//Размер файла
        //Парсинг начинается с конца
        //По сути это вариация метода split т.е разделяю bmpArray на блоки с разделами
        //разделители это prob_1 и prob_2

        //dataArray хранит в себе данные файла
        //infoArray - хранит в себе название и размер файла

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
        lg->createLog(" find " + fileName + " size " + fileSize + " totalsize "  + QString::number(dataArray.size())
                      +(dataArray.size() == fileSize.toInt() ?" Success" : " Failed" ) );

        //Если полученный размер данных не соответсвует количеству изначальных данных то файл не создётся
        if(fileSize.toInt()==dataArray.size())
        {
            QFile file(imagePath+"/"+fileName);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(dataArray);
            }
            file.close();
        }
    }
}
