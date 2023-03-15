#include "scene.h"

#include <QGraphicsView>
#include <QInputDialog>
#include <QMessageBox>
#include <QUuid>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QAction>

#include <type_traits>
#include <assert.h>
#include <fstream>
#include <sstream>

#include "common.h"
#include "str_utils.h"
#include "tokenizer.h"
#include "parser.h"
#include <iostream>

static const char* x_token       = "x_";
static const char* y_token       = "y_";
static const char* id_token      = "id_";

static const char* sEquality     = "==";
static const char* sNEquality    = "!=";
static const char* sOR           = "||";
static const char* sAND          = "&&";

static const char* sSet          = "set";

static const char* sVoid         = "void";

using namespace cat;

//-----------------------------------------------------------------------------------------
static std::string trim_sp_s(const std::string& string_, std::string::value_type symbol_)
{
   return trim_right(trim_left(string_, symbol_), symbol_);
}

//-----------------------------------------------------------------------------------------
static std::string trim_sp(const std::string& string_)
{
   return trim_sp_s(string_, ' ');
}

//----------------------------------------------------------------------
static std::string toID(const QGraphicsItem* const pItem_)
{
   return pItem_->data(eID).toString().toStdString();
}

//----------------------------------------------------------------------
static std::string rnd_uuid()
{
   return QUuid::createUuid().toString().mid(1, 36).toStdString();
}

//----------------------------------------------------------------------
static std::list<Function> function_values(const std::string& source_, const std::string& target_, std::shared_ptr<cat::Node> pNode_)
{
   auto target = pNode_->QueryNodes(target_);
   if (target.empty())
      return std::list<Function>();

   Arrow::List morphisms = pNode_->QueryArrows("* :: " + source_ + "->" + target_);
   if (morphisms.empty())
      return std::list<Function>();

   std::list<Function> fns;

   for (const auto& function : morphisms.front().QueryArrows("* :: * -> *"))
   {
      auto vals = target.front().QueryNodes(function.Target());
      if (!vals.empty())
         fns.emplace_back(function.Name(), vals.front().GetValue());
   }

   return fns;
}

//----------------------------------------------------------------------
Scene::Scene()
{
   Init();

   m_pMnu = new QMenu(NULL);

   m_pAddProp     = m_pMnu->addAction(tr("Add property"));
   m_pClone       = m_pMnu->addAction(tr("Clone"));
   m_pCreateArrow = m_pMnu->addAction(tr("Create arrow"));
   m_pDeleteArrow = m_pMnu->addAction(tr("Delete arrows"));

   connect(m_pMnu, SIGNAL(triggered(QAction*)), SLOT(slotActivated(QAction*)));

   connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

//----------------------------------------------------------------------
Scene::~Scene()
{
   DeInit();
}

//----------------------------------------------------------------------
void Scene::Init()
{
   if (m_pLCategory)
      return;

   m_pLCategory.reset(new Node(Node::NName("Root"), Node::EType::eSCategory));

   setSceneRect(0, 0, scene_size, scene_size);

   emit updateStatistics(Statistics());
}

//----------------------------------------------------------------------
void Scene::DeInit()
{
   if (!m_pLCategory)
      return;

   m_pSource = nullptr;

   m_pLCategory = nullptr;

   clear();

   emit updateStatistics(Statistics());
}

//----------------------------------------------------------------------
bool Scene::AddProperty2Node(QGraphicsItem* pItem_, const Function& property_)
{
   if (!pItem_)
      pItem_ = m_pSource;

   if (!pItem_ || !m_pLCategory)
      return false;

   const auto& [fn_name, fn_value] = property_;

   auto name = toID(pItem_);

   Arrow::List arrows = m_pLCategory->QueryArrows("* :: " + name + "->" + sSet);
   if (arrows.empty())
      return false;

   Arrow& arrow = arrows.front();

   m_pLCategory->EraseArrow(arrow.Name());

   auto target_set = rnd_uuid();

   arrow.AddArrow(Arrow(Arrow::EType::eFunction, sVoid, target_set, fn_name));

   {
      Node::List nodes = m_pLCategory->QueryNodes(name);
      if (!nodes.empty())
      {
         auto& source = nodes.front();

         source.AddNode(Node(sVoid, Node::EType::eSet));

         m_pLCategory->ReplaceNode(source);
      }
   }

   {
      Node::List nodes = m_pLCategory->QueryNodes(sSet);
      if (!nodes.empty())
      {
         auto target = nodes.front();

         Node node = Node(target_set, Node::EType::eSet);
         node.SetValue(fn_value);

         target.AddNode(node);

         m_pLCategory->ReplaceNode(target);
      }
   }

   m_pLCategory->AddArrow(arrow);

   emit updateNodeData(function_values(name, sSet, m_pLCategory));

   changeLabel(pItem_);

   return true;
}

//----------------------------------------------------------------------
void Scene::RemovePropertyFromNode(QGraphicsItem* pItem_, const cat::FunctionName& name_)
{
   if (!pItem_)
      pItem_ = m_pSource;

   if (!pItem_ || !m_pLCategory)
      return;

   Arrow::List arrows = m_pLCategory->QueryArrows("*::" + toID(pItem_) + "->" + sSet);
   if (arrows.empty())
      return;

   Arrow& arrow = arrows.front();

   m_pLCategory->EraseArrow(arrow.Name());

   Arrow::List functions = arrow.QueryArrows(name_ + " :: * -> *");
   if (functions.empty())
      return;

   arrow.EraseArrow(name_);

   m_pLCategory->AddArrow(arrow);

   {
      Node::List nodes = m_pLCategory->QueryNodes(arrow.Source());
      if (!nodes.empty())
      {
         auto& source = nodes.front();

         source.EraseNode(functions.front().Source());

         m_pLCategory->ReplaceNode(source);
      }
   }

   {
      Node::List nodes = m_pLCategory->QueryNodes(arrow.Target());
      if (!nodes.empty())
      {
         auto target = nodes.front();

         target.EraseNode(functions.front().Target());

         m_pLCategory->ReplaceNode(target);
      }
   }

   emit updateNodeData(function_values(toID(pItem_), sSet, m_pLCategory));
}

//----------------------------------------------------------------------
void Scene::OnContextMenu()
{
   if (m_pSource)
      m_pMnu->exec(QCursor::pos());
}

//----------------------------------------------------------------------
QString Scene::Statistics()
{
   if (!m_pLCategory)
      return "";

   return tr("Stat: ") +
      QString::number(m_pLCategory->CountNodes()) + tr(" nodes, ") +
      QString::number(m_pLCategory->CountArrows() - m_pLCategory->CountNodes()) + tr(" arrows");
}

//----------------------------------------------------------------------
void Scene::mousePressEvent(QGraphicsSceneMouseEvent* pEvent_)
{
   QGraphicsScene::mousePressEvent(pEvent_);
}

//----------------------------------------------------------------------
void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent_)
{
   QGraphicsScene::mouseMoveEvent(pEvent_);

   m_LastMousePos = pEvent_->scenePos();
}

//----------------------------------------------------------------------
void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent_)
{
   QGraphicsScene::mouseReleaseEvent(pEvent_);

   if (pEvent_->button() == Qt::RightButton)
      OnContextMenu();
}

//----------------------------------------------------------------------
void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* pEvent_)
{
   QGraphicsScene::mouseDoubleClickEvent(pEvent_);

   if (!m_pLCategory)
      return;

   auto selected = selectedItems();

   if (selected.size() != 1)
   {
      bool answer {};
      auto node_name = QInputDialog::getText(NULL, tr("Create node"), tr("Node name"), QLineEdit::Normal, tr(""), &answer);
      if (!answer)
         return;

      if (node_name.isEmpty())
         node_name = rnd_uuid().c_str();

      createNode(node_name, pEvent_->scenePos());

      emit updateStatistics(Statistics());
   }
   else
   {
      if (QMessageBox::question(NULL, tr("Delete node?"), tr("Are you sure?"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
      {
         if (m_pLCategory->EraseNode(toID(selected.front())))
         {
            auto item = selected.front();

            static_cast<CNode*>(item)->DeInit();
            removeItem(item);
            delete item;

            emit updateStatistics(Statistics());
         }
      }
   }
}

//----------------------------------------------------------------------
void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* pEvent_)
{
   QGraphicsScene::dragMoveEvent(pEvent_);
}

//----------------------------------------------------------------------
void Scene::selectionChanged()
{
   if (!m_pLCategory)
      return;

   auto items = selectedItems();

   if (items.empty())
   {
      m_pSource = nullptr;

      emit updateNodeData(std::list<Function>());
   }

   if (items.size() == 1)
   {
      m_pSource = items.at(0);

      emit updateNodeData(function_values(toID(m_pSource), sSet, m_pLCategory));
   }
}

//----------------------------------------------------------------------
void Scene::slotActivated(QAction* pAction_)
{
   if (!pAction_)
      return;

   if (pAction_ == m_pAddProp)
   {
      auto name = QInputDialog::getText(NULL, tr("Property name"), tr("Name"));
      if (name.isEmpty())
         return;

      auto type = QInputDialog::getItem(NULL, tr("Choose property type"), tr("Type"), cat_types);

      if (type == cat_types.at((int)cat::ESetTypes::eString))
      {
         auto value = QInputDialog::getText(NULL, tr("Property value"), tr("Value"));
         cat::Function prop(name.toStdString(), value.toStdString());
         AddProperty2Node(m_pSource, prop);
      }
      else if (type == cat_types.at((int)cat::ESetTypes::eInt))
      {
         auto value = QInputDialog::getInt(NULL, tr("Property value"), tr("Value"));
         cat::Function prop(name.toStdString(), int(value));
         AddProperty2Node(m_pSource, prop);
      }
      else if (type == cat_types.at((int)cat::ESetTypes::eDouble))
      {
         auto value = QInputDialog::getDouble(NULL, tr("Property value"), tr("Value"));
         cat::Function prop(name.toStdString(), double(value));
         AddProperty2Node(m_pSource, prop);
      }
      else if (type == cat_types.at((int)cat::ESetTypes::eFloat))
      {
         auto value = QInputDialog::getDouble(NULL, tr("Property value"), tr("Value"));
         cat::Function prop(name.toStdString(), float(value));
         AddProperty2Node(m_pSource, prop);
      }

      if (name == x_token || name == y_token)
         positionChanged(static_cast<CNode*>(m_pSource));
   }
   else if (pAction_ == m_pClone && m_pSource && m_pLCategory)
   {
      auto old_name = toID    (m_pSource);
      auto new_name = rnd_uuid();

      m_pLCategory->CloneNode(old_name, new_name);

      CNode* pNewNode = new CNode(m_LastMousePos.x(), m_LastMousePos.y(), new_name.c_str());
      addItem(pNewNode);

      connect(pNewNode, &CNode::positionChanged, this, &Scene::positionChanged);

      emit updateStatistics(Statistics());

      // outward
      {
         Arrow::List new_arrows = m_pLCategory->QueryArrows("*::" + new_name + "->*");

         for (Arrow& arrow : new_arrows)
         {
            if (arrow.Source() == arrow.Target())
               continue;

            QGraphicsItem* pTargetNode = getItem(arrow.Target().c_str());

            CArrow* pItem = new CArrow(pNewNode, (CNode*)pTargetNode, arrow.Name().c_str());
            addItem(pItem);
         }
      }

      // inward
      {
         Arrow::List new_arrows = m_pLCategory->QueryArrows("*::*->" + new_name);

         for (Arrow& arrow : new_arrows)
         {
            if (arrow.Source() == arrow.Target())
               continue;

            QGraphicsItem* pSourceNode = getItem(arrow.Source().c_str());

            CArrow* pItem = new CArrow((CNode*)pSourceNode, pNewNode, arrow.Name().c_str());
            addItem(pItem);
         }
      }
   }
   else if (pAction_ == m_pDeleteArrow && m_pSource && selectedItems().size() == 2 && m_pLCategory)
   {
      auto sitems = selectedItems();

      CNode* source = static_cast<CNode*>(sitems.at(0));
      CNode* target = static_cast<CNode*>(sitems.at(1));

      if (m_pSource != source)
         std::swap(source, target);

      Arrow::List arrows = m_pLCategory->QueryArrows("*::" + toID(source) + "->" + toID(target));
      if (!arrows.empty())
      {
         for (const auto& it : arrows)
         {
            if (m_pLCategory->EraseArrow(it.Name()))
            {
               if (CArrow* pNode = (CArrow*)getItem(it.Name().c_str()))
               {
                  removeItem(pNode);
                  ((CArrow*)pNode)->DeInit();
                  delete pNode;
               }
            }
         }
      }

      emit updateStatistics(Statistics());
   }
   else if (pAction_ == m_pCreateArrow && m_pSource && selectedItems().size() == 2)
   {
      bool ok{};
      auto arrow_name = QInputDialog::getText(NULL, tr("Create arrow"), tr("Arrow name"), QLineEdit::Normal, "", &ok);

      if (ok)
      {
         auto items = selectedItems();

         CNode* source = static_cast<CNode*>(items.at(0));
         CNode* target = static_cast<CNode*>(items.at(1));

         if (m_pSource != source)
            std::swap(source, target);

         createArrow(source, target, arrow_name, std::list<cat::Function>());

         emit updateStatistics(Statistics());
      }

      emit updateNodeData(std::list<cat::Function>());
   }
}

//----------------------------------------------------------------------
void Scene::positionChanged(const CNode* pNode_)
{
   if (!m_pLCategory)
      return;

   std::list<Function> fns = function_values(toID(pNode_), sSet, m_pLCategory);

   for (Function& fn : fns)
   {
      if (fn.first == x_token)
      {
         fn.second = (int)pNode_->pos().x();
         RemovePropertyFromNode((QGraphicsItem*)pNode_, fn.first);
         AddProperty2Node((QGraphicsItem*)pNode_, fn);
      }

      if (fn.first == y_token)
      {
         fn.second = (int)pNode_->pos().y();
         RemovePropertyFromNode((QGraphicsItem*)pNode_, fn.first);
         AddProperty2Node((QGraphicsItem*)pNode_, fn);
      }
   }
}

//----------------------------------------------------------------------
CNode* Scene::createNode(const QString& name_, const QPointF& pos_)
{
   if (!m_pLCategory)
      return nullptr;

   if (!m_pLCategory->AddNode(Node(name_.toStdString(), Node::EType::eObject)))
      return nullptr;

   CNode* pItem = new CNode(pos_.x(), pos_.y(), name_);
   addItem(pItem);

   connect(pItem, &CNode::positionChanged, this, &Scene::positionChanged);

   return pItem;
}

//----------------------------------------------------------------------
CNode* Scene::createNode(const cat::Node& node_, const QPointF& pos_)
{
   if (!m_pLCategory)
      return nullptr;

   if (!m_pLCategory->AddNode(node_))
      return nullptr;

   CNode* pItem = new CNode(pos_.x(), pos_.y(), node_.Name().c_str());
   addItem(pItem);

   connect(pItem, &CNode::positionChanged, this, &Scene::positionChanged);

   return pItem;
}

//----------------------------------------------------------------------
CArrow* Scene::createArrow(CNode* pSource_, CNode* pTarget_, const QString& name_, std::list<cat::Function> pFns_)
{
   if (!m_pLCategory)
      return nullptr;

   auto source_name = pSource_->data(eID).toString();
   auto target_name = pTarget_->data(eID).toString();

   Arrow arrow = name_.isEmpty() ?
      Arrow(Arrow::EType::eMorphism, source_name.toStdString(), target_name.toStdString()) :
      Arrow(Arrow::EType::eMorphism, source_name.toStdString(), target_name.toStdString(), name_.toStdString());

   Node::List nodes = m_pLCategory->QueryNodes(target_name.toStdString());
   if (nodes.empty())
      return nullptr;

   for (const auto& it : pFns_)
   {
      auto target_set = rnd_uuid();

      arrow.AddArrow(Arrow(Arrow::EType::eFunction, sVoid, target_set, it.first));

      if (!nodes.empty())
      {
         Node node = Node(target_set, Node::EType::eSet);
         node.SetValue(it.second);

         nodes.front().AddNode(node);
      }
   }

   m_pLCategory->ReplaceNode(nodes.front());

   if (!m_pLCategory->AddArrow(arrow))
      return nullptr;

   CArrow* pItem = new CArrow(pSource_, pTarget_, arrow.Name().c_str());
   addItem(pItem);

   return pItem;
}

//----------------------------------------------------------------------
QGraphicsItem* Scene::getItem(const QString& name_) const
{
   const QList<QGraphicsItem*> allItems = items();
   for (QGraphicsItem* pItem : allItems)
   {
      if (name_ == pItem->data(eID).toString())
         return pItem;
   }

   return nullptr;
}

//----------------------------------------------------------------------
void Scene::changeLabel(QGraphicsItem* pItem_) const
{
   if (CNode* pItem = dynamic_cast<CNode*>(pItem_))
   {
      auto id = pItem->data(eID);
      if (!id.isValid())
      {
         pItem->SetText("");
         return;
      }

      auto node_name = id.toString().toStdString();

      if (m_ShownName.empty())
      {
         pItem->SetText(node_name.c_str());
         return;
      }

      Arrow::List arrows = m_pLCategory->QueryArrows("*::" + node_name + "->" + sSet);
      if (arrows.empty())
      {
         pItem->SetText("");
         return;
      }

      const Arrow& arrow = arrows.front();

      std::list<Function> fns = function_values(arrow.Source(), arrow.Target(), m_pLCategory);

      for (Function& f : fns)
      {
         if (f.first == m_ShownName)
         {
            if (const double* pVal = std::get_if<double>(&f.second))
            {
               pItem->SetText(QString::number(*pVal));
            }
            else if (const float* pVal = std::get_if<float>(&f.second))
            {
               pItem->SetText(QString::number(*pVal));
            }
            else if (const int* pVal = std::get_if<int>(&f.second))
            {
               pItem->SetText(QString::number(*pVal));
            }
            else if (const std::string* pVal = std::get_if<std::string>(&f.second))
            {
               pItem->SetText((*pVal).c_str());
            }
         }
      }
   }
}

//----------------------------------------------------------------------
QMap<QString, QString> Scene::getRecord(QGraphicsItem* pItem_) const
{
   if (CNode* pItem = dynamic_cast<CNode*>(pItem_))
   {
      QMap<QString, QString> ret;

      auto id = pItem->data(eID);
      if (!id.isValid())
         return ret;

      auto node_name = id.toString();

      ret.insert("id", node_name);

      Arrow::List arrows = m_pLCategory->QueryArrows("*::" + node_name.toStdString() + "->" + sSet);
      if (arrows.empty())
         return ret;

      const Arrow& arrow = arrows.front();

      std::list<Function> fns = function_values(arrow.Source(), arrow.Target(), m_pLCategory);

      for (Function& f : fns)
      {
         if (const double* pVal = std::get_if<double>(&f.second))
         {
            ret.insert(f.first.c_str(), QString::number(*pVal));
         }
         else if (const float* pVal = std::get_if<float>(&f.second))
         {
            ret.insert(f.first.c_str(), QString::number(*pVal));
         }
         else if (const int* pVal = std::get_if<int>(&f.second))
         {
            ret.insert(f.first.c_str(), QString::number(*pVal));
         }
         else if (const std::string* pVal = std::get_if<std::string>(&f.second))
         {
            ret.insert(f.first.c_str(), QString((*pVal).c_str()));
         }
      }

      return ret;
   }

   return QMap<QString, QString>();
}

//----------------------------------------------------------------------
static void readif(std::ifstream& stream_, std::string& str_)
{
   size_t string_sz{};
   stream_.read((char*)&string_sz, sizeof(string_sz));

   str_.resize(string_sz);

   stream_.read(str_.data(), string_sz);
}

//----------------------------------------------------------------------
static void readif(std::ifstream& stream_, TSetValue& value_)
{
   size_t type_ind{};
   stream_.read((char*)&type_ind, sizeof(type_ind));

   if (type_ind == (size_t)ESetTypes::eDouble)
   {
      double val{};
      stream_.read((char*)&val, sizeof(val));
      value_ = val;
   }
   else if (type_ind == (size_t)ESetTypes::eFloat)
   {
      float val{};
      stream_.read((char*)&val, sizeof(val));
      value_ = val;
   }
   else if (type_ind == (size_t)ESetTypes::eInt)
   {
      int val{};
      stream_.read((char*)&val, sizeof(val));
      value_ = val;
   }
   else if (type_ind == (size_t)ESetTypes::eString)
   {
      std::string str;
      readif(stream_, str);
      value_ = str;
   }
}

//----------------------------------------------------------------------
static void write2of(std::ofstream& stream_, const std::string& str_)
{
   auto ssize = str_.size();
   stream_.write((char*)&ssize, sizeof(ssize));
   stream_.write(str_.c_str(), str_.size());
}

//----------------------------------------------------------------------
static void write2of(std::ofstream& stream_, const TSetValue& value_)
{
   size_t type_ind = value_.index();

   stream_.write((char*)&type_ind, sizeof(type_ind));

   std::visit([&](const auto& elem_)
   {
      if constexpr (std::is_same_v<std::decay_t<decltype(elem_)>, std::string>)
            write2of(stream_, elem_);
      else
            stream_.write((char*)&elem_, sizeof(elem_));
   }, value_);
}

//----------------------------------------------------------------------
static void write2of(std::ofstream& stream_, const Function& fn_)
{
   write2of(stream_, fn_.first);
   write2of(stream_, fn_.second);
}

//----------------------------------------------------------------------
bool Scene::SaveBinary(const QString& path_) const
{
   if (!m_pLCategory)
      return false;

   std::ofstream output(path_.toStdString(), std::ios::out | std::ios::binary);
   if (!output.is_open())
      return false;

   auto nodes = m_pLCategory->QueryNodes("*");

   size_t sz = nodes.size();

   output.write((char*)&sz, sizeof(sz));

   for (const auto& node : nodes)
      write2of(output, node.Name());

   auto arrows = m_pLCategory->QueryArrows("* :: * -> *");

   sz = arrows.size() - nodes.size();

   output.write((char*)&sz, sizeof(sz));

   for (const auto& arrow : arrows)
   {
      // Skipping identity
      if (arrow.Source() == arrow.Target())
         continue;

      write2of(output, arrow.Name   ());
      write2of(output, arrow.Source ());
      write2of(output, arrow.Target ());

      std::list<Function> fns = function_values(arrow.Source(), arrow.Target(), m_pLCategory);

      sz = fns.size();

      output.write((char*)&sz, sizeof(sz));

      for (const Function& fn : fns)
         write2of(output, fn);
   }

   output.close();

   return true;
}

//----------------------------------------------------------------------
void Scene::New()
{
   DeInit();
   Init  ();
}

//----------------------------------------------------------------------
bool eval_expr(const std::string& expr_, const std::list<Function>& fns_)
{
   bool eq = expr_.find(sEquality ) != -1;
   bool nq = expr_.find(sNEquality) != -1;

   std::string split_factor;

   if (eq)
      split_factor = sEquality;
   else if (nq)
      split_factor = sNEquality;

   auto cond_parts = split(expr_, split_factor, false);
   if (cond_parts.size() != 2)
      return false;

   bool (*cmp_d)(double left_, double right_);
   bool (*cmp_f)(float left_, float right_);
   bool (*cmp_i)(int left_, int right_);
   bool (*cmp_str)(const std::string & left_, const std::string & right_);

   if (eq)
   {
      cmp_d = [](double left_, double right_) { return left_ == right_; };
      cmp_f = [](float left_, float right_) { return left_ == right_; };
      cmp_i = [](int left_, int right_) { return left_ == right_; };
      cmp_str = [](const std::string& left_, const std::string& right_) { return left_ == right_; };
   }
   else if (nq)
   {
      cmp_d = [](double left_, double right_) { return left_ != right_; };
      cmp_f = [](float left_, float right_) { return left_ != right_; };
      cmp_i = [](int left_, int right_) { return left_ != right_; };
      cmp_str = [](const std::string& left_, const std::string& right_) { return left_ != right_; };
   }

   for (auto& it : cond_parts)
      it = trim_sp(it);

   auto& name = cond_parts[0];
   auto& value = cond_parts[1];

   for (const Function& f : fns_)
   {
      if (f.first == name)
      {
         if (const double* pVal = std::get_if<double>(&f.second))
         {
            return cmp_d(*pVal, std::stod(value));
         }
         else if (const float* pVal = std::get_if<float>(&f.second))
         {
            return cmp_f(*pVal, std::stof(value));
         }
         else if (const int* pVal = std::get_if<int>(&f.second))
         {
            return cmp_i(*pVal, std::stoi(value));
         }
         else if (const std::string* pVal = std::get_if<std::string>(&f.second))
         {
            return cmp_str(*pVal, value);
         }
      }
   }

   return false;
}

//-----------------------------------------------------------------------------------------
static StringVec csplit(const std::string& string_, const std::string& splitter_, bool keep_empty_, bool keep_splitter_)
{
   StringVec ret;

   std::string remain = string_;

   auto ind = remain.find(splitter_);
   if (ind == -1)
      return { string_ };

   while (ind != -1)
   {
      std::string chunk(std::string(remain.begin(), remain.begin() + ind));

      bool isCopy = !chunk.empty() || keep_empty_;
      if (isCopy)
         ret.push_back(chunk);

      if (keep_splitter_)
         ret.push_back(std::string(remain.begin() + ind, remain.begin() + ind + splitter_.length()));

      remain = std::string(remain.begin() + ind + splitter_.length(), remain.end());

      ind = remain.find(splitter_);
      if (ind == -1)
      {
         bool isCopy = !remain.empty() || keep_empty_;
         if (isCopy)
            ret.push_back(remain);
      }
   }

   return ret;
}

//----------------------------------------------------------------------
StringVec chain_expr(const std::string& expr_)
{
   auto parts = csplit(expr_, sOR, false, true);

   StringVec ret;
   for (auto& it : parts)
   {
      auto sub_parts = csplit(it, sAND, false, true);
      ret.insert(ret.end(), sub_parts.begin(), sub_parts.end());
   }

   for (auto& it : ret)
      it = trim_sp(it);

   return ret;
}

//----------------------------------------------------------------------
void Scene::Filter(const QString& filter_)
{
   chain_expr(filter_.toStdString());

   std::string filter = filter_.toStdString();

   QList<QGraphicsItem*> nodes = items();

   if (filter.empty())
   {
      for (QGraphicsItem* pNode : nodes)
         pNode->setVisible(true);

      return;
   }

   auto expr_parts = chain_expr(filter);

   for (QGraphicsItem* it : nodes)
   {
      auto id = it->data(eID);

      CNode* pNode = dynamic_cast<CNode*>(it);
      if (!pNode)
         continue;

      auto node_name = id.toString().toStdString();

      Arrow::List arrows = m_pLCategory->QueryArrows("*::" + node_name + "->" + sSet);
      if (arrows.empty())
         continue;

      const Arrow& arrow = arrows.front();

      std::list<Function> fns = function_values(arrow.Source(), arrow.Target(), m_pLCategory);

      fns.push_back({ FunctionName(id_token), TSetValue(node_name) });

      std::optional<bool> success;
      for (size_t i = 0; i < expr_parts.size(); ++i)
      {
         if (i % 2 != 0)
            continue;

         if (!success.has_value())
         {
            success = eval_expr(expr_parts[i], fns);
         }
         else
         {
            if (expr_parts[i - 1] == sOR)
            {
               success = success.value() || eval_expr(expr_parts[i], fns);
            }
            else if (expr_parts[i - 1] == sAND)
            {
               success = success.value() && eval_expr(expr_parts[i], fns);
            }
         }
      }

      pNode->SetVisibility(success.value());
   }
}

//----------------------------------------------------------------------
void Scene::ChangeLabel(const QString& name_)
{
   m_ShownName = name_.toStdString();

   QList<QGraphicsItem*> nodes = items();
   for (QGraphicsItem* pNode : nodes)
      changeLabel(pNode);
}

//----------------------------------------------------------------------
QList<QMap<QString, QString>> Scene::GetDescription() const
{
   QList<QMap<QString, QString>> ret;

   QList<QGraphicsItem*> nodes = items();

   for (QGraphicsItem* pNode : nodes)
   {
      if (pNode->isVisible())
      {
         auto record = getRecord(pNode);
         if (!record.empty())
            ret.push_back(record);
      }
   }

   return ret;
}

//----------------------------------------------------------------------
bool Scene::Build(const QString& path_)
{
   Parser prs;
   if (!prs.Parse(path_.toStdString().c_str()))
      return false;

   if (!prs.Data() || prs.Data()->Type() != Node::EType::eSCategory)
      return false;

   m_pLCategory = prs.Data();

   for (auto& node : m_pLCategory->QueryNodes("*"))
   {
      CNode* pItem = new CNode(scene_size * 0.5, scene_size * 0.5, node.Name().c_str());
      addItem(pItem);

      connect(pItem, &CNode::positionChanged, this, &Scene::positionChanged);
   }

   for (auto& arrow : m_pLCategory->QueryArrows("* :: * -> *"))
   {
      CNode* pSource = (CNode*)getItem(arrow.Source().c_str());
      CNode* pTarget = (CNode*)getItem(arrow.Target().c_str());

      CArrow* pItem = new CArrow(pSource, pTarget, arrow.Name().c_str());
      addItem(pItem);
   }

   emit updateStatistics(Statistics());

   return true;
}

//----------------------------------------------------------------------
bool Scene::LoadBinary(const QString& path_)
{
   std::ifstream input(path_.toStdString(), std::ios::in | std::ios::binary);
   if (!input.is_open())
      return false;

   size_t node_count{};
   input.read((char*)&node_count, sizeof(node_count));

   for (int i = 0; i < node_count; ++i)
   {
      Node::NName name;
      readif(input, name);

      if (!createNode(name.c_str(), QPointF(scene_size * 0.5, scene_size * 0.5)))
         return false;
   }

   size_t arrow_count{};
   input.read((char*)&arrow_count, sizeof(arrow_count));

   for (int i = 0; i < arrow_count; ++i)
   {
      Arrow::AName name;
      readif(input, name);

      std::string source;
      readif(input, source);

      std::string target;
      readif(input, target);

      CNode* pSource = (CNode*)getItem(source.c_str());
      CNode* pTarget = (CNode*)getItem(target.c_str());

      assert(pSource && pTarget);

      if (!pSource || !pTarget)
         return false;

      size_t prop_count{};
      input.read((char*)&prop_count, sizeof(prop_count));

      std::list<Function> fns; fns.resize(prop_count);

      auto itFns = fns.begin();
      for (int i = 0; i < prop_count; ++i, ++itFns)
      {
         readif(input, itFns->first);
         readif(input, itFns->second);
      }

      std::optional<int> node_x;
      std::optional<int> node_y;

      for (auto& fn : fns)
      {
         if (fn.first == x_token && fn.second.index() == (size_t)ESetTypes::eInt)
            node_x = std::get<(size_t)ESetTypes::eInt>(fn.second);

         if (fn.first == y_token && fn.second.index() == (size_t)ESetTypes::eInt)
            node_y = std::get<(size_t)ESetTypes::eInt>(fn.second);
      }

      if (node_x && node_y)
         pSource->setPos(node_x.value(), node_y.value());

      if (!createArrow(pSource, pTarget, name.c_str(), fns))
         printf("Error creating arrow: %s \n", name.c_str());
   }

   input.close();

   emit updateStatistics(Statistics());

   return true;
}
