#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "node.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Scene;
class QContextMenuEvent;
class QTableWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent_ = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* pEvent_) override;
    void resizeEvent(QResizeEvent* pEvent_) override;

private slots:
   void on_leShowLabel_editingFinished();
   void on_pbSaveImage_clicked();
   void updateInfo(const QString& str_);
   void updateNodeData(const std::list<cat::Function>& data_);
   void onNew();
   void onImport();
   void onLoad();
   void onSave();
   void onSaveAs();
   void onSelectAll();
   void onTableItemChanged(int row_, int col_);
   void TableKeyPressed(QKeyEvent* pKeyEvent_);
   void on_leFilter_editingFinished();

private:
   void createMenu();

   Ui::MainWindow*   ui       {};
   Scene*            m_pScene {};
   QString           m_currentFile;
};

#endif
