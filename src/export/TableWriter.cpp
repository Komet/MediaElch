#include "export/TableWriter.h"

namespace mediaelch {

void TableWriter::writeHeading()
{
    for (int i = 0; i < m_layout.columnCount(); ++i) {
        writeCell(m_layout.column(i).heading());
    }
    m_out << "|";
    for (int i = 0; i < m_layout.columnCount(); ++i) {
        m_out << '-' << std::string(m_layout.column(i).width(), '-') << "-|";
    }
    m_out << "\n";
}

void TableWriter::writeCell(const QString& str)
{
    beforeCell();
    m_out << str.toUtf8().data();
    afterCell();
}

void TableWriter::writeCell(const std::string& str)
{
    beforeCell();
    m_out << str;
    afterCell();
}

void TableWriter::fillCell(char fillChar)
{
    beforeCell();
    m_out << std::string(m_layout.column(m_currentColumn).width(), fillChar);
    afterCell();
}

void TableWriter::beforeCell()

{
    m_out << "| ";
    prepareCellProperties();
}

void TableWriter::afterCell()

{
    m_out << ((++m_currentColumn != m_layout.columnCount()) ? " " : " |\n");
    m_currentColumn %= m_layout.columnCount();
}

void TableWriter::prepareCellProperties()

{
    const auto& col = m_layout.column(m_currentColumn);
    m_out << std::setw(static_cast<int>(col.width()));
    switch (col.alignment()) {
    case ColumnAlignment::Left: m_out << std::left; break;
    case ColumnAlignment::Right: m_out << std::right; break;
    }
}

} // namespace mediaelch
