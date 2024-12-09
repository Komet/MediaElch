#pragma once

#include "data/movie/Movie.h"
#include "ui/renamer/RenamerDialog.h"

#include <QDialog>

class MovieRenamerDialog final : public RenamerDialog
{
    Q_OBJECT

public:
    explicit MovieRenamerDialog(QWidget* parent = nullptr);
    ~MovieRenamerDialog() override;

    void setMovies(QVector<Movie*> movies);

private:
    void renameType(bool isDryRun) override;
    void rejectImpl() override;
    QString dialogInfoLabel() override;
    void renameMovies(QVector<Movie*> movies, const RenamerConfig& config);

    QStringList fileNameDefaults() override;
    QStringList fileNameMultiDefaults() override;
    QStringList directoryNameDefaults() override;

private:
    QVector<Movie*> m_movies;
};
