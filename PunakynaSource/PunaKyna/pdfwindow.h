#ifndef PDFWINDOW_H
#define PDFWINDOW_H

#include <QWidget>
#include <QPdfDocument>

/* Lainattu suoraan qpdf esimerkistä demoilua varten
 */
#include "pagerenderer.h"

class PDFWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PDFWindow(QWidget *parent = nullptr);
    ~PDFWindow();

    void paintEvent(QPaintEvent * event);
    qreal zoom() { return m_zoom; }
    qreal yForPage(int page);
    int topPageShowing() { return m_topPageShowing; }
    int bottomPageShowing() { return m_bottomPageShowing; }

    void setDocument(QPdfDocument *document);

public slots:
    void setZoom(qreal factor);
    void setZoomWindowWidth();
    void invalidate();

signals:
    void showingPageRange(int start, int end);
    void zoomChanged(qreal factor);

private slots:
    void documentStatusChanged();
    void pageLoaded(int page, qreal zoom, QImage image);

private:
    int pageCount();
    QSizeF pageSize(int page);
    void render(int page);

private:
    QHash<int, QImage> m_pageCache;
    QVector<int> m_cachedPagesLRU;
    int m_pageCacheLimit;
    QVector<QSizeF> m_pageSizes;
    PageRenderer *m_pageRenderer;
    QBrush m_background;
    QPixmap m_placeholderIcon;
    QBrush m_placeholderBackground;
    int m_pageSpacing;
    int m_topPageShowing;
    int m_bottomPageShowing;
    QSize m_totalSize;
    qreal m_zoom;
    qreal m_screenResolution; // pixels per point

    QPdfDocument *m_document;
};


#endif // PDFWINDOW_H
