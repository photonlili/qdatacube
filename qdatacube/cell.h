#ifndef CELL_H
#define CELL_H
#include <QDebug>

namespace qdatacube {

/**
 * Represents a cell in the datacube
 * NOTE: If this class is exported, remember to consider whether the datamembers should be moved to a private class
 * and inlines moved to class.
 */
class cell_t
{
  public:
    cell_t(int row_section, int column_section) : m_row(row_section), m_column(column_section) {}
    cell_t() : m_row(-1), m_column(-1) {}
    bool operator==(const cell_t& rhs) {
      return rhs.m_row == m_row && rhs.m_column == m_column;
    }
    bool invalid() const {
      return m_row == -1;
    }
    int row() const {
      return m_row;
    }

    int column() const {
      return m_column;
    }
  private:
    int m_row;
    int m_column;
};

QDebug operator<<(QDebug dbg, const cell_t& cell);

} // end of namespace
#endif // CELL_H
