#pragma once

#include <QAbstractTableModel>
#include <cppstacksize/logger.h>
#include <cppstacksize/reader.h>
#include <deque>
#include <string_view>

namespace cppstacksize {
class Log_Table_Model : public QAbstractTableModel, public Logger {
  Q_OBJECT
 public:
  explicit Log_Table_Model(QObject *parent = nullptr);
  ~Log_Table_Model();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  void log(std::string_view message, const Location &location) override;

 private:
  std::deque<Captured_Log_Message> messages_;
};
}
