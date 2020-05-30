#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QByteArray>
#include <QDataStream>
#include <QString>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
signals:

public slots:

    void onReadyRead(); // обработчик

private:
    QTcpSocket _socket; // сокет
    QString initPath = ".ini";
    quint16 port; // порт
    QString path; // адрес папки в которую скачивать картинки
    QString host; // адрес хоста

    void initialization(); //инициализация
    void initAsDefault();//инициализация в случае отсутствия или повреждения ini файла
    void createInit(); //Создание init файла в случае его отсутствия или повреждения

signals:

};



#endif // CLIENT_H
