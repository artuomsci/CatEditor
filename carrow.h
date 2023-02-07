#ifndef CARROW_H
#define CARROW_H

#include <QGraphicsItem>
#include <QPainter>

class CNode;

class CArrow : public QObject, public QGraphicsItem
{
   Q_OBJECT

public:
   CArrow(CNode* pSource_, CNode* pTarget_, const QString& name_);
   ~CArrow();
   void Init();
   void DeInit();
   void UpdatePosition();
   void SetVisibility(bool visible_);

protected:
   QRectF boundingRect() const override;
   void paint(QPainter* pPainter_, const QStyleOptionGraphicsItem* pOption_, QWidget* pWidget_) override;

private:
   CNode* m_pSource {};
   CNode* m_pTarget {};
   qreal m_x1 {}, m_y1 {}, m_x2 {}, m_y2 {};
   QPen  m_pen;
   QPainterPath m_head;
};

#endif
