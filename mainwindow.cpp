#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsView>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QDebug>
#include <QHeaderView>
#include <QAction>
#include <QSet>

#include "scene.h"
#include "common.h"
#include "ctable.h"
#include "str_utils.h"

using namespace cat;

static const float table_width_coef = 0.125f;

enum EProperty
{
      eName = 0
   ,  eType
   ,  eValue
};

//----------------------------------------------------------------------
MainWindow::MainWindow(QWidget* pParent_)
   : QMainWindow  (pParent_)
   , ui           (new Ui::MainWindow)
{
   ui->setupUi(this);

   m_pScene = new Scene;
   m_pScene->Init();

   ui->View->setScene(m_pScene);
   ui->View->show();

   ui->tw->setMinimumWidth(width() * table_width_coef);

   createMenu();

   connect(m_pScene, SIGNAL(updateStatistics(const QString&)), this, SLOT(updateInfo(const QString&)));
   connect(m_pScene, SIGNAL(updateNodeData(const std::list<cat::Function>&)), this, SLOT(updateNodeData(const std::list<cat::Function>&)));

   connect(ui->tw, SIGNAL(cellChanged(int, int)), this, SLOT(onTableItemChanged(int, int)));
   connect(ui->tw, SIGNAL(KeyPressed(QKeyEvent*)), this, SLOT(TableKeyPressed(QKeyEvent*)));

   updateNodeData(std::list<cat::Function>());
   updateInfo(m_pScene->Statistics());
}

//----------------------------------------------------------------------
void MainWindow::createMenu()
{
   QAction* pNewCategory = new QAction(tr("&New"), this);
   connect(pNewCategory, &QAction::triggered, this, &MainWindow::onNew);

   QAction* pFileLoad = new QAction(tr("&Load"), this);
   connect(pFileLoad, &QAction::triggered, this, &MainWindow::onLoad);

   QAction* pFileSave = new QAction(tr("&Save"), this);
   connect(pFileSave, &QAction::triggered, this, &MainWindow::onSave);

   QAction* pFileSaveAs = new QAction(tr("&SaveAs"), this);
   connect(pFileSaveAs, &QAction::triggered, this, &MainWindow::onSaveAs);

   auto pMenu = menuBar()->addMenu(tr("&File"));
   pMenu->addAction(pNewCategory);
   pMenu->addAction(pFileLoad);
   pMenu->addAction(pFileSave);
   pMenu->addAction(pFileSaveAs);

   QAction* pSelectAll = new QAction(tr("&SelectAll"), this);
   connect(pSelectAll, &QAction::triggered, this, &MainWindow::onSelectAll);

   auto pEditMenu = menuBar()->addMenu(tr("&Edit"));
   pEditMenu->addAction(pSelectAll);
}

//----------------------------------------------------------------------
MainWindow::~MainWindow()
{
   m_pScene->DeInit();
   delete m_pScene;
   delete ui;
}

//----------------------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent* pEvent_)
{
   QMainWindow::keyPressEvent(pEvent_);
}

//----------------------------------------------------------------------
void MainWindow::resizeEvent(QResizeEvent* pEvent_)
{
   QMainWindow::resizeEvent(pEvent_);

   ui->tw->setMinimumWidth(width() * table_width_coef);
}

//----------------------------------------------------------------------
void MainWindow::updateInfo(const QString& str_)
{
   ui->lbInfo->setText(str_);
}

//----------------------------------------------------------------------
void MainWindow::updateNodeData(const std::list<cat::Function>& data_)
{
   ui->tw->blockSignals(true);

   ui->tw->setRowCount(0);

   QStringList labels;
   labels << tr("Name") << tr("Type") << tr("Value");
   ui->tw->setHorizontalHeaderLabels(labels);

   for (const auto& it : data_)
   {
      int new_row = ui->tw->rowCount();
      ui->tw->insertRow(new_row);

      const auto& name  = it.first.c_str();
      const auto& value = it.second;

      auto prop_name = new QTableWidgetItem(name);
      prop_name->setFlags(prop_name->flags().setFlag(Qt::ItemIsEnabled, false));
      prop_name->setFlags(prop_name->flags() & ~Qt::ItemIsEditable);

      ui->tw->setItem(new_row, EProperty::eName, prop_name);

      if (const double* pVal = std::get_if<double>(&value))
      {
         {
            auto pItem = new QTableWidgetItem();
            pItem->setData(Qt::DisplayRole, cat_types.at((int)ESetTypes::eDouble));
            pItem->setFlags(pItem->flags().setFlag(Qt::ItemIsEnabled, false));

            ui->tw->setItem(new_row, EProperty::eType, pItem);
         }

         {
            auto pItem = new QTableWidgetItem();

            pItem->setData(Qt::DisplayRole, *pVal);

            ui->tw->setItem(new_row, EProperty::eValue, pItem);
            pItem->setToolTip(QString::number(*pVal));
         }
      }
      else if (const float* pVal = std::get_if<float>(&value))
      {
         {
            auto pItem = new QTableWidgetItem();
            pItem->setData(Qt::DisplayRole, cat_types.at((int)ESetTypes::eFloat));
            pItem->setFlags(pItem->flags().setFlag(Qt::ItemIsEnabled, false));

            ui->tw->setItem(new_row, EProperty::eType, pItem);
         }

         {
            auto pItem = new QTableWidgetItem();

            pItem->setData(Qt::DisplayRole, *pVal);

            ui->tw->setItem(new_row, EProperty::eValue, pItem);
            pItem->setToolTip(QString::number(*pVal));
         }
      }
      else if (const int* pVal = std::get_if<int>(&value))
      {
         {
            auto pItem = new QTableWidgetItem();
            pItem->setData(Qt::DisplayRole, cat_types.at((int)ESetTypes::eInt));
            pItem->setFlags(pItem->flags().setFlag(Qt::ItemIsEnabled, false));

            ui->tw->setItem(new_row, EProperty::eType, pItem);
         }

         {
            auto pItem = new QTableWidgetItem();

            pItem->setData(Qt::DisplayRole, *pVal);

            ui->tw->setItem(new_row, EProperty::eValue, pItem);
            pItem->setToolTip(QString::number(*pVal));
         }
      }
      else if (const std::string* pVal = std::get_if<std::string>(&value))
      {
         {
            auto pItem = new QTableWidgetItem();
            pItem->setData(Qt::DisplayRole, cat_types.at((int)ESetTypes::eString));
            pItem->setFlags(pItem->flags().setFlag(Qt::ItemIsEnabled, false));

            ui->tw->setItem(new_row, EProperty::eType, pItem);
         }

         {
            auto pItem = new QTableWidgetItem();

            pItem->setData(Qt::DisplayRole, QString((*pVal).c_str()));
            pItem->setToolTip(QString((*pVal).c_str()));

            ui->tw->setItem(new_row, EProperty::eValue, pItem);
         }
      }
   }

   ui->tw->resizeColumnsToContents();
   ui->tw->blockSignals(false);
}

//----------------------------------------------------------------------
void MainWindow::onNew()
{
   m_pScene->New();

   m_currentFile.clear();
   setWindowTitle(m_currentFile);
}

//----------------------------------------------------------------------
void MainWindow::onLoad()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Open data"), "", tr("Data files (*.dat)"));
   if (fileName.isEmpty())
      return;

   m_pScene->New();

   if (m_pScene->LoadBinary(fileName))
   {
      m_currentFile = fileName;
      setWindowTitle(m_currentFile);
   }
   else
   {
      m_currentFile.clear();
      setWindowTitle(m_currentFile);
   }
}

//----------------------------------------------------------------------
void MainWindow::onSave()
{
   if (!m_pScene->SaveBinary(m_currentFile))
      onSaveAs();
}

//----------------------------------------------------------------------
void MainWindow::onSaveAs()
{
   QString fileName = QFileDialog::getSaveFileName(const_cast<MainWindow*>(this), tr("Save data"), "", tr("Data files (*.dat)"));
   if (fileName.isEmpty())
      return;

   fileName = fileName.contains(".dat") ? fileName : fileName + ".dat";

   if (m_pScene->SaveBinary(fileName))
      m_currentFile = fileName;
   else
      m_currentFile.clear();
}

//----------------------------------------------------------------------
void MainWindow::onSelectAll()
{
   const QList<QGraphicsItem*>& items = m_pScene->items();
   for (const auto& it : items)
      it->setSelected(true);
}

//----------------------------------------------------------------------
void MainWindow::onTableItemChanged(int row_, int col_)
{
   QTableWidgetItem* pItemName   = ui->tw->item(row_, EProperty::eName);
   QTableWidgetItem* pItemType   = ui->tw->item(row_, EProperty::eType);
   QTableWidgetItem* pItem       = ui->tw->item(row_, EProperty::eValue);

   Function fn;
   fn.first = pItemName->text().toStdString();

   if       (pItemType->data(Qt::DisplayRole).toString() == cat_types.at((int)ESetTypes::eInt))
      fn.second = pItem->data(Qt::DisplayRole).value<int>();
   else if  (pItemType->data(Qt::DisplayRole).toString() == cat_types.at((int)ESetTypes::eFloat))
      fn.second = pItem->data(Qt::DisplayRole).value<float>();
   else if  (pItemType->data(Qt::DisplayRole).toString() == cat_types.at((int)ESetTypes::eDouble))
      fn.second = pItem->data(Qt::DisplayRole).value<double>();
   else if  (pItemType->data(Qt::DisplayRole).toString() == cat_types.at((int)ESetTypes::eString))
      fn.second = pItem->data(Qt::DisplayRole).value<QString>().toStdString();

   m_pScene->RemovePropertyFromNode(nullptr, fn.first);
   m_pScene->AddProperty2Node(nullptr, fn);
}

//----------------------------------------------------------------------
void MainWindow::TableKeyPressed(QKeyEvent* pKeyEvent_)
{
   if (pKeyEvent_->key() == Qt::Key_Delete)
   {
      if (QTableWidgetItem* pItem = ui->tw->item(ui->tw->currentRow(), eName))
      {
         m_pScene->RemovePropertyFromNode(nullptr, pItem->text().toStdString());
         ui->tw->removeRow(ui->tw->currentRow());
      }
   }
}

//----------------------------------------------------------------------
void MainWindow::on_leFilter_editingFinished()
{
   m_pScene->Filter(ui->leFilter->text());

   ui->twTable->clear();
   ui->twTable->setRowCount(0);

   QSet<QString> headers;

   for (auto& it : m_pScene->GetDescription())
   {
      for (QMap<QString, QString>::iterator rec = it.begin(); rec != it.end(); ++rec)
      {
         headers.insert(rec.key());
      }
   }

   QStringList labels;
   for (const auto& it : headers)
      labels.push_back(it);
   
   ui->twTable->setColumnCount(labels.size());

   ui->twTable->setHorizontalHeaderLabels(labels);

   for (auto& it : m_pScene->GetDescription())
   {
      int new_row = ui->twTable->rowCount();
      ui->twTable->insertRow(new_row);

      int count{};
      for (QMap<QString, QString>::iterator rec = it.begin(); rec != it.end(); ++rec)
      {
         auto pItem = new QTableWidgetItem();

         pItem->setData(Qt::DisplayRole, rec.value());

         headers.find(rec.key());

         auto pos = std::distance(headers.begin(), headers.find(rec.key()));

         ui->twTable->setItem(new_row, pos, pItem);
      }
   }

   ui->twTable->resizeColumnsToContents();
}

//----------------------------------------------------------------------
void MainWindow::on_pbSaveImage_clicked()
{
   const QString fileName = QFileDialog::getSaveFileName(this, tr("Save configuration"), "", tr("Image File (*.png)"));

   ui->View->grab().save(fileName.contains(".png") ? fileName : fileName + ".png");
}

//----------------------------------------------------------------------
void MainWindow::on_leShowLabel_editingFinished()
{
   m_pScene->ChangeLabel(ui->leShowLabel->text());
}
