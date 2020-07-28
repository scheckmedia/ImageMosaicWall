#ifndef ASYNCDIRECTORYSCANNER_H
#define ASYNCDIRECTORYSCANNER_H

#include <QDirIterator>
#include <QObject>
#include <QThread>

class AsyncDirectoryScanner : public QThread
{
    Q_OBJECT
public:
    explicit AsyncDirectoryScanner(const QString path,
                                   const QStringList nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlag flags,
                                   QObject *parent = nullptr);

    void run();
    QStringList scannedFiles() const;
    int numFilesScanned() const;

private:
    bool m_isStopped;
    uint64_t m_numScannedFiles;
    QDirIterator m_dirIterator;
    QStringList m_fileList;

public slots:
    void stop();

signals:
    void fileScanned(const QString);
};

#endif // ASYNCDIRECTORYSCANNER_H
