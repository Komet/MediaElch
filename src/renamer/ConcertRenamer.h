#pragma once

#include "RenamerPlaceholders.h"
#include "globals/Helper.h"
#include "media/StreamDetails.h"
#include "renamer/Renamer.h"

class RenamerDialog;
class Concert;


namespace mediaelch {

class ConcertRenamerPlaceholders : public RenamerPlaceholders
{
public:
    ~ConcertRenamerPlaceholders() override;

    QVector<Placeholder> placeholders() override;
};

class ConcertRenamerData : public RenamerData
{
public:
    explicit ConcertRenamerData(Concert& concert) : m_concert(concert) {}
    ~ConcertRenamerData() override;

    void setPartNo(int partNo) { m_partNo = partNo; }
    void setExtension(QString extension) { m_extension = extension; }
    void setIsBluRay(bool isBluRay) { m_isBluRay = isBluRay; }
    void setIsDvd(bool isDvd) { m_isDvd = isDvd; }
    void setVideoDetails(QMap<StreamDetails::VideoDetails, QString> videoDetails) { m_videoDetails = videoDetails; }

    ELCH_NODISCARD QString value(const QString& name) const override;
    ELCH_NODISCARD bool passesCondition(const QString& name) const override;

private:
    Concert& m_concert;
    QString m_extension;
    QMap<StreamDetails::VideoDetails, QString> m_videoDetails;
    int m_partNo{0};
    bool m_isBluRay{false};
    bool m_isDvd{false};
};

} // namespace mediaelch


class ConcertRenamer : public Renamer
{
public:
    ConcertRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog);
    RenameError renameConcert(Concert& concert);
};
