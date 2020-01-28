#pragma once

#include <QString>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

namespace mediaelch {

enum class ColumnAlignment
{
    Right,
    Left
};

class TableColumn
{
public:
    TableColumn() = default;
    explicit TableColumn(QString heading, unsigned width, ColumnAlignment align = ColumnAlignment::Left) :
        m_heading{heading}, m_width{width}, m_align{align}
    {
    }

    const QString& heading() const { return m_heading; }
    ColumnAlignment alignment() const { return m_align; }
    unsigned width() const { return m_width; }

private:
    QString m_heading;
    unsigned m_width = 0;
    ColumnAlignment m_align = ColumnAlignment::Left;
};

class TableLayout
{
public:
    TableLayout() = default;

    void addColumn(TableColumn column) { m_columns.push_back(std::move(column)); }

    int columnCount() const { return m_columns.size(); }
    const TableColumn& column(int index) const { return m_columns.at(index); }

private:
    std::vector<TableColumn> m_columns;
};

class TableWriter
{
public:
    explicit TableWriter(std::ostream& out, TableLayout layout) : m_out{out}, m_layout{std::move(layout)} {}

    void writeHeading();
    void writeCell(const QString& str);
    void writeCell(const std::string& str);
    void fillCell(char fillChar);

private:
    void beforeCell();
    void afterCell();
    void prepareCellProperties();

    std::ostream& m_out;
    TableLayout m_layout;
    int m_currentColumn = 0;
};

} // namespace mediaelch
