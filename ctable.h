#ifndef CTABLE_H
#define CTABLE_H

#include <QTableWidget>

class CTable : public QTableWidget
{
   Q_OBJECT

public:
   CTable(QWidget* pParent_ = nullptr);
   ~CTable();

signals:
   void KeyPressed(QKeyEvent*);

protected:
   void keyPressEvent(QKeyEvent* pEvent_) override;
};

#endif
