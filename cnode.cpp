#include "cnode.h"
#include "common.h"
#include "carrow.h"

#include <QGraphicsScene>
#include <QBrush>
#include <QStyleOptionGraphicsItem>

static const QColor select_color(150, 250, 150, 255);

//----------------------------------------------------------------------
CNode::CNode(qreal x_, qreal y_, const QString& name_)
{
   setData(eID, QVariant(name_));

   setRect(-blob_radius, -blob_radius, blob_radius * 2.0, blob_radius * 2.0);
   setPos(x_, y_);

   setFlag(QGraphicsItem::ItemIsSelectable);
   setFlag(QGraphicsItem::ItemIsMovable);
   setFlag(QGraphicsItem::ItemSendsGeometryChanges);
   setFlag(QGraphicsItem::ItemSendsScenePositionChanges);

   QBrush brush(QColor(Qt::lightGray), Qt::SolidPattern);

   setBrush (brush);
   setZValue(blob_layer);

   m_pText = new QGraphicsTextItem(name_);

   m_pText->setPos(QPointF(blob_radius, 0));
   m_pText->setParentItem(this);
   m_pText->setZValue(label_layer);
}

//----------------------------------------------------------------------
CNode::~CNode()
{
}

//----------------------------------------------------------------------
void CNode::Init()
{
}

//----------------------------------------------------------------------
void CNode::DeInit()
{
   auto backup = m_children;

   for (auto& arrow : backup)
   {
      arrow->DeInit();
      scene()->removeItem(arrow);
      delete arrow;
   }
}

//----------------------------------------------------------------------
QVariant CNode::itemChange(GraphicsItemChange change_, const QVariant& value_)
{
   if (change_ == ItemPositionHasChanged)
   {
      for (auto& arrow : m_children)
         arrow->UpdatePosition();

      emit positionChanged(this);
   }

   QBrush br = brush();
   br.setColor(isSelected() ? select_color : Qt::GlobalColor::gray);
   setBrush(br);

   return QGraphicsEllipseItem::itemChange(change_, value_);
}

//----------------------------------------------------------------------
void CNode::paint(QPainter* pPainter_, const QStyleOptionGraphicsItem* pOption_, QWidget* pWidget_)
{
   QStyleOptionGraphicsItem opt = *pOption_;
   opt.state.setFlag(QStyle::State_Selected, false);

   QGraphicsEllipseItem::paint(pPainter_, &opt, pWidget_);
}

//----------------------------------------------------------------------
const std::set<CArrow*>& CNode::Children() const
{
   return m_children;
}

//----------------------------------------------------------------------
void CNode::AddChild(CArrow* pArrow_)
{
   m_children.insert(pArrow_);
}

//----------------------------------------------------------------------
void CNode::RemoveChild(CArrow* pArrow_)
{
   m_children.erase(pArrow_);
}

//----------------------------------------------------------------------
void CNode::SetVisibility(bool visible_)
{
   QGraphicsEllipseItem::setVisible(visible_);

   for (CArrow* pArrow : m_children)
      pArrow->SetVisibility(visible_);
}

//----------------------------------------------------------------------
void CNode::SetText(const QString& text_)
{
   m_pText->setPlainText(text_);
}

//----------------------------------------------------------------------
QString CNode::GetText() const
{
   return m_pText->toPlainText();
}
