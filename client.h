#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QUdpSocket>
#include "log.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
signals:

public slots:

    void onReadyRead(); // обработчик tcp
    void readMessage(); // обработчик udp

private:
    QTcpSocket *_socket; // сокет
    QUdpSocket *_udpSocket; // сокет
    QString initPath = ".ini";
    quint16 port; // порт
    QString imagePath; // адрес папки в которую скачивать картинки
    QString logPath; // адрес папки для логирования
    QString host; // адрес хоста
    QByteArray bmp1Array;
    bool udp;//флаг использования udp  true - UDP  false - TCP
    Log *lg;//Указатель на класс логирования

    void initialization(); //инициализация
    void initAsDefault();//инициализация в случае отсутствия или повреждения ini файла
    void createInit(); //Создание init файла в случае его отсутствия или повреждения
    void parseData(QByteArray bmpArray);//Разборка полученных сообщений

signals:

};



#endif // CLIENT_H
