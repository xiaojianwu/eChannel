#include "filehandle.h"

FileHandle::FileHandle(QObject *parent)
    : QObject(parent)
{
    connect(this, SIGNAL(sigInit()), this, SLOT(onInit()));
    
}

FileHandle::~FileHandle()
{

}

void FileHandle::init(QString filePath)
{
    m_filePath = filePath;
    emit sigInit();
}

void FileHandle::onInit()
{
    m_file = new QFile;
    connect(this, SIGNAL(sigWrite(const QByteArray&)), this, SLOT(onWrite(const QByteArray&)));
    connect(this, SIGNAL(sigSetFilePath()), this, SLOT(onSetFilePath()));
    connect(this, SIGNAL(sigClose()), this, SLOT(onClose()));

    emit sigSetFilePath();
}


void FileHandle::setFilePath(QString filePath)
{
    m_filePath = filePath;
    emit sigSetFilePath();
}
void FileHandle::onSetFilePath()
{
    onClose();

    m_file->setFileName(m_filePath);
    m_file->open(QFile::ReadWrite | QFile::Truncate);
}

void FileHandle::write(const QByteArray& data)
{
    emit sigWrite(data);
}

void FileHandle::onWrite(const QByteArray& data)
{
    m_file->write(data);
}

void FileHandle::close()
{
    emit sigClose();
}

void FileHandle::onClose()
{
    if (m_file->isOpen())
    {
        m_file->flush();
        m_file->close();
    }
}


