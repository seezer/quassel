/***************************************************************************
 *   Copyright (C) 2005-08 by the Quassel Project                          *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "treemodel.h"

#include <QDebug>

/*****************************************
 *  Abstract Items of a TreeModel
 *****************************************/
AbstractTreeItem::AbstractTreeItem(AbstractTreeItem *parent)
  : QObject(parent),
    _flags(Qt::ItemIsSelectable | Qt::ItemIsEnabled)
{
}

AbstractTreeItem::~AbstractTreeItem() {
}

quint64 AbstractTreeItem::id() const {
  return (quint64)this;
}

int AbstractTreeItem::defaultColumn() const {
  // invalid QModelIndexes aka rootNodes get their Childs stuffed into column -1
  // all others to 0
  if(parent() == 0)
    return -1;
  else
    return 0;
}

bool AbstractTreeItem::newChild(int column, AbstractTreeItem *item) {
  if(column >= columnCount()) {
    qWarning() << "AbstractTreeItem::newChild() cannot append Child to not existing column!" << this << column;
    return false;
  }
  
  if(!_childItems.contains(column)) {
    _childItems[column] = QList<AbstractTreeItem *>();
  }

  int newRow = _childItems[column].count();
  emit beginAppendChilds(column, newRow, newRow);
  _childItems[column].append(item);
  emit endAppendChilds();
  
  return true;
}

bool AbstractTreeItem::newChild(AbstractTreeItem *item) {
  return newChild(defaultColumn(), item);
}

bool AbstractTreeItem::removeChild(int column, int row) {
  if(!_childItems.contains(column) || row >= childCount(column))
    return false;

  emit beginRemoveChilds(column, row, row);
  AbstractTreeItem *treeitem = _childItems[column].takeAt(row);
  treeitem->deleteLater();
  emit endRemoveChilds();

  return true;
}

bool AbstractTreeItem::removeChild(int row) {
  return removeChild(defaultColumn(), row);
}

bool AbstractTreeItem::removeChildById(int column, const quint64 &id) {
  if(!_childItems.contains(column))
    return false;

  for(int i = 0; i < _childItems[column].count(); i++) {
    if(_childItems[column][i]->id() == id)
      return removeChild(column, i);
  }
  return false;
}

bool AbstractTreeItem::removeChildById(const quint64 &id) {
  return removeChildById(defaultColumn(), id);
}

void AbstractTreeItem::removeAllChilds() {
  AbstractTreeItem *child;

  QHash<int, QList<AbstractTreeItem *> >::iterator columnIter = _childItems.begin();
  while(columnIter != _childItems.end()) {
    if(columnIter->count() > 0) {
      emit beginRemoveChilds(columnIter.key(), 0, columnIter->count() - 1);
      QList<AbstractTreeItem *>::iterator childIter = columnIter->begin();
      while(childIter != columnIter->end()) {
	child = *childIter;
	// child->removeAllChilds();
	childIter = columnIter->erase(childIter);
	child->deleteLater();
      }
      emit endRemoveChilds();
    }
    columnIter++;
  }
}

AbstractTreeItem *AbstractTreeItem::child(int column, int row) const {
  if(!_childItems.contains(column) || _childItems[column].size() <= row)
    return 0;
  else
    return _childItems[column].value(row);
}

AbstractTreeItem *AbstractTreeItem::child(int row) const {
  return child(defaultColumn(), row);
}

AbstractTreeItem *AbstractTreeItem::childById(int column, const quint64 &id) const {
  if(!_childItems.contains(column))
    return 0;

  for(int i = 0; i < _childItems[column].count(); i++) {
    if(_childItems[column][i]->id() == id)
      return _childItems[column][i];
  }
  return 0;
}

AbstractTreeItem *AbstractTreeItem::childById(const quint64 &id) const {
  return childById(defaultColumn(), id);
}

int AbstractTreeItem::childCount(int column) const {
  if(!_childItems.contains(column))
    return 0;
  else
    return _childItems[column].count();
}

int AbstractTreeItem::childCount() const {
  return childCount(defaultColumn());
}

int AbstractTreeItem::column() const {
  if(!parent())
    return -1;

  QHash<int, QList<AbstractTreeItem*> >::const_iterator iter = parent()->_childItems.constBegin();
  while(iter != parent()->_childItems.constEnd()) {
    if(iter->contains(const_cast<AbstractTreeItem *>(this)))
      return iter.key();
    iter++;
  }

  qWarning() << "AbstractTreeItem::column(): unable to determine the Column of" << this;
  return parent()->defaultColumn();
}

int AbstractTreeItem::row() const {
  if(!parent())
    return -1;
  else
    return parent()->_childItems[column()].indexOf(const_cast<AbstractTreeItem *>(this));
}

AbstractTreeItem *AbstractTreeItem::parent() const {
  return qobject_cast<AbstractTreeItem *>(QObject::parent());
}

Qt::ItemFlags AbstractTreeItem::flags() const {
  return _flags;
}

void AbstractTreeItem::setFlags(Qt::ItemFlags flags) {
  _flags = flags;
}

/*****************************************
 * SimpleTreeItem
 *****************************************/
SimpleTreeItem::SimpleTreeItem(const QList<QVariant> &data, AbstractTreeItem *parent)
  : AbstractTreeItem(parent),
    _itemData(data)
{
}

SimpleTreeItem::~SimpleTreeItem() {
}

QVariant SimpleTreeItem::data(int column, int role) const {
  if(column >= columnCount() || role != Qt::DisplayRole)
    return QVariant();
  else
    return _itemData[column];
}

bool SimpleTreeItem::setData(int column, const QVariant &value, int role) {
  if(column > columnCount() || role != Qt::DisplayRole)
    return false;

  if(column == columnCount())
    _itemData.append(value);
  else
    _itemData[column] = value;

  emit dataChanged(column);
  return true;
}

int SimpleTreeItem::columnCount() const {
  return _itemData.count();
}

/*****************************************
 * PropertyMapItem
 *****************************************/
PropertyMapItem::PropertyMapItem(const QStringList &propertyOrder, AbstractTreeItem *parent)
  : AbstractTreeItem(parent),
    _propertyOrder(propertyOrder)
{
}

PropertyMapItem::PropertyMapItem(AbstractTreeItem *parent)
  : AbstractTreeItem(parent),
    _propertyOrder(QStringList())
{
}


PropertyMapItem::~PropertyMapItem() {
}
  
QVariant PropertyMapItem::data(int column, int role) const {
  if(column >= columnCount())
    return QVariant();

  switch(role) {
  case Qt::ToolTipRole:
    return toolTip(column);
  case Qt::DisplayRole:
    return property(_propertyOrder[column].toAscii());
  default:
    return QVariant();
  }
  
}

bool PropertyMapItem::setData(int column, const QVariant &value, int role) {
  if(column >= columnCount() || role != Qt::DisplayRole)
    return false;

  emit dataChanged(column);
  return setProperty(_propertyOrder[column].toAscii(), value);
}

int PropertyMapItem::columnCount() const {
  return _propertyOrder.count();
}
  
void PropertyMapItem::appendProperty(const QString &property) {
  _propertyOrder << property;
}



/*****************************************
 * TreeModel
 *****************************************/
TreeModel::TreeModel(const QList<QVariant> &data, QObject *parent)
  : QAbstractItemModel(parent),
    _childStatus(QModelIndex(), 0, 0, 0),
    _aboutToRemoveOrInsert(false)
{
  rootItem = new SimpleTreeItem(data, 0);
  connectItem(rootItem);
}

TreeModel::~TreeModel() {
  delete rootItem;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
  if(!hasIndex(row, column, parent))
    return QModelIndex();
  
  AbstractTreeItem *parentItem;
  
  if(!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<AbstractTreeItem*>(parent.internalPointer());
  
  AbstractTreeItem *childItem = parentItem->child(parent.column(), row);

  if(childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex TreeModel::indexById(quint64 id, const QModelIndex &parent) const {
  AbstractTreeItem *parentItem; 
  
  if(!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<AbstractTreeItem *>(parent.internalPointer());
  
  AbstractTreeItem *childItem = parentItem->childById(parent.column(), id);
  
  if(childItem)
    return createIndex(childItem->row(), 0, childItem);
  else
    return QModelIndex();
}

QModelIndex TreeModel::indexByItem(AbstractTreeItem *item, int column) const {
  if(item == 0) {
    qWarning() << "TreeModel::indexByItem(AbstractTreeItem *item) received NULL-Pointer";
    return QModelIndex();
  }
  
  if(item == rootItem)
    return QModelIndex();
  else
    return createIndex(item->row(), column, item);

}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
  if(!index.isValid())
    return QModelIndex();
  
  AbstractTreeItem *childItem = static_cast<AbstractTreeItem *>(index.internalPointer());
  AbstractTreeItem *parentItem = static_cast<AbstractTreeItem *>(childItem->parent());
  
  if(parentItem == rootItem)
    return QModelIndex();
  
  return createIndex(parentItem->row(), childItem->column(), parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const {
  AbstractTreeItem *parentItem;
  if(!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<AbstractTreeItem*>(parent.internalPointer());

  return parentItem->childCount(parent.column());
}

int TreeModel::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent)
  // since there the Qt Views don't draw more columns than the header has columns
  // we can be lazy and simply return the count of header columns
  // actually this gives us more freedom cause we don't have to ensure that a rows parent
  // has equal or more columns than that row
  
//   if(parent.isValid()) {
//     AbstractTreeItem *child;
//     if(child = static_cast<AbstractTreeItem *>(parent.internalPointer())->child(parent.column(), parent.row()))
//       return child->columnCount();
//     else
//       return static_cast<AbstractTreeItem*>(parent.internalPointer())->columnCount();
//   } else {
//     return rootItem->columnCount();
//   }

  return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
  if(!index.isValid())
    return QVariant();

  AbstractTreeItem *item = static_cast<AbstractTreeItem *>(index.internalPointer());
  return item->data(index.column(), role);
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if(!index.isValid())
    return false;

  AbstractTreeItem *item = static_cast<AbstractTreeItem *>(index.internalPointer());
  return item->setData(index.column(), value, role);
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
  AbstractTreeItem *item;
  if(!index.isValid())
    item = rootItem;
  else
    item = static_cast<AbstractTreeItem *>(index.internalPointer());
  return item->flags();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section, role);
  else
    return QVariant();
}

void TreeModel::itemDataChanged(int column) {
  AbstractTreeItem *item = qobject_cast<AbstractTreeItem *>(sender());
  QModelIndex leftIndex, rightIndex;

  if(item == rootItem)
    return;

  if(column == -1) {
    leftIndex = createIndex(item->row(), 0, item);
    rightIndex = createIndex(item->row(), item->columnCount()-1, item);
  } else {
    leftIndex = createIndex(item->row(), column, item);
    rightIndex = leftIndex;
  }

  emit dataChanged(leftIndex, rightIndex);
}

void TreeModel::connectItem(AbstractTreeItem *item) {
  connect(item, SIGNAL(dataChanged(int)),
	  this, SLOT(itemDataChanged(int)));
  
  connect(item, SIGNAL(beginAppendChilds(int, int, int)),
	  this, SLOT(beginAppendChilds(int, int, int)));
  connect(item, SIGNAL(endAppendChilds()),
	  this, SLOT(endAppendChilds()));
  
  connect(item, SIGNAL(beginRemoveChilds(int, int, int)),
	  this, SLOT(beginRemoveChilds(int, int, int)));
  connect(item, SIGNAL(endRemoveChilds()),
	  this, SLOT(endRemoveChilds()));
}

void TreeModel::beginAppendChilds(int column, int firstRow, int lastRow) {
  AbstractTreeItem *parentItem = qobject_cast<AbstractTreeItem *>(sender());
  if(!parentItem) {
    qWarning() << "TreeModel::beginAppendChilds(): cannot append Childs to unknown parent";
    return;
  }
  QModelIndex parent = indexByItem(parentItem, column);
  Q_ASSERT(!_aboutToRemoveOrInsert);
  
  _aboutToRemoveOrInsert = true;
  _childStatus = ChildStatus(parent, rowCount(parent), firstRow, lastRow);
  beginInsertRows(parent, firstRow, lastRow);
}

void TreeModel::endAppendChilds() {
  AbstractTreeItem *parentItem = qobject_cast<AbstractTreeItem *>(sender());
  if(!parentItem) {
    qWarning() << "TreeModel::endAppendChilds(): cannot append Childs to unknown parent";
    return;
  }
  Q_ASSERT(_aboutToRemoveOrInsert);
  ChildStatus cs = _childStatus;
  QModelIndex parent = indexByItem(parentItem, cs.parent.column());
  Q_ASSERT(cs.parent == parent);
  Q_ASSERT(rowCount(parent) == cs.childCount + cs.end - cs.start + 1);

  _aboutToRemoveOrInsert = false;
  for(int i = cs.start; i <= cs.end; i++) {
    connectItem(parentItem->child(parent.column(), i));
  }
  endInsertRows();
}

void TreeModel::beginRemoveChilds(int column, int firstRow, int lastRow) {
  AbstractTreeItem *parentItem = qobject_cast<AbstractTreeItem *>(sender());
  if(!parentItem) {
    qWarning() << "TreeModel::beginRemoveChilds(): cannot append Childs to unknown parent";
    return;
  }
  QModelIndex parent = indexByItem(parentItem, column);
  Q_ASSERT(firstRow <= lastRow);
  Q_ASSERT(parentItem->childCount(column) > lastRow);
  Q_ASSERT(!_aboutToRemoveOrInsert);
  
  _aboutToRemoveOrInsert = true;
  _childStatus = ChildStatus(parent, rowCount(parent), firstRow, lastRow);
  beginRemoveRows(parent, firstRow, lastRow);
}

void TreeModel::endRemoveChilds() {
  AbstractTreeItem *parentItem = qobject_cast<AbstractTreeItem *>(sender());
  if(!parentItem) {
    qWarning() << "TreeModel::endRemoveChilds(): cannot append Childs to unknown parent";
    return;
  }
  Q_ASSERT(_aboutToRemoveOrInsert);
  ChildStatus cs = _childStatus;
  QModelIndex parent = indexByItem(parentItem, cs.parent.column());
  Q_ASSERT(cs.parent == parent);
  Q_ASSERT(rowCount(parent) == cs.childCount - cs.end + cs.start - 1);
  
  _aboutToRemoveOrInsert = false;
  endRemoveRows();
}

void TreeModel::clear() {
  rootItem->removeAllChilds();
}
