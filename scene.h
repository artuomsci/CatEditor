#ifndef SCENE_H
#define SCENE_H

#include <memory>

#include <QGraphicsScene>
#include <QMap>

#include "node.h"
#include "carrow.h"
#include "cnode.h"

class QMenu;
class QAction;
class QGraphicsSceneMouseEvent;
class QContextMenuEvent;

class Scene : public QGraphicsScene
{
   Q_OBJECT

public:
   Scene();
   ~Scene();

   void Init();
   void DeInit();

   bool AddProperty2Node(QGraphicsItem* pItem_, const cat::Function& property_);
   void RemovePropertyFromNode(QGraphicsItem* pItem_, const cat::FunctionName& name_);

   void OnContextMenu();
   QString Statistics();
   void New();
   void Filter(const QString& filter_);
   void ChangeLabel(const QString& name_);
   QList<QMap<QString, QString>> GetDescription() const;

   bool Build(const QString& path_);
   bool LoadBinary(const QString& path_);
   bool SaveBinary(const QString& path_) const;

protected:
   void mousePressEvent(QGraphicsSceneMouseEvent* pEvent_) override;
   void mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent_) override;
   void mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent_) override;
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* pEvent_) override;
   void dragMoveEvent(QGraphicsSceneDragDropEvent* pEvent_) override;

signals:
   void updateStatistics(const QString&);
   void updateNodeData(const std::list<cat::Function>&);

private slots:
   void selectionChanged();
   void slotActivated(QAction* pAction_);
   void positionChanged(const CNode* pNode_);

private:
   CNode* createNode(const QString& name_, const QPointF& pos_);
   CNode* createNode(const cat::Node& node_, const QPointF& pos_);
   CArrow* createArrow(CNode* pSource_, CNode* pTarget_, const QString& name_, std::list<cat::Function> pFns_);
   QGraphicsItem* getItem(const QString& name_) const;
   void changeLabel(QGraphicsItem* pItem_) const;
   QMap<QString, QString> getRecord(QGraphicsItem* pItem_) const;

   std::shared_ptr<cat::Node>
                          m_pLCategory   {};
   QGraphicsItem*         m_pSource      {};
   QPointF                m_LastMousePos;
   cat::FunctionName      m_ShownName;

   QMenu*                 m_pMnu         {};
   QAction*               m_pAddProp     {};
   QAction*               m_pClone       {};
   QAction*               m_pCreateArrow {};
   QAction*               m_pDeleteArrow {};
};

#endif
