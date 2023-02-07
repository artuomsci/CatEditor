#include "sgraphicsview.h"
#include "ui_sgraphicsview.h"

#include <QScrollBar>

static const qreal neg_scale = 0.9;
static const qreal pos_scale = 1.1;

//----------------------------------------------------------------------
SGraphicsView::SGraphicsView(QWidget* pParent_) :
      QGraphicsView  (pParent_)
   ,  m_pUi          (new Ui::SGraphicsView)
{
   m_pUi->setupUi(this);

   setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

   setHorizontalScrollBarPolicy (Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
   setVerticalScrollBarPolicy   (Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}

//----------------------------------------------------------------------
SGraphicsView::~SGraphicsView()
{
   delete m_pUi;
}

//----------------------------------------------------------------------
void SGraphicsView::wheelEvent(QWheelEvent* pEvent_)
{
   if (pEvent_->modifiers() & Qt::ControlModifier)
   {
      auto anchor = transformationAnchor();
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

      int angle = pEvent_->angleDelta().y();

      qreal factor = angle > 0 ? pos_scale : neg_scale;

      scale(factor, factor);
      setTransformationAnchor(anchor);
   }
}

//----------------------------------------------------------------------
void SGraphicsView::mousePressEvent(QMouseEvent* pEvent_)
{
   if (pEvent_->button() == Qt::RightButton)
   {
      m_xpan = pEvent_->x();
      m_ypan = pEvent_->y();
      setCursor(Qt::ClosedHandCursor);
      pEvent_->accept();

      m_drag = true;
   }

   QGraphicsView::mousePressEvent(pEvent_);
}

//----------------------------------------------------------------------
void SGraphicsView::mouseReleaseEvent(QMouseEvent* pEvent_)
{
   if (pEvent_->button() == Qt::RightButton)
   {
      setCursor(Qt::ArrowCursor);
      pEvent_->accept();

      m_drag = false;
   }

   QGraphicsView::mouseReleaseEvent(pEvent_);
}

//----------------------------------------------------------------------
void SGraphicsView::mouseMoveEvent(QMouseEvent* pEvent_)
{
   if (m_drag)
   {
      horizontalScrollBar  ()->setValue(horizontalScrollBar ()->value() - (pEvent_->x() - m_xpan));
      verticalScrollBar    ()->setValue(verticalScrollBar   ()->value() - (pEvent_->y() - m_ypan));

      m_xpan = pEvent_->x();
      m_ypan = pEvent_->y();
   }

   QGraphicsView::mouseMoveEvent(pEvent_);
}