#include "ctable.h"

#include <QKeyEvent>

//----------------------------------------------------------------------
CTable::CTable(QWidget* pParent_) :
   QTableWidget(pParent_)
{
}

//----------------------------------------------------------------------
CTable::~CTable()
{
}

//----------------------------------------------------------------------
void CTable::keyPressEvent(QKeyEvent* pEvent_)
{
   QTableWidget::keyPressEvent(pEvent_);

   emit KeyPressed(pEvent_);
}
