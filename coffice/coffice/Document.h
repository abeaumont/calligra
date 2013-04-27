#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QRectF>
#include <QImage>
#include <QSharedPointer>

class KoPart;

class Document;

class Page : public QObject
{
    Q_OBJECT
public:
    Page(Document *doc, int pageNumber, const QRectF &rect);
    virtual ~Page();
    Document* doc() const;
    int pageNumber() const;
    QRectF originalRect() const;
    QRectF rect() const;
    void setRect(const QRectF &r);
    bool isDirty() const;
    void setDirty(bool dirty);
    bool isUpdating() const;
    void setUpdating(bool updating);
Q_SIGNALS:
    void thumbnailFinished(const QImage &image);
private:
    class Private;
    Private *const d;
};

class Document : public QObject
{
    Q_OBJECT
public:
    explicit Document(QObject *parent = 0);
    virtual ~Document();

    QList< QSharedPointer<Page> > pages() const;

    QString file() const;
    bool openFile(const QString &file);
    void emitProgressUpdated(int percent);

Q_SIGNALS:
    //void openFileSucceeded();
    void openFileFailed(const QString &file, const QString &error);
    void progressUpdated(int percent);
    void layoutFinished();

public Q_SLOTS:
    void updatePage(const QSharedPointer<Page> &page);
private:
    friend class OpenFileCommand;
    friend class UpdatePageCommand;
    friend class AppManager;

    class Private;
    Private *const d;
};

#endif // DOCUMENT_H
