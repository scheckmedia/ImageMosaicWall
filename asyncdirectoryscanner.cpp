#include "asyncdirectoryscanner.h"

AsyncDirectoryScanner::AsyncDirectoryScanner(const QString path,
                                             const QStringList nameFilters,
                                             QDir::QDir::Filters filters,
                                             QDirIterator::IteratorFlag flags,
                                             QObject *parent)
    : m_dirIterator(path, nameFilters, filters, flags)
    , QThread(parent)
    , m_isStopped(false)
    , m_fileList()

{
}

void AsyncDirectoryScanner::run()
{
    while (!m_isStopped && m_dirIterator.hasNext())
    {
        m_dirIterator.next();

        QString f = m_dirIterator.filePath();
        m_fileList.append(f);
        emit fileScanned(f);
    }
}

QStringList AsyncDirectoryScanner::scannedFiles() const
{
    return m_fileList;
}

int AsyncDirectoryScanner::numFilesScanned() const
{
    return m_fileList.size();
}

void AsyncDirectoryScanner::stop()
{
    m_isStopped = true;
}
