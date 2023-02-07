#ifndef SGRAPHICSVIEW_H
#define SGRAPHICSVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>

namespace Ui {
class SGraphicsView;
}

class SGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SGraphicsView(QWidget* pParent_ = nullptr);
    ~SGraphicsView();

protected:
   void wheelEvent(QWheelEvent* pEvent_) override;
   void mousePressEvent(QMouseEvent* pEvent_) override;
   void mouseReleaseEvent(QMouseEvent* pEvent_) override;
   void mouseMoveEvent(QMouseEvent* pEvent_) override;

   Ui::SGraphicsView*   m_pUi    {};
   int                  m_xpan   {};
   int                  m_ypan   {};
   bool                 m_drag   {};
};

#endif
