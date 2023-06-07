#include <cppstacksize/gui/function-table.h>

namespace cppstacksize {
Function_Table_Model::Function_Table_Model(QObject *parent)
    : QAbstractTableModel(parent) {}

int Function_Table_Model::rowCount(const QModelIndex &) const { return 2; }

int Function_Table_Model::columnCount(const QModelIndex &) const { return 3; }

QVariant Function_Table_Model::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return QString("function_%1").arg(index.row());
      case 1:
        return index.row() * 8;
      case 2:
        return 32;
      default:
        __builtin_unreachable();
        break;
    }
  }
  return QVariant();
}

QVariant Function_Table_Model::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("function");
      case 1:
        return QString("self size");
      case 2:
        return QString("params size");
    }
  }
  return QVariant();
}
}
