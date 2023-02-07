#include "carrow.h"
#include "common.h"
#include "cnode.h"

#ifdef WIN32
#include <corecrt_math_defines.h>
#else
#include <cmath>
#endif

#include <QPainterPath>
#include <QVector2D>

//----------------------------------------------------------------------
CArrow::CArrow(CNode* pSource_, CNode* pTarget_, const QString& name_) :
   m_pSource(pSource_),
   m_pTarget(pTarget_)
{
   setData(eID, QVariant(name_));

   m_pen = QPen(QColor(Qt::darkGray), Qt::SolidLine);

   m_pSource->AddChild(this);
   m_pTarget->AddChild(this);

   m_head.moveTo(0, 0);
   m_head.lineTo(-arrow_head_size, -arrow_head_size * 0.125);
   m_head.lineTo(-arrow_head_size,  arrow_head_size * 0.125);
   m_head.lineTo(0, 0);
   m_head.closeSubpath();

   UpdatePosition();
}

//----------------------------------------------------------------------
CArrow::~CArrow()
{
}

//----------------------------------------------------------------------
void CArrow::Init()
{

}

//----------------------------------------------------------------------
void CArrow::DeInit()
{
   m_pSource->RemoveChild(this);
   m_pTarget->RemoveChild(this);
}

//----------------------------------------------------------------------
QRectF CArrow::boundingRect() const
{
   return QRectF(
            std::min(m_x1, m_x2) - arrow_head_size, std::min(m_y1, m_y2) - arrow_head_size,
            std::max(m_x1, m_x2) + arrow_head_size, std::max(m_y1, m_y2) + arrow_head_size);
}

//----------------------------------------------------------------------
void CArrow::paint(QPainter* pPainter_, const QStyleOptionGraphicsItem* pOption_, QWidget* pWidget_)
{
   pPainter_->setPen(m_pen);
   pPainter_->drawLine(m_x1, m_y1, m_x2, m_y2);

   QVector2D dir = QVector2D(m_x2 - m_x1, m_y2 - m_y1).normalized();

   qreal tetha = std::atan2(dir.y(), dir.x()) * 180.0 / M_PI;

   pPainter_->translate(m_x2, m_y2);
   pPainter_->translate(-dir.x() * blob_radius, -dir.y() * blob_radius);
   pPainter_->rotate(tetha);

   pPainter_->fillPath(m_head, Qt::darkGray);
   pPainter_->drawPath(m_head);
}

//----------------------------------------------------------------------
void CArrow::UpdatePosition()
{
   prepareGeometryChange();

   auto source_pos = m_pSource->sceneBoundingRect().center();
   m_x1 = source_pos.x();
   m_y1 = source_pos.y();

   auto target_pos = m_pTarget->sceneBoundingRect().center();
   m_x2 = target_pos.x();
   m_y2 = target_pos.y();

   update();
}

//----------------------------------------------------------------------
void CArrow::SetVisibility(bool visible_)
{
   bool hide = !m_pSource->isVisible() || !m_pTarget->isVisible();
   setVisible(!hide);
}
