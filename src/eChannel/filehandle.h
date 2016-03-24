#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <QObject>

#include <QFile>

class FileHandle : public QObject
{
    Q_OBJECT

public:
    FileHandle(QObject *parent = NULL);
    ~FileHandle();


public:
    void write(const QByteArray& data);

    void init(QString filePath);

    void setFilePath(QString filePath);

    void close();


signals:
    void sigWrite(const QByteArray& data);
    void sigInit();
    void sigSetFilePath();
    void sigClose();

private slots:
    void onInit();
    void onSetFilePath();
    void onWrite(const QByteArray& data);
    void onClose();
private:
    QFile*      m_file;

    QString     m_filePath;
    
};

#endif // FILEHANDLE_H
