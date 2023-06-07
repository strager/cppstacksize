#pragma once

#include <QAbstractTableModel>

namespace cppstacksize {
class Function_Table_Model : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit Function_Table_Model(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
};
}
