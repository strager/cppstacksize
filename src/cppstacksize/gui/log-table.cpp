#include <cppstacksize/base.h>
#include <cppstacksize/gui/log-table.h>
#include <cppstacksize/logger.h>

namespace cppstacksize {
Log_Table_Model::Log_Table_Model(QObject* parent)
    : QAbstractTableModel(parent) {}

Log_Table_Model::~Log_Table_Model() = default;

int Log_Table_Model::rowCount(const QModelIndex&) const {
  return narrow_cast<int>(this->messages_.size());
}

int Log_Table_Model::columnCount(const QModelIndex&) const { return 4; }

QVariant Log_Table_Model::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    CSS_ASSERT(index.row() >= 0);
    U64 row = narrow_cast<U64>(index.row());
    CSS_ASSERT(row < this->messages_.size());
    if (row >= this->messages_.size()) {
      return QVariant();
    }
    const Captured_Log_Message& message = this->messages_[index.row()];
    switch (index.column()) {
      case 0:
        return narrow_cast<qulonglong>(message.location.file_offset);
      case 1:
        return message.location.stream_index.has_value()
                   ? QVariant(*message.location.stream_index)
                   : QVariant();
      case 2:
        return message.location.stream_offset.has_value()
                   ? QVariant(*message.location.stream_offset)
                   : QVariant();
      case 3:
        // TODO(strager): Store QString-s in this->message_.
        return QString(message.message.c_str());
      default:
        CSS_UNREACHABLE();
        break;
    }
  }
  return QVariant();
}

QVariant Log_Table_Model::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("file offset");
      case 1:
        return QString("stream");
      case 2:
        return QString("stream offset");
      case 3:
        return QString("message");
    }
  }
  return QVariant();
}

void Log_Table_Model::log(std::string_view message, const Location& location) {
  int new_message_row_number = this->messages_.size();
  this->beginInsertRows(QModelIndex(), new_message_row_number,
                        new_message_row_number);
  this->messages_.push_back(Captured_Log_Message{
      .location = location,
      .message = std::string(message),
  });
  this->endInsertRows();
}
}
