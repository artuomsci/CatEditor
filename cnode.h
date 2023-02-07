#ifndef CNODE_H
#define CNODE_H

#include <QGraphicsEllipseItem>
#include <set>

class CArrow;

class CNode : public QObject,  public QGraphicsEllipseItem
{
   Q_OBJECT

public:
   CNode(qreal x_, qreal y_, const QString& name_);
   ~CNode();

   void Init();
   void DeInit();

   const std::set<CArrow*>& Children() const;
   void AddChild(CArrow* pArrow_);
   void RemoveChild(CArrow* pArrow_);
   void SetVisibility(bool visible_);

   void SetText(const QString& text_);
   QString GetText() const;

signals:
   void positionChanged(const CNode*);

protected:
   QVariant itemChange(GraphicsItemChange change_, const QVariant& value_) override;
   void paint(QPainter* pPainter_, const QStyleOptionGraphicsItem* pOption_, QWidget* pWidget_) override;

private:
   std::set<CArrow*>    m_children;
   QGraphicsTextItem*   m_pText     {};
};

#endif
