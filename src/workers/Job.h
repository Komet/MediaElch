#pragma once

#include "utils/Meta.h"

#include <QObject>

namespace mediaelch {
namespace worker {

class Job : public QObject
{
    Q_OBJECT
public:
    explicit Job(QObject* parent = nullptr) : QObject(parent) {}
    ~Job() override;

public slots:
    /// \brief Start the job. Calls virtual doStart().
    void start(); // NOLINT(readability-make-member-function-const)
    /// \brief Kill this job. Emits finished() signal with error set to KilledJobError.
    bool kill();

signals:
    /// \brief Signal emitted when the job is done or killed.
    /// \note  This is a private signal. Use emitFinished() instead.
    ///        You can still connect to this signal.
    ///
    /// \param job The job that emitted this signal.
    void finished(mediaelch::worker::Job* job, QPrivateSignal);
    /// \brief Emitted when elements were processed.
    /// \note  This is a private signal. Use emitPercent() instead.
    ///
    /// \param job The job that emitted this signal.
    /// \param percent Percentage between 0.0 and 100.0
    void percentChanged(mediaelch::worker::Job* job, float percent, QPrivateSignal);

public:
    enum ErrorCode : int
    {
        NoError = 0,
        /// The job was killed.
        KilledJobError = 1,
        /// Subclasses should provide error codes starting after this value.
        UserError = 1000,
    };

    /// \brief Returns true if there is no error, i.e. errorCode() is 0.
    ELCH_NODISCARD bool hasError() const;
    /// \brief Integer representing the job error. 0 if there isn't an error.
    /// \see JobError
    ELCH_NODISCARD int errorCode() const;
    /// \brief Human readable error string, often translated.
    ELCH_NODISCARD QString errorString() const;
    /// \brief Technical error text, e.g. URL.
    ELCH_NODISCARD QString errorText() const;
    /// \brief Whether the job was killed/canceled.
    ELCH_NODISCARD bool wasKilled() const;
    /// \brief Whether the job was finished or killed.
    ELCH_NODISCARD bool isFinished() const;

    ELCH_NODISCARD bool isAutoDelete() const;
    void setAutoDelete(bool autoDelete);

protected:
    virtual void doStart() = 0;
    /// \brief   Aborts the job without emitting any signals.
    /// \details This virtual method is used in kill().
    ///          Return false if the job can't be killed.
    virtual bool doKill();

    /// \see ErrorCode
    void setError(int errorCode);
    void setErrorString(QString msg);
    void setErrorText(QString msg);

    /// Emit that the job is finished.
    /// Deletes this job using deleteLater().
    ///
    /// \see result()
    /// \see finished()
    void emitFinished();
    void emitPercent(elch_ssize_t processed, elch_ssize_t total);

private:
    float m_percent = 0.0f;
    bool m_isFinished = false;
    bool m_isAutoDelete = true;

    ErrorCode m_errorCode = ErrorCode::NoError;
    QString m_errorString;
    QString m_errorText;
};

} // namespace worker
} // namespace mediaelch
